#pragma once

#include <QtEndian>
#include <QIODevice>

class WPacket;
class WPacketModel;

class WPacketIo : public QObject
{
	Q_OBJECT
public:
    WPacketIo(WPacketModel* packetModel,
			  QIODevice* internalIo,
			  QObject* parent=0);
    virtual ~WPacketIo();
	bool send(WPacket& packet);
	bool receive(quint32& packetId, QByteArray& block, int msecs=1000);
	bool receive(QByteArray& block, int msecs=1000);
	QByteArray rawReceive(qint64 maxSize);
	qint64 rawReceive(char* data, qint64 maxSize);
	qint64 bytesAvailable() const;
	QByteArray peek(qint64 maxSize);
	qint64 peek(char* data, qint64 maxSize);
	WPacketModel* packetModel() {
		return m_packetModel;
	}
	QIODevice* io() {
		return m_io;
	}
	const QIODevice* io() const {
		return m_io;
	}
signals:
	void readyBlock(quint32 packetId, const QByteArray& block);
	void failedBlock(quint32 packetId);
	void error(const QString& reason);
private slots:
	void tryReceive();
protected:
	bool tryReceiveOneBlock(quint32& packetId, QByteArray& block);

private:
	WPacketModel* m_packetModel;
	QIODevice* m_io;
};

