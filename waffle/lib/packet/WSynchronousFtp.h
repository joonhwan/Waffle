#pragma once

#include <QDir>
#include <QObject>
#include <QStringList>
#include <QUrlInfo>

class WFtp;

class WSynchronousFtp : public QObject
{
	Q_OBJECT
public:
    WSynchronousFtp(QObject* parent=0);
    virtual ~WSynchronousFtp();

	bool open();
	bool close();
	bool createRemoteDir(const QString& dir);
	bool cd(const QString& dir);
	bool fileListAt(const QString& dir = QString());
	bool fileExists(const QString& fileFullPath);
	bool dirExists(const QString& dirFullPath);
	bool downloadFile(const QString& remoteFilePath);
	bool uploadFile(const QString& localFilePath,
					const QString& remoteFilePath);
	void setTextCodec(const char* codecName);
	void setHost(const QString& hostName, quint16 port=21) {
		m_hostName = hostName;
		m_port = port;
	}
	void setUser(const QString& user) {
		m_user = user;
	}
	void setPassword(const QString& password) {
		m_password = password;
	}
	void setTimeoutMsec(int msec) {
		m_timeOutMsec = msec;
	}
	void setCaseSensitivity(Qt::CaseSensitivity v) {
		m_caseSensitivity = v;
	}
	QStringList lastFileList() {
		return m_lastFileList;
	}
private slots:
	void onCommandStarted(int id);
	void onCommandFinished(int id, bool error);
	void onStateChanged(int state);
	void addToList(QUrlInfo url);
	void onRawCommandReply(int replyCode, const QString& detail);
protected:
	bool waitForDone(int id, int timeOutMsec);
	bool waitForRawCommand(int expectedResponseId, int timeOutMsec);

	WFtp* m_ftp;
	QString m_hostName;
	quint16 m_port;
	QString m_user;
	QString m_password;
	int m_openTimeOutMsec;
	int m_timeOutMsec;

private:
	Qt::CaseSensitivity m_caseSensitivity;
	int m_lastId;
	bool m_lastError;
	int m_lastRawCommandReplayCode;
	QDir m_lastListDir;
	QStringList m_lastDirList;
	QStringList m_lastFileList;
};
