/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtNetwork module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

//#define QFTPPI_DEBUG
//#define QFTPDTP_DEBUG

#include "wftp.h"
#include "qabstractsocket.h"

#include "qcoreapplication.h"
#include "qtcpsocket.h"
#include "qurlinfo.h"
#include "qstringlist.h"
#include "qregexp.h"
#include "qtimer.h"
#include "qfileinfo.h"
#include "qhash.h"
#include "qtcpserver.h"
#include "qlocale.h"
#include <QTextCodec>
#include <QDebug>
#ifdef TEST_ENCODE_DETECTOR
#include "nsUniversalDetector.h"
#endif

QBasicAtomicInt WFtpCommand::idCounter = Q_BASIC_ATOMIC_INITIALIZER(1);

WFtpCommand::WFtpCommand(WFtp::Command cmd, QStringList raw, const QByteArray &ba)
    : command(cmd), rawCmds(raw), is_ba(true)
{
    id = idCounter.fetchAndAddRelaxed(1);
    data.ba = new QByteArray(ba);
}

WFtpCommand::WFtpCommand(WFtp::Command cmd, QStringList raw, QIODevice *dev)
    : command(cmd), rawCmds(raw), is_ba(false)
{
    id = idCounter.fetchAndAddRelaxed(1);
    data.dev = dev;
}

WFtpCommand::~WFtpCommand()
{
    if (is_ba)
        delete data.ba;
}

/**********************************************************************
 *
 * WFtpDTP implemenatation
 *
 *********************************************************************/
WFtpDTP::WFtpDTP(WFtpPI *p, QObject *parent) :
    QObject(parent),
    socket(0),
    listener(this),
    pi(p),
    callWriteData(false),
	codec(QTextCodec::codecForLocale())
{
    clearData();
    listener.setObjectName(QLatin1String("WFtpDTP active state server"));
    connect(&listener, SIGNAL(newConnection()), SLOT(setupSocket()));
}

void WFtpDTP::setData(QByteArray *ba)
{
    is_ba = true;
    data.ba = ba;
}

void WFtpDTP::setDevice(QIODevice *dev)
{
    is_ba = false;
    data.dev = dev;
}

void WFtpDTP::setBytesTotal(qint64 bytes)
{
    bytesTotal = bytes;
    bytesDone = 0;
    emit dataTransferProgress(bytesDone, bytesTotal);
}

void WFtpDTP::connectToHost(const QString & host, quint16 port)
{
    bytesFromSocket.clear();

    if (socket) {
        delete socket;
        socket = 0;
    }
    socket = new QTcpSocket(this);
#ifndef QT_NO_BEARERMANAGEMENT
    // copy network session down to the socket
    socket->setProperty("_q_networksession", property("_q_networksession"));
#endif
    socket->setObjectName(QLatin1String("WFtpDTP Passive state socket"));
    connect(socket, SIGNAL(connected()), SLOT(socketConnected()));
    connect(socket, SIGNAL(readyRead()), SLOT(socketReadyRead()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(socketError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(disconnected()), SLOT(socketConnectionClosed()));
    connect(socket, SIGNAL(bytesWritten(qint64)), SLOT(socketBytesWritten(qint64)));

    socket->connectToHost(host, port);
}

int WFtpDTP::setupListener(const QHostAddress &address)
{
#ifndef QT_NO_BEARERMANAGEMENT
    //copy network session down to the socket
    listener.setProperty("_q_networksession", property("_q_networksession"));
#endif
    if (!listener.isListening() && !listener.listen(address, 0))
        return -1;
    return listener.serverPort();
}

void WFtpDTP::waitForConnection()
{
    // This function is only interesting in Active transfer mode; it works
    // around a limitation in WFtp's design by blocking, waiting for an
    // incoming connection. For the default Passive mode, it does nothing.
    if (listener.isListening())
        listener.waitForNewConnection();
}

QTcpSocket::SocketState WFtpDTP::state() const
{
    return socket ? socket->state() : QTcpSocket::UnconnectedState;
}

qint64 WFtpDTP::bytesAvailable() const
{
    if (!socket || socket->state() != QTcpSocket::ConnectedState)
        return (qint64) bytesFromSocket.size();
    return socket->bytesAvailable();
}

qint64 WFtpDTP::read(char *data, qint64 maxlen)
{
    qint64 read;
    if (socket && socket->state() == QTcpSocket::ConnectedState) {
        read = socket->read(data, maxlen);
    } else {
        read = qMin(maxlen, qint64(bytesFromSocket.size()));
        memcpy(data, bytesFromSocket.data(), read);
        bytesFromSocket.remove(0, read);
    }

    bytesDone += read;
    return read;
}

QByteArray WFtpDTP::readAll()
{
    QByteArray tmp;
    if (socket && socket->state() == QTcpSocket::ConnectedState) {
        tmp = socket->readAll();
        bytesDone += tmp.size();
    } else {
        tmp = bytesFromSocket;
        bytesFromSocket.clear();
    }
    return tmp;
}

void WFtpDTP::writeData()
{
    if (!socket)
        return;

    if (is_ba) {
#if defined(QFTPDTP_DEBUG)
        qDebug("WFtpDTP::writeData: write %d bytes", data.ba->size());
#endif
        if (data.ba->size() == 0)
            emit dataTransferProgress(0, bytesTotal);
        else
            socket->write(data.ba->data(), data.ba->size());

        socket->close();

        clearData();
    } else if (data.dev) {
        callWriteData = false;
        const qint64 blockSize = 16*1024;
        char buf[16*1024];
        qint64 read = data.dev->read(buf, blockSize);
#if defined(QFTPDTP_DEBUG)
        qDebug("WFtpDTP::writeData: write() of size %lli bytes", read);
#endif
        if (read > 0) {
            socket->write(buf, read);
        } else if (read == -1 || (!data.dev->isSequential() && data.dev->atEnd())) {
            // error or EOF
            if (bytesDone == 0 && socket->bytesToWrite() == 0)
                emit dataTransferProgress(0, bytesTotal);
            socket->close();
            clearData();
        }

        // do we continue uploading?
        callWriteData = data.dev != 0;
    }
}

void WFtpDTP::dataReadyRead()
{
    writeData();
}

inline bool WFtpDTP::hasError() const
{
    return !err.isNull();
}

inline QString WFtpDTP::errorMessage() const
{
    return err;
}

inline void WFtpDTP::clearError()
{
    err.clear();
}

void WFtpDTP::abortConnection()
{
#if defined(QFTPDTP_DEBUG)
    qDebug("WFtpDTP::abortConnection, bytesAvailable == %lli",
           socket ? socket->bytesAvailable() : (qint64) 0);
#endif
    callWriteData = false;
    clearData();

    if (socket)
        socket->abort();
}

static void _q_fixupDateTime(QDateTime *dateTime)
{
    // Adjust for future tolerance.
    const int futureTolerance = 86400;
    if (dateTime->secsTo(QDateTime::currentDateTime()) < -futureTolerance) {
        QDate d = dateTime->date();
        d.setYMD(d.year() - 1, d.month(), d.day());
        dateTime->setDate(d);
    }
}

static void _q_parseUnixDir(const QStringList &tokens, const QString &userName, QUrlInfo *info)
{
    // Unix style, 7 + 1 entries
    // -rw-r--r--    1 ftp      ftp      17358091 Aug 10  2004 qt-x11-free-3.3.3.tar.gz
    // drwxr-xr-x    3 ftp      ftp          4096 Apr 14  2000 compiled-examples
    // lrwxrwxrwx    1 ftp      ftp             9 Oct 29  2005 qtscape -> qtmozilla
    if (tokens.size() != 8)
        return;

    char first = tokens.at(1).at(0).toLatin1();
    if (first == 'd') {
        info->setDir(true);
        info->setFile(false);
        info->setSymLink(false);
    } else if (first == '-') {
        info->setDir(false);
        info->setFile(true);
        info->setSymLink(false);
    } else if (first == 'l') {
        info->setDir(true);
        info->setFile(false);
        info->setSymLink(true);
    }

    // Resolve filename
    QString name = tokens.at(7);
    if (info->isSymLink()) {
        int linkPos = name.indexOf(QLatin1String(" ->"));
        if (linkPos != -1)
            name.resize(linkPos);
    }
    info->setName(name);

    // Resolve owner & group
    info->setOwner(tokens.at(3));
    info->setGroup(tokens.at(4));

    // Resolve size
    info->setSize(tokens.at(5).toLongLong());

    QStringList formats;
    formats << QLatin1String("MMM dd  yyyy") << QLatin1String("MMM dd hh:mm") << QLatin1String("MMM  d  yyyy")
            << QLatin1String("MMM  d hh:mm") << QLatin1String("MMM  d yyyy") << QLatin1String("MMM dd yyyy");

    QString dateString = tokens.at(6);
    dateString[0] = dateString[0].toUpper();

    // Resolve the modification date by parsing all possible formats
    QDateTime dateTime;
    int n = 0;
#ifndef QT_NO_DATESTRING
    do {
        dateTime = QLocale::c().toDateTime(dateString, formats.at(n++));
    }  while (n < formats.size() && (!dateTime.isValid()));
#endif

    if (n == 2 || n == 4) {
        // Guess the year.
        dateTime.setDate(QDate(QDate::currentDate().year(),
                               dateTime.date().month(),
                               dateTime.date().day()));
        _q_fixupDateTime(&dateTime);
    }
    if (dateTime.isValid())
        info->setLastModified(dateTime);

    // Resolve permissions
    int permissions = 0;
    QString p = tokens.at(2);
    permissions |= (p[0] == QLatin1Char('r') ? QUrlInfo::ReadOwner : 0);
    permissions |= (p[1] == QLatin1Char('w') ? QUrlInfo::WriteOwner : 0);
    permissions |= (p[2] == QLatin1Char('x') ? QUrlInfo::ExeOwner : 0);
    permissions |= (p[3] == QLatin1Char('r') ? QUrlInfo::ReadGroup : 0);
    permissions |= (p[4] == QLatin1Char('w') ? QUrlInfo::WriteGroup : 0);
    permissions |= (p[5] == QLatin1Char('x') ? QUrlInfo::ExeGroup : 0);
    permissions |= (p[6] == QLatin1Char('r') ? QUrlInfo::ReadOther : 0);
    permissions |= (p[7] == QLatin1Char('w') ? QUrlInfo::WriteOther : 0);
    permissions |= (p[8] == QLatin1Char('x') ? QUrlInfo::ExeOther : 0);
    info->setPermissions(permissions);

    bool isOwner = info->owner() == userName;
    info->setReadable((permissions & QUrlInfo::ReadOther) || ((permissions & QUrlInfo::ReadOwner) && isOwner));
    info->setWritable((permissions & QUrlInfo::WriteOther) || ((permissions & QUrlInfo::WriteOwner) && isOwner));
}

static void _q_parseDosDir(const QStringList &tokens, const QString &userName, QUrlInfo *info)
{
    // DOS style, 3 + 1 entries
    // 01-16-02  11:14AM       <DIR>          epsgroup
    // 06-05-03  03:19PM                 1973 readme.txt
    if (tokens.size() != 4)
        return;

    Q_UNUSED(userName);

    QString name = tokens.at(3);
    info->setName(name);
    info->setSymLink(name.toLower().endsWith(QLatin1String(".lnk")));

    if (tokens.at(2) == QLatin1String("<DIR>")) {
        info->setFile(false);
        info->setDir(true);
    } else {
        info->setFile(true);
        info->setDir(false);
        info->setSize(tokens.at(2).toLongLong());
    }

    // Note: We cannot use QFileInfo; permissions are for the server-side
    // machine, and QFileInfo's behavior depends on the local platform.
    int permissions = QUrlInfo::ReadOwner | QUrlInfo::WriteOwner
                      | QUrlInfo::ReadGroup | QUrlInfo::WriteGroup
                      | QUrlInfo::ReadOther | QUrlInfo::WriteOther;
    QString ext;
    int extIndex = name.lastIndexOf(QLatin1Char('.'));
    if (extIndex != -1)
        ext = name.mid(extIndex + 1);
    if (ext == QLatin1String("exe") || ext == QLatin1String("bat") || ext == QLatin1String("com"))
        permissions |= QUrlInfo::ExeOwner | QUrlInfo::ExeGroup | QUrlInfo::ExeOther;
    info->setPermissions(permissions);

    info->setReadable(true);
    info->setWritable(info->isFile());

    QDateTime dateTime;
#ifndef QT_NO_DATESTRING
    dateTime = QLocale::c().toDateTime(tokens.at(1), QLatin1String("MM-dd-yy  hh:mmAP"));
    if (dateTime.date().year() < 1971) {
        dateTime.setDate(QDate(dateTime.date().year() + 100,
                               dateTime.date().month(),
                               dateTime.date().day()));
    }
#endif

    info->setLastModified(dateTime);

}

bool WFtpDTP::parseDir(const QByteArray &buffer, const QString &userName, QUrlInfo *info, QTextCodec* codec)
{
    if (buffer.isEmpty())
        return false;

	QTextCodec* c = codec;
#ifdef TEST_ENCODE_DETECTOR
	nsUniversalDetector detector;
	detector.HandleData(buffer.constData(), buffer.length());
	const char* detectedCharSet = detector.DetectedCharset();
	if (detectedCharSet) {
		qDebug() << "detect " << detectedCharSet;
	}
#endif
    QString bufferStr = codec->toUnicode(buffer).trimmed();//QString::fromLatin1(buffer).trimmed();

    // Unix style FTP servers
    QRegExp unixPattern(QLatin1String("^([\\-dl])([a-zA-Z\\-]{9,9})\\s+\\d+\\s+(\\S*)\\s+"
                                      "(\\S*)\\s+(\\d+)\\s+(\\S+\\s+\\S+\\s+\\S+)\\s+(\\S.*)"));
    if (unixPattern.indexIn(bufferStr) == 0) {
        _q_parseUnixDir(unixPattern.capturedTexts(), userName, info);
        return true;
    }

    // DOS style FTP servers
    QRegExp dosPattern(QLatin1String("^(\\d\\d-\\d\\d-\\d\\d\\ \\ \\d\\d:\\d\\d[AP]M)\\s+"
                                     "(<DIR>|\\d+)\\s+(\\S.*)$"));
    if (dosPattern.indexIn(bufferStr) == 0) {
        _q_parseDosDir(dosPattern.capturedTexts(), userName, info);
        return true;
    }

    // Unsupported
    return false;
}

void WFtpDTP::socketConnected()
{
    bytesDone = 0;
#if defined(QFTPDTP_DEBUG)
    qDebug("WFtpDTP::connectState(CsConnected)");
#endif
    emit connectState(WFtpDTP::CsConnected);
}

void WFtpDTP::socketReadyRead()
{
    if (!socket)
        return;

    if (pi->currentCommand().isEmpty()) {
        socket->close();
#if defined(QFTPDTP_DEBUG)
        qDebug("WFtpDTP::connectState(CsClosed)");
#endif
        emit connectState(WFtpDTP::CsClosed);
        return;
    }

    if (pi->abortState == WFtpPI::AbortStarted) {
        // discard data
        socket->readAll();
        return;
    }

    if (pi->currentCommand().startsWith(QLatin1String("LIST"))) {
        while (socket->canReadLine()) {
            QUrlInfo i;
            QByteArray line = socket->readLine();
#if defined(QFTPDTP_DEBUG)
            qDebug("WFtpDTP read (list): '%s'", line.constData());
#endif
            if (parseDir(line, QLatin1String(""), &i, codec)) {
                emit listInfo(i);
            } else {
                // some FTP servers don't return a 550 if the file or directory
                // does not exist, but rather write a text to the data socket
                // -- try to catch these cases
                if (line.endsWith("No such file or directory\r\n"))
                    err = QString::fromLatin1(line);
            }
        }
    } else {
        if (!is_ba && data.dev) {
            do {
                QByteArray ba;
                ba.resize(socket->bytesAvailable());
                qint64 bytesRead = socket->read(ba.data(), ba.size());
                if (bytesRead < 0) {
                    // a read following a readyRead() signal will
                    // never fail.
                    return;
                }
                ba.resize(bytesRead);
                bytesDone += bytesRead;
#if defined(QFTPDTP_DEBUG)
                qDebug("WFtpDTP read: %lli bytes (total %lli bytes)", bytesRead, bytesDone);
#endif
                if (data.dev)       // make sure it wasn't deleted in the slot
                    data.dev->write(ba);
                emit dataTransferProgress(bytesDone, bytesTotal);

                // Need to loop; dataTransferProgress is often connected to
                // slots that update the GUI (e.g., progress bar values), and
                // if events are processed, more data may have arrived.
            } while (socket->bytesAvailable());
        } else {
#if defined(QFTPDTP_DEBUG)
            qDebug("WFtpDTP readyRead: %lli bytes available (total %lli bytes read)",
                   bytesAvailable(), bytesDone);
#endif
            emit dataTransferProgress(bytesDone+socket->bytesAvailable(), bytesTotal);
            emit readyRead();
        }
    }
}

void WFtpDTP::socketError(QAbstractSocket::SocketError e)
{
    if (e == QTcpSocket::HostNotFoundError) {
#if defined(QFTPDTP_DEBUG)
        qDebug("WFtpDTP::connectState(CsHostNotFound)");
#endif
        emit connectState(WFtpDTP::CsHostNotFound);
    } else if (e == QTcpSocket::ConnectionRefusedError) {
#if defined(QFTPDTP_DEBUG)
        qDebug("WFtpDTP::connectState(CsConnectionRefused)");
#endif
        emit connectState(WFtpDTP::CsConnectionRefused);
    }
}

void WFtpDTP::socketConnectionClosed()
{
    if (!is_ba && data.dev) {
        clearData();
    }

    bytesFromSocket = socket->readAll();
#if defined(QFTPDTP_DEBUG)
    qDebug("WFtpDTP::connectState(CsClosed)");
#endif
    emit connectState(WFtpDTP::CsClosed);
}

void WFtpDTP::socketBytesWritten(qint64 bytes)
{
    bytesDone += bytes;
#if defined(QFTPDTP_DEBUG)
    qDebug("WFtpDTP::bytesWritten(%lli)", bytesDone);
#endif
    emit dataTransferProgress(bytesDone, bytesTotal);
    if (callWriteData)
        writeData();
}

void WFtpDTP::setupSocket()
{
    socket = listener.nextPendingConnection();
    socket->setObjectName(QLatin1String("WFtpDTP Active state socket"));
    connect(socket, SIGNAL(connected()), SLOT(socketConnected()));
    connect(socket, SIGNAL(readyRead()), SLOT(socketReadyRead()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(socketError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(disconnected()), SLOT(socketConnectionClosed()));
    connect(socket, SIGNAL(bytesWritten(qint64)), SLOT(socketBytesWritten(qint64)));

    listener.close();
}

void WFtpDTP::clearData()
{
    is_ba = false;
    data.dev = 0;
}

QTextCodec* WFtpDTP::textCodec()
{
	return codec;
}

void WFtpDTP::setTextCodec(QTextCodec* codec)
{
	if (!codec) {
		codec = QTextCodec::codecForLocale();
	}
	this->codec = codec;
}

void WFtpDTP::setTextCodec(const char* name)
{
	setTextCodec(QTextCodec::codecForName(name));
}

/**********************************************************************
 *
 * WFtpPI implemenatation
 *
 *********************************************************************/
WFtpPI::WFtpPI(QObject *parent) :
    QObject(parent),
    rawCommand(false),
    transferConnectionExtended(true),
    dtp(this),
    commandSocket(0),
    state(Begin), abortState(None),
    currentCmd(QString()),
    waitForDtpToConnect(false),
    waitForDtpToClose(false)
{
    commandSocket.setObjectName(QLatin1String("WFtpPI_socket"));
    connect(&commandSocket, SIGNAL(hostFound()),
            SLOT(hostFound()));
    connect(&commandSocket, SIGNAL(connected()),
            SLOT(connected()));
    connect(&commandSocket, SIGNAL(disconnected()),
            SLOT(connectionClosed()));
    connect(&commandSocket, SIGNAL(readyRead()),
            SLOT(readyRead()));
    connect(&commandSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            SLOT(error(QAbstractSocket::SocketError)));

    connect(&dtp, SIGNAL(connectState(int)),
             SLOT(dtpConnectState(int)));
}

void WFtpPI::connectToHost(const QString &host, quint16 port)
{
    emit connectState(WFtp::HostLookup);
#ifndef QT_NO_BEARERMANAGEMENT
    //copy network session down to the socket & DTP
    commandSocket.setProperty("_q_networksession", property("_q_networksession"));
    dtp.setProperty("_q_networksession", property("_q_networksession"));
#endif
    commandSocket.connectToHost(host, port);
}

/*
  Sends the sequence of commands \a cmds to the FTP server. When the commands
  are all done the finished() signal is emitted. When an error occurs, the
  error() signal is emitted.

  If there are pending commands in the queue this functions returns false and
  the \a cmds are not added to the queue; otherwise it returns true.
*/
bool WFtpPI::sendCommands(const QStringList &cmds)
{
    if (!pendingCommands.isEmpty())
        return false;

    if (commandSocket.state() != QTcpSocket::ConnectedState || state!=Idle) {
        emit error(WFtp::NotConnected, WFtp::tr("Not connected"));
        return true; // there are no pending commands
    }

    pendingCommands = cmds;
    startNextCmd();
    return true;
}

void WFtpPI::clearPendingCommands()
{
    pendingCommands.clear();
    dtp.abortConnection();
    currentCmd.clear();
    state = Idle;
}

void WFtpPI::abort()
{
    pendingCommands.clear();

    if (abortState != None)
        // ABOR already sent
        return;

    abortState = AbortStarted;
#if defined(QFTPPI_DEBUG)
    qDebug("WFtpPI send: ABOR");
#endif
	QByteArray cmd = dtp.textCodec()->fromUnicode(QString::fromLatin1("ABOR\r\n"));
    commandSocket.write(cmd, 6);

    if (currentCmd.startsWith(QLatin1String("STOR ")))
        dtp.abortConnection();
}

void WFtpPI::hostFound()
{
    emit connectState(WFtp::Connecting);
}

void WFtpPI::connected()
{
    state = Begin;
#if defined(QFTPPI_DEBUG)
//    qDebug("WFtpPI state: %d [connected()]", state);
#endif
    // try to improve performance by setting TCP_NODELAY
    commandSocket.setSocketOption(QAbstractSocket::LowDelayOption, 1);

    emit connectState(WFtp::Connected);
}

void WFtpPI::connectionClosed()
{
    commandSocket.close();
    emit connectState(WFtp::Unconnected);
}

void WFtpPI::delayedCloseFinished()
{
    emit connectState(WFtp::Unconnected);
}

void WFtpPI::error(QAbstractSocket::SocketError e)
{
    if (e == QTcpSocket::HostNotFoundError) {
        emit connectState(WFtp::Unconnected);
        emit error(WFtp::HostNotFound,
                    WFtp::tr("Host %1 not found").arg(commandSocket.peerName()));
    } else if (e == QTcpSocket::ConnectionRefusedError) {
        emit connectState(WFtp::Unconnected);
        emit error(WFtp::ConnectionRefused,
                    WFtp::tr("Connection refused to host %1").arg(commandSocket.peerName()));
    } else if (e == QTcpSocket::SocketTimeoutError) {
        emit connectState(WFtp::Unconnected);
        emit error(WFtp::ConnectionRefused,
                   WFtp::tr("Connection timed out to host %1").arg(commandSocket.peerName()));
    }
}

void WFtpPI::readyRead()
{
    if (waitForDtpToClose)
        return;

    while (commandSocket.canReadLine()) {
        // read line with respect to line continuation
        QString line = QString::fromAscii(commandSocket.readLine());
        if (replyText.isEmpty()) {
            if (line.length() < 3) {
                // protocol error
                return;
            }
            const int lowerLimit[3] = {1,0,0};
            const int upperLimit[3] = {5,5,9};
            for (int i=0; i<3; i++) {
                replyCode[i] = line[i].digitValue();
                if (replyCode[i]<lowerLimit[i] || replyCode[i]>upperLimit[i]) {
                    // protocol error
                    return;
                }
            }
        }
        QString endOfMultiLine;
        endOfMultiLine[0] = '0' + replyCode[0];
        endOfMultiLine[1] = '0' + replyCode[1];
        endOfMultiLine[2] = '0' + replyCode[2];
        endOfMultiLine[3] = QLatin1Char(' ');
        QString lineCont(endOfMultiLine);
        lineCont[3] = QLatin1Char('-');
        QString lineLeft4 = line.left(4);

        while (lineLeft4 != endOfMultiLine) {
            if (lineLeft4 == lineCont)
                replyText += line.mid(4); // strip 'xyz-'
            else
                replyText += line;
            if (!commandSocket.canReadLine())
                return;
            line = QString::fromAscii(commandSocket.readLine());
            lineLeft4 = line.left(4);
        }
        replyText += line.mid(4); // strip reply code 'xyz '
        if (replyText.endsWith(QLatin1String("\r\n")))
            replyText.chop(2);

        if (processReply())
            replyText = QLatin1String("");
    }
}

/*
  Process a reply from the FTP server.

  Returns true if the reply was processed or false if the reply has to be
  processed at a later point.
*/
bool WFtpPI::processReply()
{
#if defined(QFTPPI_DEBUG)
//    qDebug("WFtpPI state: %d [processReply() begin]", state);
    if (replyText.length() < 400)
        qDebug("WFtpPI recv: %d %s", 100*replyCode[0]+10*replyCode[1]+replyCode[2], replyText.toLatin1().constData());
    else
        qDebug("WFtpPI recv: %d (text skipped)", 100*replyCode[0]+10*replyCode[1]+replyCode[2]);
#endif

    int replyCodeInt = 100*replyCode[0] + 10*replyCode[1] + replyCode[2];

    // process 226 replies ("Closing Data Connection") only when the data
    // connection is really closed to avoid short reads of the DTP
    if (replyCodeInt == 226 || (replyCodeInt == 250 && currentCmd.startsWith(QLatin1String("RETR")))) {
        if (dtp.state() != QTcpSocket::UnconnectedState) {
            waitForDtpToClose = true;
            return false;
        }
    }

    switch (abortState) {
        case AbortStarted:
            abortState = WaitForAbortToFinish;
            break;
        case WaitForAbortToFinish:
            abortState = None;
            return true;
        default:
            break;
    }

    // get new state
    static const State table[5] = {
        /* 1yz   2yz      3yz   4yz      5yz */
        Waiting, Success, Idle, Failure, Failure
    };
    switch (state) {
        case Begin:
            if (replyCode[0] == 1) {
                return true;
            } else if (replyCode[0] == 2) {
                state = Idle;
                emit finished(WFtp::tr("Connected to host %1").arg(commandSocket.peerName()));
                break;
            }
            // reply codes not starting with 1 or 2 are not handled.
            return true;
        case Waiting:
            if (static_cast<signed char>(replyCode[0]) < 0 || replyCode[0] > 5)
                state = Failure;
            else
#if defined(Q_OS_IRIX) && defined(Q_CC_GNU)
            {
                // work around a crash on 64 bit gcc IRIX
                State *t = (State *) table;
                state = t[replyCode[0] - 1];
            }
#else
            if (replyCodeInt == 202)
                state = Failure;
            else
                state = table[replyCode[0] - 1];
#endif
            break;
        default:
            // ignore unrequested message
            return true;
    }
#if defined(QFTPPI_DEBUG)
//    qDebug("WFtpPI state: %d [processReply() intermediate]", state);
#endif

    // special actions on certain replies
    emit rawFtpReply(replyCodeInt, replyText);
    if (rawCommand) {
        rawCommand = false;
    } else if (replyCodeInt == 227) {
        // 227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)
        // rfc959 does not define this response precisely, and gives
        // both examples where the parenthesis are used, and where
        // they are missing. We need to scan for the address and host
        // info.
        QRegExp addrPortPattern(QLatin1String("(\\d+),(\\d+),(\\d+),(\\d+),(\\d+),(\\d+)"));
        if (addrPortPattern.indexIn(replyText) == -1) {
#if defined(QFTPPI_DEBUG)
            qDebug("WFtp: bad 227 response -- address and port information missing");
#endif
            // this error should be reported
        } else {
            QStringList lst = addrPortPattern.capturedTexts();
            QString host = lst[1] + QLatin1Char('.') + lst[2] + QLatin1Char('.') + lst[3] + QLatin1Char('.') + lst[4];
            quint16 port = (lst[5].toUInt() << 8) + lst[6].toUInt();
            waitForDtpToConnect = true;
            dtp.connectToHost(host, port);
        }
    } else if (replyCodeInt == 229) {
        // 229 Extended Passive mode OK (|||10982|)
        int portPos = replyText.indexOf(QLatin1Char('('));
        if (portPos == -1) {
#if defined(QFTPPI_DEBUG)
            qDebug("WFtp: bad 229 response -- port information missing");
#endif
            // this error should be reported
        } else {
            ++portPos;
            QChar delimiter = replyText.at(portPos);
            QStringList epsvParameters = replyText.mid(portPos).split(delimiter);

            waitForDtpToConnect = true;
            dtp.connectToHost(commandSocket.peerAddress().toString(),
                              epsvParameters.at(3).toInt());
        }

    } else if (replyCodeInt == 230) {
        if (currentCmd.startsWith(QLatin1String("USER ")) && pendingCommands.count()>0 &&
            pendingCommands.first().startsWith(QLatin1String("PASS "))) {
            // no need to send the PASS -- we are already logged in
            pendingCommands.pop_front();
        }
        // 230 User logged in, proceed.
        emit connectState(WFtp::LoggedIn);
    } else if (replyCodeInt == 213) {
        // 213 File status.
        if (currentCmd.startsWith(QLatin1String("SIZE ")))
            dtp.setBytesTotal(replyText.simplified().toLongLong());
    } else if (replyCode[0]==1 && currentCmd.startsWith(QLatin1String("STOR "))) {
        dtp.waitForConnection();
        dtp.writeData();
    }

    // react on new state
    switch (state) {
        case Begin:
            // should never happen
            break;
        case Success:
            // success handling
            state = Idle;
            // no break!
        case Idle:
            if (dtp.hasError()) {
                emit error(WFtp::UnknownError, dtp.errorMessage());
                dtp.clearError();
            }
            startNextCmd();
            break;
        case Waiting:
            // do nothing
            break;
        case Failure:
            // If the EPSV or EPRT commands fail, replace them with
            // the old PASV and PORT instead and try again.
            if (currentCmd.startsWith(QLatin1String("EPSV"))) {
                transferConnectionExtended = false;
                pendingCommands.prepend(QLatin1String("PASV\r\n"));
            } else if (currentCmd.startsWith(QLatin1String("EPRT"))) {
                transferConnectionExtended = false;
                pendingCommands.prepend(QLatin1String("PORT\r\n"));
            } else {
                emit error(WFtp::UnknownError, replyText);
            }
            if (state != Waiting) {
                state = Idle;
                startNextCmd();
            }
            break;
    }
#if defined(QFTPPI_DEBUG)
//    qDebug("WFtpPI state: %d [processReply() end]", state);
#endif
    return true;
}

/*
  Starts next pending command. Returns false if there are no pending commands,
  otherwise it returns true.
*/
bool WFtpPI::startNextCmd()
{
    if (waitForDtpToConnect)
        // don't process any new commands until we are connected
        return true;

#if defined(QFTPPI_DEBUG)
    if (state != Idle)
        qDebug("WFtpPI startNextCmd: Internal error! WFtpPI called in non-Idle state %d", state);
#endif
    if (pendingCommands.isEmpty()) {
        currentCmd.clear();
        emit finished(replyText);
        return false;
    }
    currentCmd = pendingCommands.first();

    // PORT and PASV are edited in-place, depending on whether we
    // should try the extended transfer connection commands EPRT and
    // EPSV. The PORT command also triggers setting up a listener, and
    // the address/port arguments are edited in.
    QHostAddress address = commandSocket.localAddress();
    if (currentCmd.startsWith(QLatin1String("PORT"))) {
        if ((address.protocol() == QTcpSocket::IPv6Protocol) && transferConnectionExtended) {
            int port = dtp.setupListener(address);
            currentCmd = QLatin1String("EPRT |");
            currentCmd += (address.protocol() == QTcpSocket::IPv4Protocol) ? QLatin1Char('1') : QLatin1Char('2');
            currentCmd += QLatin1Char('|') + address.toString() + QLatin1Char('|') + QString::number(port);
            currentCmd += QLatin1Char('|');
        } else if (address.protocol() == QTcpSocket::IPv4Protocol) {
            int port = dtp.setupListener(address);
            QString portArg;
            quint32 ip = address.toIPv4Address();
            portArg += QString::number((ip & 0xff000000) >> 24);
            portArg += QLatin1Char(',') + QString::number((ip & 0xff0000) >> 16);
            portArg += QLatin1Char(',') + QString::number((ip & 0xff00) >> 8);
            portArg += QLatin1Char(',') + QString::number(ip & 0xff);
            portArg += QLatin1Char(',') + QString::number((port & 0xff00) >> 8);
            portArg += QLatin1Char(',') + QString::number(port & 0xff);

            currentCmd = QLatin1String("PORT ");
            currentCmd += portArg;
        } else {
            // No IPv6 connection can be set up with the PORT
            // command.
            return false;
        }

        currentCmd += QLatin1String("\r\n");
    } else if (currentCmd.startsWith(QLatin1String("PASV"))) {
        if ((address.protocol() == QTcpSocket::IPv6Protocol) && transferConnectionExtended)
            currentCmd = QLatin1String("EPSV\r\n");
    }

    pendingCommands.pop_front();
#if defined(QFTPPI_DEBUG)
    qDebug("WFtpPI send: %s", currentCmd.left(currentCmd.length()-2).toLatin1().constData());
#endif
    state = Waiting;
	QByteArray currentCmdEncoded = dtp.textCodec()->fromUnicode(currentCmd);
    commandSocket.write(currentCmdEncoded);
    return true;
}

void WFtpPI::dtpConnectState(int s)
{
    switch (s) {
        case WFtpDTP::CsClosed:
            if (waitForDtpToClose) {
                // there is an unprocessed reply
                if (processReply())
                    replyText = QLatin1String("");
                else
                    return;
            }
            waitForDtpToClose = false;
            readyRead();
            return;
        case WFtpDTP::CsConnected:
            waitForDtpToConnect = false;
            startNextCmd();
            return;
        case WFtpDTP::CsHostNotFound:
        case WFtpDTP::CsConnectionRefused:
            emit error(WFtp::ConnectionRefused,
                        WFtp::tr("Connection refused for data connection"));
            startNextCmd();
            return;
        default:
            return;
    }
}

int WFtpPrivate::addCommand(WFtpCommand *cmd)
{
    pending.append(cmd);

    if (pending.count() == 1) {
        // don't emit the commandStarted() signal before the ID is returned
        QTimer::singleShot(0, q_func(), SLOT(_q_startNextCommand()));
    }
    return cmd->id;
}

/**********************************************************************
 *
 * WFtp implementation
 *
 *********************************************************************/
/*!
    \class WFtp
    \brief The WFtp class provides an implementation of the client side of FTP protocol.

    \ingroup network
    \inmodule QtNetwork


    This class provides a direct interface to FTP that allows you to
    have more control over the requests. However, for new
    applications, it is recommended to use QNetworkAccessManager and
    QNetworkReply, as those classes possess a simpler, yet more
    powerful API.

    The class works asynchronously, so there are no blocking
    functions. If an operation cannot be executed immediately, the
    function will still return straight away and the operation will be
    scheduled for later execution. The results of scheduled operations
    are reported via signals. This approach depends on the event loop
    being in operation.

    The operations that can be scheduled (they are called "commands"
    in the rest of the documentation) are the following:
    connectToHost(), login(), close(), list(), cd(), get(), put(),
    remove(), mkdir(), rmdir(), rename() and rawCommand().

    All of these commands return a unique identifier that allows you
    to keep track of the command that is currently being executed.
    When the execution of a command starts, the commandStarted()
    signal with the command's identifier is emitted. When the command
    is finished, the commandFinished() signal is emitted with the
    command's identifier and a bool that indicates whether the command
    finished with an error.

    In some cases, you might want to execute a sequence of commands,
    e.g. if you want to connect and login to a FTP server. This is
    simply achieved:

    \snippet doc/src/snippets/code/src_network_access_wftp.cpp 0

    In this case two FTP commands have been scheduled. When the last
    scheduled command has finished, a done() signal is emitted with
    a bool argument that tells you whether the sequence finished with
    an error.

    If an error occurs during the execution of one of the commands in
    a sequence of commands, all the pending commands (i.e. scheduled,
    but not yet executed commands) are cleared and no signals are
    emitted for them.

    Some commands, e.g. list(), emit additional signals to report
    their results.

    Example: If you want to download the INSTALL file from the Qt
    FTP server, you would write this:

    \snippet doc/src/snippets/code/src_network_access_wftp.cpp 1

    For this example the following sequence of signals is emitted
    (with small variations, depending on network traffic, etc.):

    \snippet doc/src/snippets/code/src_network_access_wftp.cpp 2

    The dataTransferProgress() signal in the above example is useful
    if you want to show a \link QProgressBar progress bar \endlink to
    inform the user about the progress of the download. The
    readyRead() signal tells you that there is data ready to be read.
    The amount of data can be queried then with the bytesAvailable()
    function and it can be read with the read() or readAll()
    function.

    If the login fails for the above example, the signals would look
    like this:

    \snippet doc/src/snippets/code/src_network_access_wftp.cpp 3

    You can then get details about the error with the error() and
    errorString() functions.

    For file transfer, WFtp can use both active or passive mode, and
    it uses passive file transfer mode by default; see the
    documentation for setTransferMode() for more details about this.

    Call setProxy() to make WFtp connect via an FTP proxy server.

    The functions currentId() and currentCommand() provide more
    information about the currently executing command.

    The functions hasPendingCommands() and clearPendingCommands()
    allow you to query and clear the list of pending commands.

    If you are an experienced network programmer and want to have
    complete control you can use rawCommand() to execute arbitrary FTP
    commands.

    \warning The current version of WFtp doesn't fully support
    non-Unix FTP servers.

    \sa QNetworkAccessManager, QNetworkRequest, QNetworkReply,
        {FTP Example}
*/


/*!
    Constructs a WFtp object with the given \a parent.
*/
WFtp::WFtp(QObject *parent)
    : QObject(*new WFtpPrivate, parent)
{
    Q_D(WFtp);
    d->errorString = tr("Unknown error");

    connect(&d->pi, SIGNAL(connectState(int)),
            SLOT(_q_piConnectState(int)));
    connect(&d->pi, SIGNAL(finished(QString)),
            SLOT(_q_piFinished(QString)));
    connect(&d->pi, SIGNAL(error(int,QString)),
            SLOT(_q_piError(int,QString)));
    connect(&d->pi, SIGNAL(rawFtpReply(int,QString)),
            SLOT(_q_piFtpReply(int,QString)));

    connect(&d->pi.dtp, SIGNAL(readyRead()),
            SIGNAL(readyRead()));
    connect(&d->pi.dtp, SIGNAL(dataTransferProgress(qint64,qint64)),
            SIGNAL(dataTransferProgress(qint64,qint64)));
    connect(&d->pi.dtp, SIGNAL(listInfo(QUrlInfo)),
            SIGNAL(listInfo(QUrlInfo)));
}

#ifdef QT3_SUPPORT
/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
WFtp::WFtp(QObject *parent, const char *name)
    : QObject(*new WFtpPrivate, parent)
{
    Q_D(WFtp);
    setObjectName(QLatin1String(name));
    d->errorString = tr("Unknown error");

    connect(&d->pi, SIGNAL(connectState(int)),
            SLOT(_q_piConnectState(int)));
    connect(&d->pi, SIGNAL(finished(QString)),
            SLOT(_q_piFinished(QString)));
    connect(&d->pi, SIGNAL(error(int,QString)),
            SLOT(_q_piError(int,QString)));
    connect(&d->pi, SIGNAL(rawFtpReply(int,QString)),
            SLOT(_q_piFtpReply(int,QString)));

    connect(&d->pi.dtp, SIGNAL(readyRead()),
            SIGNAL(readyRead()));
    connect(&d->pi.dtp, SIGNAL(dataTransferProgress(qint64,qint64)),
            SIGNAL(dataTransferProgress(qint64,qint64)));
    connect(&d->pi.dtp, SIGNAL(listInfo(QUrlInfo)),
            SIGNAL(listInfo(QUrlInfo)));
}
#endif

/*!
    \enum WFtp::State

    This enum defines the connection state:

    \value Unconnected There is no connection to the host.
    \value HostLookup A host name lookup is in progress.
    \value Connecting An attempt to connect to the host is in progress.
    \value Connected Connection to the host has been achieved.
    \value LoggedIn Connection and user login have been achieved.
    \value Closing The connection is closing down, but it is not yet
    closed. (The state will be \c Unconnected when the connection is
    closed.)

    \sa stateChanged() state()
*/
/*!
    \enum WFtp::TransferMode

    FTP works with two socket connections; one for commands and
    another for transmitting data. While the command connection is
    always initiated by the client, the second connection can be
    initiated by either the client or the server.

    This enum defines whether the client (Passive mode) or the server
    (Active mode) should set up the data connection.

    \value Passive The client connects to the server to transmit its
    data.

    \value Active The server connects to the client to transmit its
    data.
*/
/*!
    \enum WFtp::TransferType

    This enum identifies the data transfer type used with get and
    put commands.

    \value Binary The data will be transferred in Binary mode.

    \value Ascii The data will be transferred in Ascii mode and new line
    characters will be converted to the local format.
*/
/*!
    \enum WFtp::Error

    This enum identifies the error that occurred.

    \value NoError No error occurred.
    \value HostNotFound The host name lookup failed.
    \value ConnectionRefused The server refused the connection.
    \value NotConnected Tried to send a command, but there is no connection to
    a server.
    \value UnknownError An error other than those specified above
    occurred.

    \sa error()
*/

/*!
    \enum WFtp::Command

    This enum is used as the return value for the currentCommand() function.
    This allows you to perform specific actions for particular
    commands, e.g. in a FTP client, you might want to clear the
    directory view when a list() command is started; in this case you
    can simply check in the slot connected to the start() signal if
    the currentCommand() is \c List.

    \value None No command is being executed.
    \value SetTransferMode set the \link TransferMode transfer\endlink mode.
    \value SetProxy switch proxying on or off.
    \value ConnectToHost connectToHost() is being executed.
    \value Login login() is being executed.
    \value Close close() is being executed.
    \value List list() is being executed.
    \value Cd cd() is being executed.
    \value Get get() is being executed.
    \value Put put() is being executed.
    \value Remove remove() is being executed.
    \value Mkdir mkdir() is being executed.
    \value Rmdir rmdir() is being executed.
    \value Rename rename() is being executed.
    \value RawCommand rawCommand() is being executed.

    \sa currentCommand()
*/

/*!
    \fn void WFtp::stateChanged(int state)

    This signal is emitted when the state of the connection changes.
    The argument \a state is the new state of the connection; it is
    one of the \l State values.

    It is usually emitted in response to a connectToHost() or close()
    command, but it can also be emitted "spontaneously", e.g. when the
    server closes the connection unexpectedly.

    \sa connectToHost() close() state() State
*/

/*!
    \fn void WFtp::listInfo(const QUrlInfo &i);

    This signal is emitted for each directory entry the list() command
    finds. The details of the entry are stored in \a i.

    \sa list()
*/

/*!
    \fn void WFtp::commandStarted(int id)

    This signal is emitted when processing the command identified by
    \a id starts.

    \sa commandFinished() done()
*/

/*!
    \fn void WFtp::commandFinished(int id, bool error)

    This signal is emitted when processing the command identified by
    \a id has finished. \a error is true if an error occurred during
    the processing; otherwise \a error is false.

    \sa commandStarted() done() error() errorString()
*/

/*!
    \fn void WFtp::done(bool error)

    This signal is emitted when the last pending command has finished;
    (it is emitted after the last command's commandFinished() signal).
    \a error is true if an error occurred during the processing;
    otherwise \a error is false.

    \sa commandFinished() error() errorString()
*/

/*!
    \fn void WFtp::readyRead()

    This signal is emitted in response to a get() command when there
    is new data to read.

    If you specify a device as the second argument in the get()
    command, this signal is \e not emitted; instead the data is
    written directly to the device.

    You can read the data with the readAll() or read() functions.

    This signal is useful if you want to process the data in chunks as
    soon as it becomes available. If you are only interested in the
    complete data, just connect to the commandFinished() signal and
    read the data then instead.

    \sa get() read() readAll() bytesAvailable()
*/

/*!
    \fn void WFtp::dataTransferProgress(qint64 done, qint64 total)

    This signal is emitted in response to a get() or put() request to
    indicate the current progress of the download or upload.

    \a done is the amount of data that has already been transferred
    and \a total is the total amount of data to be read or written. It
    is possible that the WFtp class is not able to determine the total
    amount of data that should be transferred, in which case \a total
    is 0. (If you connect this signal to a QProgressBar, the progress
    bar shows a busy indicator if the total is 0).

    \warning \a done and \a total are not necessarily the size in
    bytes, since for large files these values might need to be
    "scaled" to avoid overflow.

    \sa get(), put(), QProgressBar
*/

/*!
    \fn void WFtp::rawCommandReply(int replyCode, const QString &detail);

    This signal is emitted in response to the rawCommand() function.
    \a replyCode is the 3 digit reply code and \a detail is the text
    that follows the reply code.

    \sa rawCommand()
*/

/*!
    Connects to the FTP server \a host using port \a port.

    The stateChanged() signal is emitted when the state of the
    connecting process changes, e.g. to \c HostLookup, then \c
    Connecting, then \c Connected.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa stateChanged() commandStarted() commandFinished()
*/
int WFtp::connectToHost(const QString &host, quint16 port)
{
    QStringList cmds;
    cmds << host;
    cmds << QString::number((uint)port);
    int id = d_func()->addCommand(new WFtpCommand(ConnectToHost, cmds));
    d_func()->pi.transferConnectionExtended = true;
    return id;
}

/*!
    Logs in to the FTP server with the username \a user and the
    password \a password.

    The stateChanged() signal is emitted when the state of the
    connecting process changes, e.g. to \c LoggedIn.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int WFtp::login(const QString &user, const QString &password)
{
    QStringList cmds;
    cmds << (QLatin1String("USER ") + (user.isNull() ? QLatin1String("anonymous") : user) + QLatin1String("\r\n"));
    cmds << (QLatin1String("PASS ") + (password.isNull() ? QLatin1String("anonymous@") : password) + QLatin1String("\r\n"));
    return d_func()->addCommand(new WFtpCommand(Login, cmds));
}

/*!
    Closes the connection to the FTP server.

    The stateChanged() signal is emitted when the state of the
    connecting process changes, e.g. to \c Closing, then \c
    Unconnected.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa stateChanged() commandStarted() commandFinished()
*/
int WFtp::close()
{
    return d_func()->addCommand(new WFtpCommand(Close, QStringList(QLatin1String("QUIT\r\n"))));
}

/*!
    Sets the current FTP transfer mode to \a mode. The default is WFtp::Passive.

    \sa WFtp::TransferMode
*/
int WFtp::setTransferMode(TransferMode mode)
{
    int id = d_func()->addCommand(new WFtpCommand(SetTransferMode, QStringList()));
    d_func()->pi.transferConnectionExtended = true;
    d_func()->transferMode = mode;
    return id;
}

/*!
    Enables use of the FTP proxy on host \a host and port \a
    port. Calling this function with \a host empty disables proxying.

    WFtp does not support FTP-over-HTTP proxy servers. Use
    QNetworkAccessManager for this.
*/
int WFtp::setProxy(const QString &host, quint16 port)
{
    QStringList args;
    args << host << QString::number(port);
    return d_func()->addCommand(new WFtpCommand(SetProxy, args));
}

/*!
    Lists the contents of directory \a dir on the FTP server. If \a
    dir is empty, it lists the contents of the current directory.

    The listInfo() signal is emitted for each directory entry found.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa listInfo() commandStarted() commandFinished()
*/
int WFtp::list(const QString &dir)
{
    QStringList cmds;
    cmds << QLatin1String("TYPE A\r\n");
    cmds << QLatin1String(d_func()->transferMode == Passive ? "PASV\r\n" : "PORT\r\n");
    if (dir.isEmpty())
        cmds << QLatin1String("LIST\r\n");
    else
        cmds << (QLatin1String("LIST ") + dir + QLatin1String("\r\n"));
    return d_func()->addCommand(new WFtpCommand(List, cmds));
}

/*!
    Changes the working directory of the server to \a dir.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int WFtp::cd(const QString &dir)
{
    return d_func()->addCommand(new WFtpCommand(Cd, QStringList(QLatin1String("CWD ") + dir + QLatin1String("\r\n"))));
}

/*!
    Downloads the file \a file from the server.

    If \a dev is 0, then the readyRead() signal is emitted when there
    is data available to read. You can then read the data with the
    read() or readAll() functions.

    If \a dev is not 0, the data is written directly to the device \a
    dev. Make sure that the \a dev pointer is valid for the duration
    of the operation (it is safe to delete it when the
    commandFinished() signal is emitted). In this case the readyRead()
    signal is \e not emitted and you cannot read data with the
    read() or readAll() functions.

    If you don't read the data immediately it becomes available, i.e.
    when the readyRead() signal is emitted, it is still available
    until the next command is started.

    For example, if you want to present the data to the user as soon
    as there is something available, connect to the readyRead() signal
    and read the data immediately. On the other hand, if you only want
    to work with the complete data, you can connect to the
    commandFinished() signal and read the data when the get() command
    is finished.

    The data is transferred as Binary or Ascii depending on the value
    of \a type.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa readyRead() dataTransferProgress() commandStarted()
    commandFinished()
*/
int WFtp::get(const QString &file, QIODevice *dev, TransferType type)
{
    QStringList cmds;
    if (type == Binary)
        cmds << QLatin1String("TYPE I\r\n");
    else
        cmds << QLatin1String("TYPE A\r\n");
    cmds << QLatin1String("SIZE ") + file + QLatin1String("\r\n");
    cmds << QLatin1String(d_func()->transferMode == Passive ? "PASV\r\n" : "PORT\r\n");
    cmds << QLatin1String("RETR ") + file + QLatin1String("\r\n");
    return d_func()->addCommand(new WFtpCommand(Get, cmds, dev));
}

/*!
    \overload

    Writes a copy of the given \a data to the file called \a file on
    the server. The progress of the upload is reported by the
    dataTransferProgress() signal.

    The data is transferred as Binary or Ascii depending on the value
    of \a type.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    Since this function takes a copy of the \a data, you can discard
    your own copy when this function returns.

    \sa dataTransferProgress() commandStarted() commandFinished()
*/
int WFtp::put(const QByteArray &data, const QString &file, TransferType type)
{
    QStringList cmds;
    if (type == Binary)
        cmds << QLatin1String("TYPE I\r\n");
    else
        cmds << QLatin1String("TYPE A\r\n");
    cmds << QLatin1String(d_func()->transferMode == Passive ? "PASV\r\n" : "PORT\r\n");
    cmds << QLatin1String("ALLO ") + QString::number(data.size()) + QLatin1String("\r\n");
    cmds << QLatin1String("STOR ") + file + QLatin1String("\r\n");
    return d_func()->addCommand(new WFtpCommand(Put, cmds, data));
}

/*!
    Reads the data from the IO device \a dev, and writes it to the
    file called \a file on the server. The data is read in chunks from
    the IO device, so this overload allows you to transmit large
    amounts of data without the need to read all the data into memory
    at once.

    The data is transferred as Binary or Ascii depending on the value
    of \a type.

    Make sure that the \a dev pointer is valid for the duration of the
    operation (it is safe to delete it when the commandFinished() is
    emitted).
*/
int WFtp::put(QIODevice *dev, const QString &file, TransferType type)
{
    QStringList cmds;
    if (type == Binary)
        cmds << QLatin1String("TYPE I\r\n");
    else
        cmds << QLatin1String("TYPE A\r\n");
    cmds << QLatin1String(d_func()->transferMode == Passive ? "PASV\r\n" : "PORT\r\n");
    if (!dev->isSequential())
        cmds << QLatin1String("ALLO ") + QString::number(dev->size()) + QLatin1String("\r\n");
    cmds << QLatin1String("STOR ") + file + QLatin1String("\r\n");
    return d_func()->addCommand(new WFtpCommand(Put, cmds, dev));
}

/*!
    Deletes the file called \a file from the server.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int WFtp::remove(const QString &file)
{
    return d_func()->addCommand(new WFtpCommand(Remove, QStringList(QLatin1String("DELE ") + file + QLatin1String("\r\n"))));
}

/*!
    Creates a directory called \a dir on the server.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int WFtp::mkdir(const QString &dir)
{
    return d_func()->addCommand(new WFtpCommand(Mkdir, QStringList(QLatin1String("MKD ") + dir + QLatin1String("\r\n"))));
}

/*!
    Removes the directory called \a dir from the server.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int WFtp::rmdir(const QString &dir)
{
    return d_func()->addCommand(new WFtpCommand(Rmdir, QStringList(QLatin1String("RMD ") + dir + QLatin1String("\r\n"))));
}

/*!
    Renames the file called \a oldname to \a newname on the server.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int WFtp::rename(const QString &oldname, const QString &newname)
{
    QStringList cmds;
    cmds << QLatin1String("RNFR ") + oldname + QLatin1String("\r\n");
    cmds << QLatin1String("RNTO ") + newname + QLatin1String("\r\n");
    return d_func()->addCommand(new WFtpCommand(Rename, cmds));
}

/*!
    Sends the raw FTP command \a command to the FTP server. This is
    useful for low-level FTP access. If the operation you wish to
    perform has an equivalent WFtp function, we recommend using the
    function instead of raw FTP commands since the functions are
    easier and safer.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa rawCommandReply() commandStarted() commandFinished()
*/
int WFtp::rawCommand(const QString &command)
{
    QString cmd = command.trimmed() + QLatin1String("\r\n");
    return d_func()->addCommand(new WFtpCommand(RawCommand, QStringList(cmd)));
}

QTextCodec* WFtp::textCodec()
{
	return d_func()->pi.dtp.textCodec();
}

void WFtp::setTextCodec(QTextCodec* codec)
{
	d_func()->pi.dtp.setTextCodec(codec);
}

void WFtp::setTextCodec(const char* name)
{
	d_func()->pi.dtp.setTextCodec(name);
}

/*!
    Returns the number of bytes that can be read from the data socket
    at the moment.

    \sa get() readyRead() read() readAll()
*/
qint64 WFtp::bytesAvailable() const
{
    return d_func()->pi.dtp.bytesAvailable();
}

/*! \fn qint64 WFtp::readBlock(char *data, quint64 maxlen)

    Use read() instead.
*/

/*!
    Reads \a maxlen bytes from the data socket into \a data and
    returns the number of bytes read. Returns -1 if an error occurred.

    \sa get() readyRead() bytesAvailable() readAll()
*/
qint64 WFtp::read(char *data, qint64 maxlen)
{
    return d_func()->pi.dtp.read(data, maxlen);
}

/*!
    Reads all the bytes available from the data socket and returns
    them.

    \sa get() readyRead() bytesAvailable() read()
*/
QByteArray WFtp::readAll()
{
    return d_func()->pi.dtp.readAll();
}

/*!
    Aborts the current command and deletes all scheduled commands.

    If there is an unfinished command (i.e. a command for which the
    commandStarted() signal has been emitted, but for which the
    commandFinished() signal has not been emitted), this function
    sends an \c ABORT command to the server. When the server replies
    that the command is aborted, the commandFinished() signal with the
    \c error argument set to \c true is emitted for the command. Due
    to timing issues, it is possible that the command had already
    finished before the abort request reached the server, in which
    case, the commandFinished() signal is emitted with the \c error
    argument set to \c false.

    For all other commands that are affected by the abort(), no
    signals are emitted.

    If you don't start further FTP commands directly after the
    abort(), there won't be any scheduled commands and the done()
    signal is emitted.

    \warning Some FTP servers, for example the BSD FTP daemon (version
    0.3), wrongly return a positive reply even when an abort has
    occurred. For these servers the commandFinished() signal has its
    error flag set to \c false, even though the command did not
    complete successfully.

    \sa clearPendingCommands()
*/
void WFtp::abort()
{
    if (d_func()->pending.isEmpty())
        return;

    clearPendingCommands();
    d_func()->pi.abort();
}

/*!
    Returns the identifier of the FTP command that is being executed
    or 0 if there is no command being executed.

    \sa currentCommand()
*/
int WFtp::currentId() const
{
    if (d_func()->pending.isEmpty())
        return 0;
    return d_func()->pending.first()->id;
}

/*!
    Returns the command type of the FTP command being executed or \c
    None if there is no command being executed.

    \sa currentId()
*/
WFtp::Command WFtp::currentCommand() const
{
    if (d_func()->pending.isEmpty())
        return None;
    return d_func()->pending.first()->command;
}

/*!
    Returns the QIODevice pointer that is used by the FTP command to read data
    from or store data to. If there is no current FTP command being executed or
    if the command does not use an IO device, this function returns 0.

    This function can be used to delete the QIODevice in the slot connected to
    the commandFinished() signal.

    \sa get() put()
*/
QIODevice* WFtp::currentDevice() const
{
    if (d_func()->pending.isEmpty())
        return 0;
    WFtpCommand *c = d_func()->pending.first();
    if (c->is_ba)
        return 0;
    return c->data.dev;
}

/*!
    Returns true if there are any commands scheduled that have not yet
    been executed; otherwise returns false.

    The command that is being executed is \e not considered as a
    scheduled command.

    \sa clearPendingCommands() currentId() currentCommand()
*/
bool WFtp::hasPendingCommands() const
{
    return d_func()->pending.count() > 1;
}

/*!
    Deletes all pending commands from the list of scheduled commands.
    This does not affect the command that is being executed. If you
    want to stop this as well, use abort().

    \sa hasPendingCommands() abort()
*/
void WFtp::clearPendingCommands()
{
    // delete all entires except the first one
    while (d_func()->pending.count() > 1)
        delete d_func()->pending.takeLast();
}

/*!
    Returns the current state of the object. When the state changes,
    the stateChanged() signal is emitted.

    \sa State stateChanged()
*/
WFtp::State WFtp::state() const
{
    return d_func()->state;
}

/*!
    Returns the last error that occurred. This is useful to find out
    what went wrong when receiving a commandFinished() or a done()
    signal with the \c error argument set to \c true.

    If you start a new command, the error status is reset to \c NoError.
*/
WFtp::Error WFtp::error() const
{
    return d_func()->error;
}

/*!
    Returns a human-readable description of the last error that
    occurred. This is useful for presenting a error message to the
    user when receiving a commandFinished() or a done() signal with
    the \c error argument set to \c true.

    The error string is often (but not always) the reply from the
    server, so it is not always possible to translate the string. If
    the message comes from Qt, the string has already passed through
    tr().
*/
QString WFtp::errorString() const
{
    return d_func()->errorString;
}

/*! \internal
*/
void WFtpPrivate::_q_startNextCommand()
{
    Q_Q(WFtp);
    if (pending.isEmpty())
        return;
    WFtpCommand *c = pending.first();

    error = WFtp::NoError;
    errorString = QT_TRANSLATE_NOOP(WFtp, QLatin1String("Unknown error"));

    if (q->bytesAvailable())
        q->readAll(); // clear the data
    emit q->commandStarted(c->id);

    // Proxy support, replace the Login argument in place, then fall
    // through.
    if (c->command == WFtp::Login && !proxyHost.isEmpty()) {
        QString loginString = c->rawCmds.first().trimmed();
        loginString += QLatin1Char('@') + host;
        if (port && port != 21)
            loginString += QLatin1Char(':') + QString::number(port);
        loginString += QLatin1String("\r\n");
        c->rawCmds[0] = loginString;
    }

    if (c->command == WFtp::SetTransferMode) {
        _q_piFinished(QLatin1String("Transfer mode set"));
    } else if (c->command == WFtp::SetProxy) {
        proxyHost = c->rawCmds[0];
        proxyPort = c->rawCmds[1].toUInt();
        c->rawCmds.clear();
        _q_piFinished(QLatin1String("Proxy set to ") + proxyHost + QLatin1Char(':') + QString::number(proxyPort));
    } else if (c->command == WFtp::ConnectToHost) {
#ifndef QT_NO_BEARERMANAGEMENT
        //copy network session down to the PI
        pi.setProperty("_q_networksession", q->property("_q_networksession"));
#endif
        if (!proxyHost.isEmpty()) {
            host = c->rawCmds[0];
            port = c->rawCmds[1].toUInt();
            pi.connectToHost(proxyHost, proxyPort);
        } else {
            pi.connectToHost(c->rawCmds[0], c->rawCmds[1].toUInt());
        }
    } else {
        if (c->command == WFtp::Put) {
            if (c->is_ba) {
                pi.dtp.setData(c->data.ba);
                pi.dtp.setBytesTotal(c->data.ba->size());
            } else if (c->data.dev && (c->data.dev->isOpen() || c->data.dev->open(QIODevice::ReadOnly))) {
                pi.dtp.setDevice(c->data.dev);
                if (c->data.dev->isSequential()) {
                    pi.dtp.setBytesTotal(0);
                    pi.dtp.connect(c->data.dev, SIGNAL(readyRead()), SLOT(dataReadyRead()));
                    pi.dtp.connect(c->data.dev, SIGNAL(readChannelFinished()), SLOT(dataReadyRead()));
                } else {
                    pi.dtp.setBytesTotal(c->data.dev->size());
                }
            }
        } else if (c->command == WFtp::Get) {
            if (!c->is_ba && c->data.dev) {
                pi.dtp.setDevice(c->data.dev);
            }
        } else if (c->command == WFtp::Close) {
            state = WFtp::Closing;
            emit q->stateChanged(state);
        }
        pi.sendCommands(c->rawCmds);
    }
}

/*! \internal
*/
void WFtpPrivate::_q_piFinished(const QString&)
{
    if (pending.isEmpty())
        return;
    WFtpCommand *c = pending.first();

    if (c->command == WFtp::Close) {
        // The order of in which the slots are called is arbitrary, so
        // disconnect the SIGNAL-SIGNAL temporary to make sure that we
        // don't get the commandFinished() signal before the stateChanged()
        // signal.
        if (state != WFtp::Unconnected) {
            close_waitForStateChange = true;
            return;
        }
    }
    emit q_func()->commandFinished(c->id, false);
    pending.removeFirst();

    delete c;

    if (pending.isEmpty()) {
        emit q_func()->done(false);
    } else {
        _q_startNextCommand();
    }
}

/*! \internal
*/
void WFtpPrivate::_q_piError(int errorCode, const QString &text)
{
    Q_Q(WFtp);

    if (pending.isEmpty()) {
        qWarning("WFtpPrivate::_q_piError was called without pending command!");
        return;
    }

    WFtpCommand *c = pending.first();

    // non-fatal errors
    if (c->command == WFtp::Get && pi.currentCommand().startsWith(QLatin1String("SIZE "))) {
        pi.dtp.setBytesTotal(0);
        return;
    } else if (c->command==WFtp::Put && pi.currentCommand().startsWith(QLatin1String("ALLO "))) {
        return;
    }

    error = WFtp::Error(errorCode);
    switch (q->currentCommand()) {
        case WFtp::ConnectToHost:
            errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("WFtp", "Connecting to host failed:\n%1"))
                          .arg(text);
            break;
        case WFtp::Login:
            errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("WFtp", "Login failed:\n%1"))
                          .arg(text);
            break;
        case WFtp::List:
            errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("WFtp", "Listing directory failed:\n%1"))
                          .arg(text);
            break;
        case WFtp::Cd:
            errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("WFtp", "Changing directory failed:\n%1"))
                          .arg(text);
            break;
        case WFtp::Get:
            errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("WFtp", "Downloading file failed:\n%1"))
                          .arg(text);
            break;
        case WFtp::Put:
            errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("WFtp", "Uploading file failed:\n%1"))
                          .arg(text);
            break;
        case WFtp::Remove:
            errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("WFtp", "Removing file failed:\n%1"))
                          .arg(text);
            break;
        case WFtp::Mkdir:
            errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("WFtp", "Creating directory failed:\n%1"))
                          .arg(text);
            break;
        case WFtp::Rmdir:
            errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("WFtp", "Removing directory failed:\n%1"))
                          .arg(text);
            break;
        default:
            errorString = text;
            break;
    }

    pi.clearPendingCommands();
    q->clearPendingCommands();
    emit q->commandFinished(c->id, true);

    pending.removeFirst();
    delete c;
    if (pending.isEmpty())
        emit q->done(true);
    else
        _q_startNextCommand();
}

/*! \internal
*/
void WFtpPrivate::_q_piConnectState(int connectState)
{
    state = WFtp::State(connectState);
    emit q_func()->stateChanged(state);
    if (close_waitForStateChange) {
        close_waitForStateChange = false;
        _q_piFinished(QLatin1String(QT_TRANSLATE_NOOP("WFtp", "Connection closed")));
    }
}

/*! \internal
*/
void WFtpPrivate::_q_piFtpReply(int code, const QString &text)
{
    if (q_func()->currentCommand() == WFtp::RawCommand) {
        pi.rawCommand = true;
        emit q_func()->rawCommandReply(code, text);
    }
}

/*!
    Destructor.
*/
WFtp::~WFtp()
{
    abort();
    close();
}

// #include "wftp.moc"

// #include "moc_wftp.cpp"
