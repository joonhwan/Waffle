#include "XCom.h"
#include "common/activex/WDispatchDriver.h"
#include <qaxtypes.h>
#include <limits.h>
#include <QUuid>

namespace {

// XCom은 0이 아닌 경우가 "실패"
const int XCOM_OK = 0;
const short XCOM_INTERNAL_ERROR = SHRT_MIN;

short fromVariant(VARIANT* rv)
{
	QVariant qv;
	if (rv) {
		qv = VARIANTToQVariant(*rv, 0);
	}
	return qv.isValid() ? qv.toInt() : XCOM_INTERNAL_ERROR;
}

}


XCom::XCom(QWidget *parent, Qt::WindowFlags f)
	: EXCOMLib::eXCom(parent,f)
	, m_driver(new WDispatchDriver)
{
	if (!isNull()) {
		IDispatch* disp = 0;
		QUuid id("{00020400-0000-0000-C000-000000000046}");
		HRESULT hr = this->queryInterface(id, (void**)&disp);
		if (S_OK==hr && disp) {
			m_driver->Attach(disp);
			disp->Release();
		}
	}
}

//virtual
XCom::~XCom()
{
	delete m_driver;
	m_driver = 0;
}

#define IMPL_FUNC(NAME, TYPE)											\
	TYPE XCom::NAME(int msgId, short* check) {							\
		TYPE value = TYPE(0);											\
		int countReceviced = 0;											\
		short sv = NAME##Array(msgId, &value, 1, &countReceviced);		\
        if (check) *check = sv;													\
		return value;													\
	}																	\
	QVector<TYPE> XCom::NAME##Array(int msgId, short* check)			\
	{																	\
		int count = this->GetCurrentItemCount(msgId);					\
		QVector<TYPE> dataList(count);									\
		short sv = NAME##Array(msgId, dataList);						\
        if (check) *check = sv;													\
        return dataList;														\
	}																	\
	short XCom::NAME##Array(int msgId, QVector<TYPE>& dataList)			\
	{																	\
		long _lMsgId = msgId;											\
		TYPE* _pValue = dataList.data();								\
		long _lRealCount = 0;											\
		long _lArrCount = dataList.size();								\
		VARIANT* rv = m_driver->InvokeMethod("Get" #NAME "Item",		\
											 _lMsgId,					\
											 _pValue,					\
											 &_lRealCount,				\
											 _lArrCount);				\
		short sv = fromVariant(rv);										\
		if (sv < 0) {													\
			dataList.clear();											\
			handleError(sv, "Set" #NAME "()" );							\
		} else {														\
			dataList.resize(_lRealCount);								\
		}																\
		return sv;														\
	}																	\
	short XCom::NAME##Array(int msgId, TYPE* dataList,					\
							int dataListSize,							\
							int* dataCount)								\
	{																	\
		long _lMsgId = msgId;											\
		TYPE* _pValue = dataList;										\
		long _lRealCount = 0;											\
		long _lArrCount = dataListSize;									\
		short sv														\
			= fromVariant(m_driver->InvokeMethod("Get" #NAME "Item",	\
												 _lMsgId,				\
												 _pValue,				\
												 &_lRealCount,			\
												 _lArrCount));			\
		if (sv < 0) {													\
			if (dataCount) *dataCount = 0;								\
			handleError(sv, "Set" #NAME "()" );							\
		} else {														\
			if (dataCount) *dataCount = _lRealCount;					\
		}																\
		return sv;														\
	}																	\
	short XCom::Set##NAME##Array(int msgId,								\
								 const QVector<TYPE>& dataList)			\
	{																	\
		long _lMsgId = msgId;											\
		TYPE* _pValue = const_cast<TYPE*>(dataList.data());				\
		long _lArrCount = dataList.size();								\
		VARIANT* rv = m_driver->InvokeMethod("Set" #NAME "Item",		\
											 _lMsgId,					\
											 _pValue,					\
											 _lArrCount);				\
		short sv = fromVariant(rv);										\
		if (XCOM_OK!=sv) {												\
			handleError(sv, "Set" #NAME "()" );							\
		}																\
		return sv;														\
	}																	\
	short XCom::Set##NAME##Array(int msgId, TYPE* dataList, int count)	\
	{																	\
		long _lMsgId = msgId;											\
		long _lArrCount = count;										\
		VARIANT* rv = m_driver->InvokeMethod("Set" #NAME "Item",		\
											 _lMsgId,					\
											 dataList,					\
											 count);					\
		short sv = fromVariant(rv);										\
		if (XCOM_OK!=sv) {												\
			handleError(sv, "Set" #NAME "()");							\
		}																\
		return sv;														\
	}																	\
	short XCom::Set##NAME(int msgId, TYPE value)						\
	{																	\
		return Set##NAME##Array(msgId, &value, 1);						\
	}																	\

// QVector<short> XCom::BoolArray(int msgId)
// {
// 	int count = this->GetCurrentItemCount(msgId);
// 	QVector<short> dataList(count);
// 	long _lMsgId = msgId;
// 	short* _pValue = dataList.data();
// 	long _lRealCount = 0;
// 	long _lArrCount = dataList.size();
// 	VARIANT* rv = m_driver->InvokeMethod("Get" "Bool" "Item",
// 										 _lMsgId,
// 										 _pValue,
// 										 &_lRealCount,
// 										 _lArrCount);
// 	short sv = fromVariant(rv);
// 	if (XCOM_OK!=sv) {
// 		dataList.clear();
// 		handleError(sv, "Bool" "Array()" );
// 	} else {
// 		dataList.resize(_lRealCount);
// 	}
// 	return dataList;
// }
// short XCom::BoolArray(int msgId, QVector<short>& dataList)
// {
// 	long _lMsgId = msgId;
// 	short* _pValue = dataList.data();
// 	long _lRealCount = 0;
// 	long _lArrCount = dataList.size();
// 	VARIANT* rv = m_driver->InvokeMethod("Get" "Bool" "Item",
// 										 _lMsgId,
// 										 _pValue,
// 										 &_lRealCount,
// 										 _lArrCount);
// 	short sv = fromVariant(rv);
// 	if (XCOM_OK!=sv) {
// 		dataList.clear();
// 		handleError(sv, "Set" "Bool" "()" );
// 	} else {
// 		dataList.resize(_lRealCount);
// 	}
// }
// short XCom::SetBoolArray(int msgId,
// 							 const QVector<short>& dataList)
// {
// 	long _lMsgId = msgId;
// 	short* _pValue = const_cast<short*>(dataList.constData());
// 	long _lArrCount = dataList.size();
// 	VARIANT* rv = m_driver->InvokeMethod("Set" "Bool" "Item",
// 										 _lMsgId,
// 										 _pValue,
// 										 _lArrCount);
// 	short sv = fromVariant(rv);
// 	if (XCOM_OK!=sv) {
// 		handleError(sv, "Set" "Bool" "()" );
// 	}
// }

IMPL_FUNC(Bool, short)
IMPL_FUNC(Binary, short)
IMPL_FUNC(I1, short)
IMPL_FUNC(I2, short)
IMPL_FUNC(I4, long)
IMPL_FUNC(I8, long)
IMPL_FUNC(U1, short)
IMPL_FUNC(U2, long)
IMPL_FUNC(U4, double)
IMPL_FUNC(U8, double)
IMPL_FUNC(F4, float)
IMPL_FUNC(F8, double)

short XCom::LoadSecsMsg(int* msgId, int* deviceId,
						int* stream, int* func,
						int* sysByte, int* wbit )
{
	long plMsgId = 0;
	short pnDevId = 0;
	short pnStrm = 0;
	short pnFunc = 0;
	long plSysByte = 0;
	short pnWbi = 0;
	short sv = fromVariant(m_driver->InvokeMethod("LoadSecsMsg",
												  &plMsgId,
												  &pnDevId,
												  &pnStrm,
												  &pnFunc,
												  &plSysByte,
												  &pnWbi));
	if (sv>=0) {
		if (msgId) *msgId = plMsgId;
		if (deviceId) *deviceId = pnDevId;
		if (stream) *stream = pnStrm;
		if (pnFunc) *func = pnFunc;
		if (sysByte) *sysByte = plSysByte;
		if (wbit) *wbit = pnWbi;
	}
	return sv;
}

short XCom::MakeSecsMsg(int* msgId, int deviceId,
						int stream, int func,
						int sysByte)
{
	long _lMsgId = 0;
	short nDevId = (short)deviceId;
	short nStrm = (short)stream;
	short nFunc = (short)func;
	long lSysByte = (long)sysByte;
	short sv = fromVariant(m_driver->InvokeMethod("MakeSecsMsg",
												  &_lMsgId,
												  nDevId,
												  nStrm,
												  nFunc,
												  lSysByte));
	if (XCOM_OK==sv) {
		if (msgId) *msgId = _lMsgId;
	} else {
		handleError(sv, "MakeSecsMsg()");
	}
	return sv;
}

short XCom::SetListCount(int msgId, int count)
{
	long _lMsgid = msgId;
	short _sCount = count;
	short sv = fromVariant(m_driver->InvokeMethod("SetListItem", _lMsgid, _sCount));
	if (XCOM_OK!=sv) {
		handleError(sv, "SetListCount()");
	}
	return sv;
}

short XCom::SetAscii(int msgId, const QString& value)
{
	long _lMsgid = msgId;
	QByteArray _ansiValue = value.toLocal8Bit();
	long _lCount = _ansiValue.length();
	short sv = fromVariant(m_driver->InvokeMethod("SetAsciiItem",
												  _lMsgid,
												  _ansiValue.data(),
												  _lCount));
	if (XCOM_OK!=sv) {
		handleError(sv, "SetAscii()");
	}
	return sv;
}

short XCom::SetJis8(int msgId, const QString& value)
{
	long _lMsgid = msgId;
	QByteArray _ansiValue = value.toLocal8Bit();
	long _lCount = _ansiValue.length();
	short sv = fromVariant(m_driver->InvokeMethod("SetJis8Item",
												  _lMsgid,
												  _ansiValue.data(),
												  _lCount));
	if (XCOM_OK!=sv) {
		handleError(sv, "SetAscii()");
	}
	return sv;
}

int XCom::ListCount(int msgId, int* nextItemFormat)
{
	int value = 0;
	long _lMsgId = msgId;
	short pnItems = 0;
	short sv = fromVariant(m_driver->InvokeMethod("GetListItem",
												  _lMsgId,
												  &pnItems));
	if (sv < 0) {
		handleError(sv, "ListCount()");
		pnItems = 0;
	} else {
		if (nextItemFormat) {
			*nextItemFormat = sv;
		}
	}

	return pnItems;
}

QString XCom::Ascii(int msgId)
{
	QString val;
	int length = 0;
	this->GetAsciiItem(msgId, val, length);
	return val;
}

QString XCom::Jis8(int msgId)
{
	QString val;
	int length = 0;
	this->GetJis8Item(msgId, val, length);
	return val;
}

void XCom::handleError(int errorCode, const char* context)
{
}
