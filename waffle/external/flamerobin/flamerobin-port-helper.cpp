#include "flamerobin-port-helper.h"
#include <QDir>

namespace fr {

namespace helper {

QString rangedString(const QChar* s, const QChar* e)
{
	Q_ASSERT(s < e);
	if (s < e) {
		return QString(s, e - s + 1);
	} else {
		return QString();
	}
}

QString stripString(const QString& s, helper::stripType method)
{
	int pos = 0;
	switch (method) {
	case leading:
		pos = 0;
		while (pos < s.length() && s.at(pos).isSpace()) {
			++pos;
		}
		return s.mid(pos);
	case trailing:
		pos = s.length() - 1;
		while (pos >= 0 && s.at(pos).isSpace()) {
			--pos;
		}
		return s.left(pos+1);
	case both:
		return stripString(stripString(s, leading), trailing);
	}

	Q_ASSERT(FALSE);
	return s;
}

QString subString(const QString& s, int from, int to)
{
	Q_ASSERT(to >= from);
	if (to >= from) {
		return s.mid(from, to-from+1);
	} else {
		return QString();
	}
}

QString& padString(QString& s, int count, QChar pad, bool fromRight)
{
	if (fromRight) {
		s.append(QString(count, pad));
	} else {
		s.insert(0, QString(count, pad));
	}
	return s;
}

QString filePathWithExtension(const QFileInfo& pathInfo, const QString& extension)
{
	QString v = pathInfo.absolutePath();
	if (!v.endsWith(QDir::separator())) {
		v += QDir::separator();
	}
	QChar dot = QChar::fromLatin1('.');
	if (extension.startsWith(dot)) {
		v += pathInfo.baseName() + extension;
	} else {
		v += pathInfo.baseName() + dot + extension;
	}
	return v;
}

QString& replaceString(QString& s, const QString& before, const QString& after,
					  replaceMethod method, Qt::CaseSensitivity cs)
{
	if (method==replaceAny) {
		s.replace(before, after, cs);
	} else {
		int len = before.length();
		int index = s.indexOf(before, 0, cs);
		if (index >= 0) {
			s.replace(index, len, after);
		}
	}
	return s;
}


} // namespace helper

} // namespace fr
