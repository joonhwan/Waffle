#pragma once

#include "qextserial/qextserialenumerator.h"
#include "qextserial/qextserialport.h"
#include <QObject>
#include <QMetaType>

// for QVariant'ing
Q_DECLARE_METATYPE(QextPortInfo);

// for easy enumerator handling
class QextSerialPortEnums : public QObject
{
	Q_OBJECT
	Q_ENUMS(BaudRateType)
	Q_ENUMS(DataBitsType)
	Q_ENUMS(ParityType)
	Q_ENUMS(StopBitsType)
	Q_ENUMS(FlowType)
public:
	enum BaudRateType
	{
#if defined(Q_OS_UNIX) || defined(qdoc)
		BAUD50 = 50,                //POSIX ONLY
		BAUD75 = 75,                //POSIX ONLY
		BAUD134 = 134,              //POSIX ONLY
		BAUD150 = 150,              //POSIX ONLY
		BAUD200 = 200,              //POSIX ONLY
		BAUD1800 = 1800,            //POSIX ONLY
#  if defined(B76800) || defined(qdoc)
		BAUD76800 = 76800,          //POSIX ONLY
#  endif
#  if (defined(B230400) && defined(B4000000)) || defined(qdoc)
		BAUD230400 = 230400,        //POSIX ONLY
		BAUD460800 = 460800,        //POSIX ONLY
		BAUD500000 = 500000,        //POSIX ONLY
		BAUD576000 = 576000,        //POSIX ONLY
		BAUD921600 = 921600,        //POSIX ONLY
		BAUD1000000 = 1000000,      //POSIX ONLY
		BAUD1152000 = 1152000,      //POSIX ONLY
		BAUD1500000 = 1500000,      //POSIX ONLY
		BAUD2000000 = 2000000,      //POSIX ONLY
		BAUD2500000 = 2500000,      //POSIX ONLY
		BAUD3000000 = 3000000,      //POSIX ONLY
		BAUD3500000 = 3500000,      //POSIX ONLY
		BAUD4000000 = 4000000,      //POSIX ONLY
#  endif
#endif //Q_OS_UNIX
#if defined(Q_OS_WIN) || defined(qdoc)
		BAUD14400 = 14400,          //WINDOWS ONLY
		BAUD56000 = 56000,          //WINDOWS ONLY
		BAUD128000 = 128000,        //WINDOWS ONLY
		BAUD256000 = 256000,        //WINDOWS ONLY
#endif  //Q_OS_WIN
		BAUD110 = 110,
		BAUD300 = 300,
		BAUD600 = 600,
		BAUD1200 = 1200,
		BAUD2400 = 2400,
		BAUD4800 = 4800,
		BAUD9600 = 9600,
		BAUD19200 = 19200,
		BAUD38400 = 38400,
		BAUD57600 = 57600,
		BAUD115200 = 115200
	};

	enum DataBitsType
	{
		DATA_5 = 5,
		DATA_6 = 6,
		DATA_7 = 7,
		DATA_8 = 8
	};

	enum ParityType
	{
		PAR_NONE,
		PAR_ODD,
		PAR_EVEN,
#if defined(Q_OS_WIN) || defined(qdoc)
		PAR_MARK,               //WINDOWS ONLY
#endif
		PAR_SPACE
	};

	enum StopBitsType
	{
		STOP_1,
#if defined(Q_OS_WIN) || defined(qdoc)
		STOP_1_5,               //WINDOWS ONLY
#endif
		STOP_2
	};

	enum FlowType
	{
		FLOW_OFF,
		FLOW_HARDWARE,
		FLOW_XONXOFF
	};

	QextSerialPortEnums() {
	}
};


class WSerialPort : public QextSerialPort
{
	Q_OBJECT
public:
	WSerialPort(QObject* parent=0);
	virtual ~WSerialPort();
};

