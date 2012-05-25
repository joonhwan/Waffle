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


  $Id: MultiStatement.cpp 2206 2012-01-21 15:12:51Z mghie $

*/
#include <algorithm>

#include "sql/MultiStatement.h"
#include "sql/SqlTokenizer.h"
#include "flamerobin-port-helper.h"

namespace fr {

//-----------------------------------------------------------------------------
SingleStatement::SingleStatement()
    : typeM(stInvalid)
{
}
//-----------------------------------------------------------------------------
SingleStatement::SingleStatement(const QString& sql)
    : sqlM(sql), typeM(stOther)
{
    SqlTokenizer tk(sql);
    // set stt1 and stt2 to first and second relevant token
    SqlTokenType stt1 = tk.getCurrentToken();
    while (stt1 == tkCOMMENT || stt1 == tkWHITESPACE)
    {
        tk.nextToken();
        stt1 = tk.getCurrentToken();
    }
    tk.jumpToken(false);
    SqlTokenType stt2 = tk.getCurrentToken();

    // make sure that only exact forms of the statements are recognized
    // pass everything else to the engine, to allow for stuff like
    // "ROLLBACK WORK TO SAVEPOINT SP23"
    // and to catch missing terminators (which would silently swallow the
    // next statement, see SF.net tracker #3375476)
    switch (stt1)
    {
        case kwCOMMIT:
            // allow for "COMMIT" or "COMMIT WORK" only
            if (stt2 == kwWORK)
            {
                tk.jumpToken(false);
                stt2 = tk.getCurrentToken();
            }
            if (stt2 == tkEOF)
                typeM = stCommit;
            break;

        case kwROLLBACK:
            // allow for "ROLLBACK" or "ROLLBACK WORK" only
            if (stt2 == kwWORK)
            {
                tk.jumpToken(false);
                stt2 = tk.getCurrentToken();
            }
            if (stt2 == tkEOF)
                typeM = stRollback;
            break;

        case kwSET:
            // allow for "SET TERM", "SET AUTO" or "SET AUTODDL" only
            // with an (optional) parameter that the calling code can access
            if (stt2 == kwTERMINATOR || stt2 == kwAUTO || stt2 == kwAUTODDL)
            {
                tk.jumpToken(false);
                thirdStringM = tk.getCurrentTokenString();
                tk.jumpToken(false);
                if (tk.getCurrentToken() == tkEOF)
                {
                    if (stt2 == kwTERMINATOR)
                        typeM = stSetTerm;
                    else
                        typeM = stSetAutoDDL;
                }
            }
            break;

        default:
            break;
    }
}
//-----------------------------------------------------------------------------
bool SingleStatement::isCommitStatement() const
{
    return typeM == stCommit;
}
//-----------------------------------------------------------------------------
bool SingleStatement::isRollbackStatement() const
{
    return typeM == stRollback;
}
//-----------------------------------------------------------------------------
bool SingleStatement::isSetTermStatement(QString& newTerm) const
{
    if (typeM != stSetTerm)
        return false;

    newTerm = thirdStringM;
    return true;
}
//-----------------------------------------------------------------------------
bool SingleStatement::isSetAutoDDLStatement(QString& newSetting) const
{
    if (typeM != stSetAutoDDL)
        return false;

    newSetting = thirdStringM;
    return true;
}
//-----------------------------------------------------------------------------
bool SingleStatement::isValid() const
{
    return typeM != stInvalid && !helper::stripString(sqlM).isEmpty();
}
//-----------------------------------------------------------------------------
QString SingleStatement::getSql() const
{
    return sqlM;
}
//-----------------------------------------------------------------------------
//! MultiStatement class
MultiStatement::MultiStatement(const QString& sql, const QString& terminator)
    : sqlM(sql), terminatorM(terminator), atEndM(false)
{
    oldPosM = searchPosM = sqlM.begin();
}
//-----------------------------------------------------------------------------
SingleStatement MultiStatement::getNextStatement()
{
    if (atEndM)    // end marked in previous iteration
        return SingleStatement();

    QString interesting(QString::fromLatin1("'/-"));
    if (!terminatorM.isEmpty())
        interesting += *terminatorM.begin();
    QString::const_iterator searchEnd = sqlM.end();

    oldPosM = searchPosM;
    while (true)
    {
        lastPosM = searchEnd;
        QString::const_iterator p = std::find_first_of(searchPosM, searchEnd,
            interesting.begin(), interesting.end());
        if (p == searchEnd)
        {
            atEndM = true;
        }
        else
        {
            // scan over embedded quotes
            if (*p == '\'')
            {
                searchPosM = std::find(p + 1, searchEnd, '\'');
                if (searchPosM != searchEnd)
                {
                    ++searchPosM;
                    continue;
                }
            }
            // scan over single-line comment
            else if (*p == '-' && p + 1 != searchEnd && *(p + 1) == '-')
            {
                searchPosM = std::find(p + 2, searchEnd, '\n');
                if (searchPosM != searchEnd)
                {
                    ++searchPosM;
                    continue;
                }
            }
            // scan over multi-line comment
            else if (*p == '/' && p + 1 != searchEnd && *(p + 1) == '*')
            {
                QString commentEnd(QString::fromLatin1("*/"));
                searchPosM = std::search(p + 2, searchEnd, commentEnd.begin(),
                    commentEnd.end());
                if (searchPosM != searchEnd)
                {
                    searchPosM += commentEnd.size();
                    continue;
                }
            }
            // ignore partial matches with terminator
            else
            {
                QString::const_iterator p1 = terminatorM.begin();
                QString::const_iterator p2 = p;
                while (p1 != terminatorM.end() && p2 != interesting.end())
                {
                    if (*p1 != *p2)
                        break;
                    p1++;
                    p2++;
                }
                if (p1 != terminatorM.end())
                {
                    searchPosM = p + 1;
                    continue;
                }
                lastPosM = p;
                searchPosM = p + terminatorM.size();
            }
            if (searchPosM == searchEnd)
                atEndM = true;
        }

        QString sql = helper::rangedString(oldPosM, lastPosM); // wxString(oldPosM, lastPosM);
        SingleStatement ss(sql);

        QString newTerm;                   // change terminator
        if (ss.isSetTermStatement(newTerm))
        {
            terminatorM = newTerm;
            if (newTerm.isEmpty())    // the caller should decide what to do as
                return ss;          // we don't want to popup msgbox from here
            if (atEndM)             // terminator is the last statement
                return SingleStatement();

            interesting = QString::fromLatin1("'/-");
            interesting += *terminatorM.begin();
            oldPosM = searchPosM;
            continue;
        }
        return ss;
    }
}
//-----------------------------------------------------------------------------
SingleStatement MultiStatement::getStatementAt(int position, int& offset)
{
    oldPosM = searchPosM = sqlM.begin();
    while (true)
    {
        SingleStatement s = getNextStatement();
        if (!s.isValid() || lastPosM - sqlM.begin() >= position)
        {
            offset = oldPosM - sqlM.begin();
            return s;
        }
    }
}
//-----------------------------------------------------------------------------
void MultiStatement::setTerminator(const QString& newTerm)
{
    terminatorM = newTerm;
}
//-----------------------------------------------------------------------------
QString MultiStatement::getTerminator() const
{
    return terminatorM;
}
//-----------------------------------------------------------------------------
int MultiStatement::getStart() const
{
    return oldPosM - sqlM.begin();
}
//-----------------------------------------------------------------------------
int MultiStatement::getEnd() const
{
    return lastPosM - sqlM.begin();
}
//-----------------------------------------------------------------------------

} // namespace fr
