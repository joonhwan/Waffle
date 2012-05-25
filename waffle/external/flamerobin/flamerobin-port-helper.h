#pragma once

#include <QString>
#include <QFileInfo>

namespace fr {

namespace helper {

enum stripType {
	leading = 0x1,
	trailing,
	both,
};

enum replaceMethod {
	replaceFirstOnly,
	replaceAny,
};

QString rangedString(const QChar* s, const QChar* e);
QString stripString(const QString& s, stripType method = trailing);
QString subString(const QString& s, int from, int to);
QString& padString(QString& s, int count, QChar pad = QLatin1Char(' '), bool fromRight = true);
QString& replaceString(QString& s, const QString& before, const QString& after,
					   replaceMethod method=replaceAny, Qt::CaseSensitivity cs=Qt::CaseSensitive);
QString filePathWithExtension(const QFileInfo& pathInfo, const QString& extension);

} // namespace helper

} // namespace fr

