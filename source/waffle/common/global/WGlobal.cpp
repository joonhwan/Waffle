#include "wglobal.h"

QString wQuoted(const QString& body)
{
	return QString() + QChar('\"') + body + QChar('\"');
}

QString wSingleQuoted(const QString& body)
{
	return QString() + QChar('\'') + body + QChar('\'');
}



