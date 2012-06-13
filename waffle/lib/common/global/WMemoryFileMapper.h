#ifndef MEMORYFILEMAPPER_H
#define MEMORYFILEMAPPER_H

#include <QObject>

class QFile;

class WMemoryFileMapper : public QObject
{
	Q_OBJECT
public:
	typedef qint64 FileSizeType;
	WMemoryFileMapper(QObject *parent=0);
	virtual ~WMemoryFileMapper();
	bool Open(const QString& filePath);
	bool Close(void);
	FileSizeType Size(void);
	bool CopyContent(FileSizeType offset, FileSizeType length, QByteArray& to);
protected:
	QFile* m_file;
	FileSizeType m_fileSize; //size in bytes of the entire file
};

#endif // MEMORYFILEMAPPER_H
