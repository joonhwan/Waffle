/*
  Copyright (c) 2004-2012 The FlameRobin Development Team

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


  $Id: StringUtils.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
#include <fstream>
#include <sstream>

#include "core/FRError.h"
#include "core/StringUtils.h"
#include "flamerobin-port-helper.h"
#include <QtGlobal>
#include <QStringList>

namespace fr {

//-----------------------------------------------------------------------------
std::string wx2std(const QString& input, QTextCodec* conv)
{
	if (conv) {
		return std::string(conv->fromUnicode(input).constData());
	} else {
		return input.toStdString();
	}

    // if (input.isEmpty())
    //     return "";
    // if (!conv)
    //     conv = wxConvCurrent;
    // const wxWX2MBbuf buf(input.mb_str(*conv));
    // // conversion may fail and return 0, which isn't a safe value to pass
    // // to std:string constructor
    // if (!buf)
    //     return "";
    // return std::string(buf);
}
//-----------------------------------------------------------------------------
QString std2wx(const std::string& input, QTextCodec* conv)
{
	if (conv) {
		return conv->toUnicode(input.c_str());
	} else {
		return QString::fromStdString(input);
	}

    // if (input.isEmpty())
    //     return wxEmptyString;
    // if (!conv)
    //     conv = wxConvCurrent;
    // return QString(input.c_str(), *conv);
}
//-----------------------------------------------------------------------------
QString std2wxIdentifier(const std::string& input, QTextCodec* conv)
{
    // if (input.isEmpty())
    //     return wxEmptyString;
    // if (!conv)
    //     conv = wxConvCurrent;
    // // trim trailing whitespace
    // size_t last = input.find_last_not_of(" ");
    // return QString(input.c_str(), *conv,
    //     (last == std::string::npos) ? std::string::npos : last + 1);
	if (input.empty())
		return QString();

	size_t last = input.find_last_not_of(" ");
	std::string _input(input.c_str(),
					   (last == std::string::npos) ? input.size() : last + 1);
	if (conv) {
		return conv->toUnicode(_input.c_str());
	} else {
		return QString::fromStdString(_input);
	}
}

//-----------------------------------------------------------------------------
QString getHtmlCharset()
{
// #if !wxUSE_UNICODE
//     struct CharsetMapping {
//         wxFontEncoding encoding;
//         const QChar* htmlCS;
//     };
//     static const CharsetMapping mappings[] = {
//         { wxFONTENCODING_ISO8859_1, QLatin1String("ISO-8859-1") },
//         { wxFONTENCODING_ISO8859_2, QLatin1String("ISO-8859-2") },
//         { wxFONTENCODING_ISO8859_3, QLatin1String("ISO-8859-3") },
//         { wxFONTENCODING_ISO8859_4, QLatin1String("ISO-8859-4") },
//         { wxFONTENCODING_ISO8859_5, QLatin1String("ISO-8859-5") },
//         { wxFONTENCODING_ISO8859_6, QLatin1String("ISO-8859-6") },
//         { wxFONTENCODING_ISO8859_7, QLatin1String("ISO-8859-7") },
//         { wxFONTENCODING_ISO8859_8, QLatin1String("ISO-8859-8") },
//         { wxFONTENCODING_ISO8859_9, QLatin1String("ISO-8859-9") },
//         { wxFONTENCODING_ISO8859_10, QLatin1String("ISO-8859-10") },
//         { wxFONTENCODING_ISO8859_11, QLatin1String("ISO-8859-11") },
//         { wxFONTENCODING_ISO8859_12, QLatin1String("ISO-8859-12") },
//         { wxFONTENCODING_ISO8859_13, QLatin1String("ISO-8859-13") },
//         { wxFONTENCODING_ISO8859_14, QLatin1String("ISO-8859-14") },
//         { wxFONTENCODING_ISO8859_15, QLatin1String("ISO-8859-15") },

//         { wxFONTENCODING_CP1250, QLatin1String("windows-1250") },
//         { wxFONTENCODING_CP1251, QLatin1String("windows-1251") },
//         { wxFONTENCODING_CP1252, QLatin1String("windows-1252") },
//         { wxFONTENCODING_CP1253, QLatin1String("windows-1253") },
//         { wxFONTENCODING_CP1254, QLatin1String("windows-1254") },
//         { wxFONTENCODING_CP1255, QLatin1String("windows-1255") },
//         { wxFONTENCODING_CP1256, QLatin1String("windows-1256") },
//         { wxFONTENCODING_CP1257, QLatin1String("windows-1257") }
//     };
//     int mappingCount = sizeof(mappings) / sizeof(CharsetMapping);

//     wxFontEncoding enc = wxLocale::GetSystemEncoding();
//     for (int i = 0; i < mappingCount; i++)
//     {
//         if (mappings[i].encoding == enc)
//             return mappings[i].htmlCS;
//     }
// #endif
    return QLatin1String("UTF-8");
}
//-----------------------------------------------------------------------------
QString escapeHtmlChars(const QString& input, bool processNewlines)
{
    if (input.isEmpty())
        return input;
    QString result;
    QString::const_iterator start = input.begin();
    while (start != input.end())
    {
        QString::const_iterator stop = start;
        while (stop != input.end())
        {
            const QChar c = *stop;
            if (c == '&' || c == '<' || c == '>' || c == '"'
                || (processNewlines && (c == '\r' || c == '\n')))
            {
                if (stop > start)
                    result += helper::rangedString(start, stop);
                if (c == '&')
                    result += QLatin1String("&amp;");
                else if (c == '<')
                    result += QLatin1String("&lt;");
                else if (c == '>')
                    result += QLatin1String("&gt;");
                else if (c == '"')
                    result += QLatin1String("&quot;");
                else if (c == '\n')
                    result += QLatin1String("<BR>");
                else if (c == '\r')
                    /* swallow silently */;
                else
                    Q_ASSERT_X(false, "StringUtils.cpp", "escape not handled");
                // start processing *after* the replaced character
                ++stop;
                start = stop;
                break;
            }
            ++stop;
        }
        if (stop > start)
            result += helper::rangedString(start, stop);
        start = stop;
    }
    return result;
}
//-----------------------------------------------------------------------------
QString escapeXmlChars(const QString& input)
{
    if (input.isEmpty())
        return input;
    QString result;
    QString::const_iterator start = input.begin();
    while (start != input.end())
    {
        QString::const_iterator stop = start;
        while (stop != input.end())
        {
            const QChar c = *stop;
            if (c == '&' || c == '<' || c == '>' || c == '"')
            {
                if (stop > start)
                    result += helper::rangedString(start, stop);
                if (c == '&')
                    result += QLatin1String("&amp;");
                else if (c == '<')
                    result += QLatin1String("&lt;");
                else if (c == '>')
                    result += QLatin1String("&gt;");
                else if (c == '"')
                    result += QLatin1String("&quot;");
                else
                    Q_ASSERT_X(false, "escapeXmlChars", "escape not handled");
                // start processing *after* the replaced character
                ++stop;
                start = stop;
                break;
            }
            ++stop;
        }
        if (stop > start)
            result += helper::rangedString(start, stop);
        start = stop;
    }
    return result;
}
//-----------------------------------------------------------------------------
QString wxArrayToString(const QStringList& arrayStr, const QString& delimiter)
{
    QString result;
    for (QStringList::const_iterator it = arrayStr.begin();
        it != arrayStr.end(); ++it)
    {
        if (result.isEmpty())
            result += *(it);
        else
            result += (delimiter + *(it));
    }
    return result;
}
//-----------------------------------------------------------------------------
QString loadEntireFile(const QFileInfo& filename)
{
    if (!filename.exists())
    {
        QString msg = QString::fromLatin1("The file \"%1\" does not exist.")
					  .arg(filename.absoluteFilePath());
        throw FRError(msg);
    }

    // read entire file into QString buffer
    std::ifstream filex(filename.absoluteFilePath().toLocal8Bit().constData());
    if (!filex)
    {
        QString msg = QString::fromLatin1("The file \"%1\" cannot be opened.")
					  .arg(filename.absoluteFilePath());
        throw FRError(msg);
    }

    std::stringstream ss;
    ss << filex.rdbuf();
    QString s = QString::fromStdString(ss.str());
    filex.close();
    return s;
}
//-----------------------------------------------------------------------------
QString wrapText(const QString& text, size_t maxWidth, size_t indent)
{
    QString indentStr;
    bool eol(false);
    QString wrappedText;
    QString line;

    QString::const_iterator lastSpace = text.end();
    QString::const_iterator lineStart = text.begin();
    for (QString::const_iterator it = lineStart; ; ++it)
    {
        if (eol)
        {
            wrappedText += QChar('\n');
            if (indentStr.isEmpty())
                indentStr.append(QByteArray((int)indent, ' '));

            lastSpace = text.end();
            line.clear();
            lineStart = it;
        }
        eol = false;

        if (it == text.end() || *it == QChar('\n'))
        {
            wrappedText += indentStr;
			wrappedText += line;
            eol = true;
            if (it == text.end())
                break;
        }
        else // not EOL
        {
            if (*it == QChar(' '))
                lastSpace = it;

            line += *it;

            if (maxWidth >= 0 && lastSpace != text.end())
            {
                int width = line.length();
                if (width > maxWidth - indentStr.length())
                {
                    // remove the last word from this line
                    line.remove(lastSpace - lineStart, it + 1 - lineStart);
                    wrappedText += (indentStr + line);
                    eol = true;

                    // go back to the last word of this line which we didn't
                    // output yet
                    it = lastSpace;
                }
            }
            // else: no wrapping at all or impossible to wrap
        }
    }
    return wrappedText;
}
//-----------------------------------------------------------------------------

} // namespace fr
