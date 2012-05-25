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


  $Id: StatementBuilder.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
#include "config/Config.h"
#include "sql/StatementBuilder.h"

namespace fr {

//-----------------------------------------------------------------------------
StatementBuilder::StatementBuilder()
    : indentCharsM(0), indentLevelM(0), lineWrappingM(false), maxLineLengthM(0)
{
    keywordsUpperCaseM = config().get(QString::fromLatin1("SQLKeywordsUpperCase"), true);
    // take settings for line wrapping from vertical editor line settings:
    // enable wrapping only if the marker is shown, and if so wrap to the
    // same column, indent by editor tab size
    
    if (config().get(QString::fromLatin1("sqlEditorShowEdge"), false))
    {
        lineWrappingM = true;
        maxLineLengthM = config().get(QString::fromLatin1("sqlEditorEdgeColumn"), 80);
        indentCharsM = config().get(QString::fromLatin1("sqlEditorTabSize"), 4);
    }
}
//-----------------------------------------------------------------------------
StatementBuilder& StatementBuilder::operator<< (const ControlToken ct)
{
    switch (ct)
    {
        case NewLine:
            addNewLine();
            break;
        case IncIndent:
            ++indentLevelM;
            break;
        case DecIndent:
            if (indentLevelM)
                --indentLevelM;
            break;
        case DisableLineWrapping:
            lineWrappingM = false;
            break;
        case EnableLineWrapping:
            if (maxLineLengthM)
                lineWrappingM = true;
            break;
        default:
            Q_ASSERT(false);
            break;
    }
    return (*this);
}
//-----------------------------------------------------------------------------
StatementBuilder& StatementBuilder::operator<< (const char c)
{
    if (lineWrappingM && currentLineM.length() + 1 > maxLineLengthM)
        addNewLine();
    currentLineM += c;
    return (*this);
}
//-----------------------------------------------------------------------------
StatementBuilder& StatementBuilder::operator<< (const QString& s)
{
    if (lineWrappingM && currentLineM.length() + s.length() > maxLineLengthM)
        addNewLine();
    currentLineM += s;
    return (*this);
}
//-----------------------------------------------------------------------------
StatementBuilder& StatementBuilder::operator<< (const SqlTokenType stt)
{
    QString kw(SqlTokenizer::getKeyword(stt, keywordsUpperCaseM));
    if (lineWrappingM && currentLineM.length() + kw.length() > maxLineLengthM)
        addNewLine();
    currentLineM += kw;
    return (*this);
}
//-----------------------------------------------------------------------------
StatementBuilder::operator QString() const
{
    return completedLinesM + currentLineM;
}
//-----------------------------------------------------------------------------
void StatementBuilder::addNewLine()
{
    completedLinesM += currentLineM;
    completedLinesM += QLatin1Char('\n'); //wxTextBuffer::GetEOL();
    currentLineM = QString(indentLevelM * indentCharsM, QChar(' '));
}
//-----------------------------------------------------------------------------
void StatementBuilder::reset()
{
    completedLinesM.clear();
    currentLineM.clear();
    indentLevelM = 0;
}
//-----------------------------------------------------------------------------

} // namespace fr
