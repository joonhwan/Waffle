#include "wlogfileoutput.h"
#include <QFileInfo>
#include <QDir>

namespace
{

const QString fmtDateTime("yyMMdd-hh:mm:ss.zzz");

}


WFileRoller::WFileRoller(const QString& filePath,
						 Type type,
						 WFileRollerWatch* watch)
	: m_type(type)
	, m_fileChanger()
	, m_filePathInput(filePath)
{
	if(!watch) {
		// using default watch
		watch = new WFileRollerWatch();
	}
	m_watch.reset(watch);

	Q_ASSERT(m_watch);
	if(m_watch) {
		m_baseTime = m_watch->now();
	}
	updateFilePath();
}

//virtual
WFileRoller::~WFileRoller()
{
}

bool WFileRoller::roll(void)
{
	bool shouldChange = false;

	do
	{
		Q_ASSERT(m_watch);
		if(!m_watch)
			break;

		QDateTime now = m_watch->now();
		switch(m_type)
		{
		case RollNothing:
			break;
		case RollDaily:
			shouldChange = (m_baseTime.daysTo(now) >= 1);
			break;
		case RollHourly:
			shouldChange = (m_baseTime.time().hour() !=
							now.time().hour());
			break;
		}

	} while(0);

	if(shouldChange) {
		updateFilePath();
	};

	return shouldChange;
}

void WFileRoller::updateFilePath(void)
{
	QFileInfo inputFileInfo(m_filePathInput);
	QString newFilePath = m_filePath;

	switch(m_type)
	{
	case RollNothing:
		// just full path of input file path
		newFilePath = inputFileInfo.absoluteFilePath();
		break;
	case RollDaily:
	case RollHourly:
		{
			QString format = m_type==RollDaily ? "yyMMdd" : "yyMMdd-HH";
			newFilePath = QString("%1/%2-%3.%4")
						  .arg(inputFileInfo.dir().path())
						  .arg(inputFileInfo.completeBaseName())
						  .arg(m_watch->now().toString(format))
						  .arg(inputFileInfo.suffix());
		}
		break;
	}

	if(newFilePath != m_filePath) {
		//Q_ASSERT(m_fileChanger);
		if(m_fileChanger) {
			m_fileChanger->changePath(newFilePath);
		}
	}
	m_filePath = newFilePath;
}

WLogFileOutput::WLogFileOutput(const QString& filePath)
	: m_fileRoller(new WFileRoller(filePath,
								   WFileRoller::RollNothing,
								   new WFileRollerWatch))
{
	m_fileRoller->setFileChanger(this);
	open(filePath);
}

WLogFileOutput::WLogFileOutput(WFileRoller* fileRoller)
	: m_fileRoller(fileRoller)
{
	m_fileRoller->setFileChanger(this);
	open(fileRoller->currentFilePath());
}

//virtual
WLogFileOutput::~WLogFileOutput()
{
}

//virtual
void WLogFileOutput::write(const WLogEvent& event)
{
	m_outputStream << event.logMessage() << endl;

	// always?
	m_outputStream.flush();
}

//virtual
bool WLogFileOutput::changePath(const QString& newPath)
{
	return open(newPath);
}

bool WLogFileOutput::open(const QString& filePath)
{
	bool opened = false;

	do
	{
		if(m_file.isOpen()) {
			QFileInfo newFileInfo(filePath);
			QFileInfo currentFileInfo(m_file);
			if(newFileInfo==currentFileInfo) {
				opened = true;
				break;
			}
		}

		// clean up if open'd
		if(m_file.isOpen()) {
			m_file.close();
			m_outputStream.setDevice(0);
		}

		m_file.setFileName(filePath);
		QIODevice::OpenMode mode = QIODevice::Append |
								   QIODevice::WriteOnly |
								   QIODevice::Text;
		if(!m_file.open(mode)) {
			break;
		}

		m_outputStream.setDevice(&m_file);
		opened = true;
	} while(0);

	return opened;
}

