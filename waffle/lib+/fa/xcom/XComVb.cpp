#include "XComVb.h"

namespace {

// XCom은 0이 아닌 경우가 "실패"
const int NO_OK = -1;

}


XComVb::XComVb(QWidget *parent, Qt::WindowFlags f)
	: EXCOMVBLib::XComVb(parent,f)
{
#define INIT_VARS(n)							\
	for (int i = 0; i < (n); ++i) {				\
		vars##n << 0;								\
	}
	INIT_VARS(2);
	INIT_VARS(3);
	INIT_VARS(4);
	INIT_VARS(5);
	INIT_VARS(6);
}

// void XComVb::LoadSecsMsg(int& msgId, int& devId, int& stream, int& function, int& systemBytes, int& wbit)
// {
// 	int nr = NO_OK;
// 	QVariant vr = this->dynamicCall("LoadSecsMsg(long&,short&,short&,short&,long&,short&)",
// 									vars6);
// 	if (vr.isValid()) {
// 		msgId = vars[0].toInt();
// 		devId = vars[1].toInt();
// 		stream = vars[2].toInt();
// 		function = vars[3].toInt();
// 		systemBytes = vars[4].toInt();
// 		wbit = vars[5].toInt();
// 		nr = vr.toInt();
// 	}
// 	return nr;
// }

// void XComVb::GetListArray(int lMsgId, int& pnItems)
// {
// 	int nr = NO_OK;
// 	var2[0] = lMsgId;
// 	// var2[1] = pnItems;
// 	QVariant vr = this->dynamicCall("GetListArray(long,short&)", var2);
// 	if (vr.isValid()) {
// 		pnItems = var2[1];
// 	}
// 	return nr;
// }

// void XComVb::SetBinaryArray(int lMsgId, short* pnValue, int lCount)
// {
// 	int nr = NO_OK;
// 	QVariant vr = this->dynamicCall("SetBinaryArray(long,short&,long)",
// 									lMsgId,
// }

QList<bool> XComVb::BoolArray()
{
	QList<bool> arr;
	int count = this->ItemCount();
	for (int i = 0; i < count; ++i) {
		arr << this->Bool ();
	}
	return arr;
}

QByteArray XComVb::BinaryArray()
{
	QByteArray arr;
	int count = this->ItemCount();
	arr.resize(count);
	for (int i = 0; i < count; ++i) {
		arr[i] = (char)this->Binary();
	}
	return arr;
}

QList<double> XComVb::F8Array()
{
	QList<double> arr;
	int count = this->ItemCount();
	for (int i = 0; i < count; ++i) {
		arr << this->F8();
	}
	return arr;
}

QList<double> XComVb::U4Array()
{
	QList<double> arr;
	int count = this->ItemCount();
	for (int i = 0; i < count; ++i) {
		arr << this->U4();
	}
	return arr;
}

QList<double> XComVb::U8Array()
{
	QList<double> arr;
	int count = this->ItemCount();
	for (int i = 0; i < count; ++i) {
		arr << this->U8();
	}
	return arr;
}

QList<float> XComVb::F4Array()
{
	QList<float> arr;
	int count = this->ItemCount();
	for (int i = 0; i < count; ++i) {
		arr << this->F4();
	}
	return arr;
}

QList<long> XComVb::I4Array()
{
	QList<long> arr;
	int count = this->ItemCount();
	for (int i = 0; i < count; ++i) {
		arr << this->I4();
	}
	return arr;
}

QList<long> XComVb::I8Array()
{
	QList<long> arr;
	int count = this->ItemCount();
	for (int i = 0; i < count; ++i) {
		arr << this->I8();
	}
	return arr;
}

QList<long> XComVb::U2Array()
{
	QList<long> arr;
	int count = this->ItemCount();
	for (int i = 0; i < count; ++i) {
		arr << this->U2();
	}
	return arr;
}

QList<short> XComVb::I1Array()
{
	QList<short> arr;
	int count = this->ItemCount();
	for (int i = 0; i < count; ++i) {
		arr << this->I1();
	}
	return arr;
}

QList<short> XComVb::I2Array()
{
	QList<short> arr;
	int count = this->ItemCount();
	for (int i = 0; i < count; ++i) {
		arr << this->I2();
	}
	return arr;
}

QList<short> XComVb::U1Array()
{
	QList<short> arr;
	int count = this->ItemCount();
	for (int i = 0; i < count; ++i) {
		arr << this->U1();
	}
	return arr;
}

void XComVb::SetBoolArray(bool* valueArray, int count)
{
	this->SetItemCount(count);
	for (int i = 0; i < count; ++i) {
		this->SetBool(valueArray[i]);
	}
}


// NOTE: not `int*' but 'char*'!
void XComVb::SetBinaryArray(char* valueArray, int count)
{
	this->SetItemCount(count);
	for (int i = 0; i < count; ++i) {
		this->SetBinary((int)valueArray[i]);
	}
}

void XComVb::SetU1Array(short* valueArray, int count)
{
	this->SetItemCount(count);
	for (int i = 0; i < count; ++i) {
		this->SetU1(valueArray[i]);
	}
}

void XComVb::SetU2Array(long* valueArray, int count)
{
	this->SetItemCount(count);
	for (int i = 0; i < count; ++i) {
		this->SetU2(valueArray[i]);
	}
}

void XComVb::SetU4Array(double* valueArray, int count)
{
	this->SetItemCount(count);
	for (int i = 0; i < count; ++i) {
		this->SetU4(valueArray[i]);
	}
}

void XComVb::SetU8Array(double* valueArray, int count)
{
	this->SetItemCount(count);
	for (int i = 0; i < count; ++i) {
		this->SetU8(valueArray[i]);
	}
}

void XComVb::SetI1Array(short* valueArray, int count)
{
	this->SetItemCount(count);
	for (int i = 0; i < count; ++i) {
		this->SetI1(valueArray[i]);
	}
}

void XComVb::SetI2Array(short* valueArray, int count)
{
	this->SetItemCount(count);
	for (int i = 0; i < count; ++i) {
		this->SetI2(valueArray[i]);
	}
}

void XComVb::SetI4Array(long* valueArray, int count)
{
	this->SetItemCount(count);
	for (int i = 0; i < count; ++i) {
		this->SetI4(valueArray[i]);
	}
}

void XComVb::SetI8Array(long* valueArray, int count)
{
	this->SetItemCount(count);
	for (int i = 0; i < count; ++i) {
		this->SetI8(valueArray[i]);
	}
}

void XComVb::SetF4Array(float* valueArray, int count)
{
	this->SetItemCount(count);
	for (int i = 0; i < count; ++i) {
		this->SetF4(valueArray[i]);
	}
}

void XComVb::SetF8Array(double* valueArray, int count)
{
	this->SetItemCount(count);
	for (int i = 0; i < count; ++i) {
		this->SetF8(valueArray[i]);
	}
}

void XComVb::SetBinaryArray(const QByteArray& valueArray)
{
	this->SetItemCount(valueArray.length());
	for (int i = 0; i < valueArray.length(); ++i) {
		this->SetBinary(valueArray[i]);
	}
}

void XComVb::SetBoolArray(const QList<bool>& valueArray)
{
	this->SetItemCount(valueArray.length());
	for (int i = 0; i < valueArray.length(); ++i) {
		this->SetBool(valueArray[i]);
	}
}

void XComVb::SetF4Array(const QList<float>& valueArray)
{
	this->SetItemCount(valueArray.length());
	for (int i = 0; i < valueArray.length(); ++i) {
		this->SetF4(valueArray[i]);
	}
}

void XComVb::SetF8Array(const QList<double>& valueArray)
{
	this->SetItemCount(valueArray.length());
	for (int i = 0; i < valueArray.length(); ++i) {
		this->SetF8(valueArray[i]);
	}
}

void XComVb::SetI1Array(const QList<short>& valueArray)
{
	this->SetItemCount(valueArray.length());
	for (int i = 0; i < valueArray.length(); ++i) {
		this->SetI1(valueArray[i]);
	}
}

void XComVb::SetI2Array(const QList<short>& valueArray)
{
	this->SetItemCount(valueArray.length());
	for (int i = 0; i < valueArray.length(); ++i) {
		this->SetI2(valueArray[i]);
	}
}

void XComVb::SetI4Array(const QList<long>& valueArray)
{
	this->SetItemCount(valueArray.length());
	for (int i = 0; i < valueArray.length(); ++i) {
		this->SetI4(valueArray[i]);
	}
}

void XComVb::SetI8Array(const QList<long>& valueArray)
{
	this->SetItemCount(valueArray.length());
	for (int i = 0; i < valueArray.length(); ++i) {
		this->SetI8(valueArray[i]);
	}
}

void XComVb::SetU1Array(const QList<short>& valueArray)
{
	this->SetItemCount(valueArray.length());
	for (int i = 0; i < valueArray.length(); ++i) {
		this->SetU1(valueArray[i]);
	}
}

void XComVb::SetU2Array(const QList<long>& valueArray)
{
	this->SetItemCount(valueArray.length());
	for (int i = 0; i < valueArray.length(); ++i) {
		this->SetU2(valueArray[i]);
	}
}

void XComVb::SetU4Array(const QList<double>& valueArray)
{
	this->SetItemCount(valueArray.length());
	for (int i = 0; i < valueArray.length(); ++i) {
		this->SetU4(valueArray[i]);
	}
}

void XComVb::SetU8Array(const QList<double>& valueArray)
{
	this->SetItemCount(valueArray.length());
	for (int i = 0; i < valueArray.length(); ++i) {
		this->SetU8(valueArray[i]);
	}
}
