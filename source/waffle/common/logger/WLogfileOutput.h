#pragma once

#include "common/global/wglobal.h"
#include "common/logger/wlogoutput.h"
#include <QDateTime>
#include <QFile>
#include <QScopedPointer>
#include <QTextStream>


class WFileRollerWatch
{
public:
	virtual QDateTime now(void) const
	{
		return QDateTime::currentDateTime();
	}
};

class WFileChanger
{
public:
	virtual bool changePath(const QString& newPath) = 0;
};

class WFileRoller
{
	WFileRoller(); // no use this ctor
public:
	enum Type
	{
		RollNothing = 0,
		RollHourly,
		RollDaily,
	};
	WFileRoller(const QString& filePath,
			   Type type,
			   WFileRollerWatch* watch = 0);
	virtual ~WFileRoller();
	bool roll(void);
	void setBaseTime(QDateTime& now) {
		m_baseTime = now;
	}
	const QString& currentFilePath(void) const {
		return m_filePath;
	}
	void setFileChanger(WFileChanger* fileChanger) {
		m_fileChanger = fileChanger;
	}
protected:
	Type m_type;
	QDateTime m_baseTime;
	QString m_filePathInput;
	QString m_filePath;
	QScopedPointer<WFileRollerWatch> m_watch;
	WFileChanger* m_fileChanger;

	void updateFilePath(void);
};

class WLogFileOutput : public WLogOutput
					 , public WFileChanger
{
public:
	WLogFileOutput(const QString& filePath);
    WLogFileOutput(WFileRoller* fileRoller);
    virtual ~WLogFileOutput();
	// WLogOutput interfaces
	virtual void write(const WLogEvent& event);
	// WFileChanger interfaces
	virtual bool changePath(const QString& newPath);
protected:
	QScopedPointer<WFileRoller> m_fileRoller;
	QFile m_file;
	QTextStream m_outputStream;
	QDateTime m_lastLoggedTime;

	bool open(const QString& filePath);
};


