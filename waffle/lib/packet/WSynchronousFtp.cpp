#include "WSynchronousFtp.h"
#include "WFtp.h"
#include "common/global/WQtSignalWaiter.h"
#include <QDebug>
#include <QElapsedTimer>

// #define WSYNCHRONOUSFTP_DEBUG

namespace {

const int OPTS_UTF8_OK = 200;

QString cmdIdToName(int id)
{
	QString s = "Unknown";
	switch (id) {
	case WFtp::None:
		s = "None";
		break;
	case WFtp::SetTransferMode:
		s = "SetTransferMode";
		break;
	case WFtp::SetProxy:
		s = "SetProxy";
		break;
	case WFtp::ConnectToHost:
		s = "ConnectToHost";
		break;
	case WFtp::Login:
		s = "Login";
		break;
	case WFtp::Close:
		s = "Close";
		break;
	case WFtp::List:
		s = "List";
		break;
	case WFtp::Cd:
		s = "Cd";
		break;
	case WFtp::Get:
		s = "Get";
		break;
	case WFtp::Put:
		s = "Put";
		break;
	case WFtp::Remove:
		s = "Remove";
		break;
	case WFtp::Mkdir:
		s = "Mkdir";
		break;
	case WFtp::Rmdir:
		s = "Rmdir";
		break;
	case WFtp::Rename:
		s = "Rename";
		break;
	case WFtp::RawCommand:
		s = "RawCommand";
		break;
	}
	return s;
};

QString stateToName(int state)
{
	QString s = "Unknown";
	switch (state) {
	case WFtp::Unconnected:
		s = "Unconnected";
		break;
	case WFtp::HostLookup:
		s = "HostLookup";
		break;
	case WFtp::Connecting:
		s = "Connecting";
		break;
	case WFtp::Connected:
		s = "Connected";
		break;
	case WFtp::LoggedIn:
		s = "LoggedIn";
		break;
	case WFtp::Closing:
		s = "Closing";
		break;
	}
	return s;
}

} // namespace

WSynchronousFtp::WSynchronousFtp(QObject* parent)
	: QObject(parent)
	, m_caseSensitivity(Qt::CaseInsensitive)
	, m_timeOutMsec(5000)
	, m_openTimeOutMsec(10000)
	, m_lastId(-1)
	, m_lastError(false)
{
#ifdef _DEBUG
	m_timeOutMsec = 50000;
	m_openTimeOutMsec = 100000;
#endif
	m_ftp = new WFtp(this);
	connect(m_ftp, SIGNAL(commandStarted(int)), SLOT(onCommandStarted(int)));
	connect(m_ftp, SIGNAL(commandFinished(int,bool)), SLOT(onCommandFinished(int,bool)));
	connect(m_ftp, SIGNAL(listInfo(QUrlInfo)), SLOT(addToList(QUrlInfo)));
	connect(m_ftp, SIGNAL(stateChanged(int)), SLOT(onStateChanged(int)));
	connect(m_ftp, SIGNAL(rawCommandReply(int,const QString&)), SLOT(onRawCommandReply(int,const QString&)));
}

//virtual
WSynchronousFtp::~WSynchronousFtp()
{
}

bool WSynchronousFtp::open()
{
	bool done = false;
	int timeOutMsec = m_openTimeOutMsec;
	do {
		QElapsedTimer timer;
		timer.start();

		if (!waitForDone(m_ftp->connectToHost(m_hostName, m_port),
						 timeOutMsec)) {
			break;
		}
		timeOutMsec = timeOutMsec - timer.elapsed();

		if (!waitForDone(m_ftp->login(m_user, m_password),
						 timeOutMsec)) {
			break;
		}

		timeOutMsec = timeOutMsec - timer.elapsed();

		m_ftp->rawCommand("OPTS UTF8 ON");
		if (waitForRawCommand(OPTS_UTF8_OK, timeOutMsec)) {
			m_ftp->setTextCodec("utf-8");
		} else {
			m_ftp->setTextCodec("euc-kr");//QTextCodec::codecForLocale());
		}

		done = true;
	} while (0);

	return done;
}

bool WSynchronousFtp::close()
{
	return waitForDone(m_ftp->close(), 5000);
}

bool WSynchronousFtp::createRemoteDir(const QString& remoteDir)
{
	bool done = false;
	return done;
}

bool WSynchronousFtp::cd(const QString& dir)
{
	m_lastListDir = QDir(dir);
	return waitForDone(m_ftp->cd(dir), m_timeOutMsec);
}

bool WSynchronousFtp::fileListAt(const QString& dir)
{
	m_lastDirList.clear();
	m_lastFileList.clear();
	if (!dir.isEmpty()) {
		m_lastListDir = QDir(dir);
	}
	return waitForDone(m_ftp->list(dir), m_timeOutMsec);
}

bool WSynchronousFtp::fileExists(const QString& fileFullPath)
{
	bool found = false;
	do {
		QFileInfo fileInfo(fileFullPath);
		QString dirName = fileInfo.dir().path();
		if (!fileListAt(dirName)) {
			break;
		}

		found = m_lastFileList.contains(fileFullPath, m_caseSensitivity);
	} while (0);

	return found;
}

bool WSynchronousFtp::dirExists(const QString& dirFullPath)
{
	bool found = false;
	do {
		QFileInfo fileInfo(dirFullPath);
		QString dirName = fileInfo.dir().path();
		if (!fileListAt(dirName)) {
			break;
		}

		found = m_lastDirList.contains(dirFullPath, m_caseSensitivity);
	} while (0);

	return found;
}

bool WSynchronousFtp::downloadFile(const QString& remoteFilePath)
{
	bool done = false;
	return done;
}

bool WSynchronousFtp::uploadFile(const QString& localFilePath,
								 const QString& remoteFilePath)
{
	bool done = false;
	return done;
}

void WSynchronousFtp::setTextCodec(const char* codecName)
{
	m_ftp->setTextCodec(codecName);
}

void WSynchronousFtp::onCommandStarted(int id)
{
#ifdef WSYNCHRONOUSFTP_DEBUG
	qDebug() << "FTP command started[cmd id=" << id << "]"
			 << " : " << cmdIdToName(m_ftp->currentCommand());
#endif
}

void WSynchronousFtp::onCommandFinished(int id, bool error)
{
#ifdef WSYNCHRONOUSFTP_DEBUG
	qDebug() << "FTP command finished[cmd id=" << id << "]"
			 << " : " << cmdIdToName(m_ftp->currentCommand())
			 << " : " << (error ? "fail" : "succeess");
#endif
	m_lastError = error;
	m_lastId = id;
}

void WSynchronousFtp::onStateChanged(int state)
{
#ifdef WSYNCHRONOUSFTP_DEBUG
	qDebug() << "FTP State : " << stateToName(state);
#endif
}

void WSynchronousFtp::addToList(QUrlInfo url)
{
	QString fullPath =  m_lastListDir.absoluteFilePath(url.name());
	if (url.isDir()) {
		if (url.name()!="." && url.name()!="..") {
			m_lastDirList << fullPath;
		}
	} else {
		m_lastFileList << fullPath;
	}
}

void WSynchronousFtp::onRawCommandReply(int replyCode, const QString& detail)
{
#ifdef WSYNCHRONOUSFTP_DEBUG
	qDebug() << "FTP raw command reply : code=[" << replyCode << "] : "
			 << detail;
#endif
	m_lastRawCommandReplayCode = replyCode;
}

bool WSynchronousFtp::waitForDone(int id, int timeOutMsec)
{
	m_lastId = -1;
	m_lastError = true;
	QElapsedTimer timer;
	timer.start();
	while (m_lastId != id) {
		timeOutMsec = timeOutMsec - timer.elapsed();
		if (timeOutMsec < 0) {
			break;
		}

		if (!WQtSignalWaiter::wait(m_ftp, SIGNAL(commandFinished(int,bool)),
								   timeOutMsec)) {
			// timeout.
			break;
		}
	} while (0);

	return (m_lastId==id) && (!m_lastError);
}

bool WSynchronousFtp::waitForRawCommand(int expectedResponseId, int timeOutMsec)
{
	m_lastRawCommandReplayCode = -1;
	bool done = false;
	if (WQtSignalWaiter::wait(m_ftp, SIGNAL(rawCommandReply(int,const QString&)), timeOutMsec)) {
		done = (expectedResponseId == m_lastRawCommandReplayCode);
	}
	return done;
}


