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


  $Id: StringUtils.h 2200 2012-01-20 08:31:01Z mghie $

*/

#ifndef FR_STRINGUTILS_H
#define FR_STRINGUTILS_H
//-----------------------------------------------------------------------------
#include <string>
#include <QFileInfo>
#include <QString>
#include <QTextCodec>
// class QTextCodec;

namespace fr {

//-----------------------------------------------------------------------------
std::string wx2std(const QString& input, QTextCodec* conv = 0);

QString std2wx(const std::string& input, QTextCodec* conv = 0);

QString std2wxIdentifier(const std::string& input, QTextCodec* conv);

//-----------------------------------------------------------------------------
// Converts chars that have special meaning in HTML or XML, so they get
// displayed.
QString escapeHtmlChars(const QString& input, bool processNewlines = true);
QString escapeXmlChars(const QString& input);
//-----------------------------------------------------------------------------
// Returns string suitable for HTML META charset tag (used only if no
// conversion to UTF-8 is available, i.e. in non-Unicode build.
QString getHtmlCharset();
//-----------------------------------------------------------------------------
// Standard way to confert a boolean to a string ("true"/"false").
inline QString getBooleanAsString(bool value) { return value ? QString::fromLatin1("true") : QString::fromLatin1("false"); }
// Standard way to confert a string ("true"/"false") to a boolean.
inline bool getStringAsBoolean(QString value) { return value == QString::fromLatin1("true") ? true : false; }
//-----------------------------------------------------------------------------
// Converts a QStringList to a delimited string of values.
QString wxArrayToString(const QStringList& arrayStr, const QString& delimiter);
//-----------------------------------------------------------------------------
//! loads the file into QString
QString loadEntireFile(const QFileInfo& filename);
//-----------------------------------------------------------------------------
//! wraps <text> into lines of maximum <maxWidth> characters, inserting a line
//  break after each line. All lines after the first are also indented by
//  <indent> spaces.
//  Code adapted from wxWidgets' wxTextWrapper function.
QString wrapText(const QString& text, size_t maxWidth, size_t indent);
//-----------------------------------------------------------------------------

} // namespace fr

#endif // FR_STRINGUTILS_H
