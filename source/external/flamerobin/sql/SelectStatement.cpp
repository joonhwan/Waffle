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


  $Id: SelectStatement.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
#include "sql/SelectStatement.h"
#include "sql/SqlTokenizer.h"

namespace fr {

//-----------------------------------------------------------------------------
SelectStatement::SelectStatement(const QString& sql)
{
    setStatement(sql);
}
//-----------------------------------------------------------------------------
bool SelectStatement::isValidSelectStatement()
{
    return (posSelectM != -1 && posFromM != -1);
}
//-----------------------------------------------------------------------------
void SelectStatement::setStatement(const QString& sql)
{
    sqlM = sql;
    posSelectM = posFromM = posFromEndM = -1;
    tokenizerM.setStatement(sql);

    // find SELECT and FROM position
    int paren = 0;
    do
    {
        SqlTokenType stt = tokenizerM.getCurrentToken();
        if (stt == tkEOF)
            break;

        // check parenthesis to ignore nested select statements
        if (stt == tkPARENOPEN)
            paren++;
        if (stt == tkPARENCLOSE && paren > 0)
            paren--;

        if (paren == 0)
        {
            if (stt == kwSELECT)
                posSelectM = tokenizerM.getCurrentTokenPosition();
            if (posSelectM != -1 && stt == kwFROM)
                posFromM = tokenizerM.getCurrentTokenPosition();
            if (posFromM != -1 && (stt == kwWHERE || stt == kwGROUP
                || stt == kwORDER || stt == kwPLAN || stt == kwROWS))
            {
                posFromEndM = tokenizerM.getCurrentTokenPosition();
            }
        }
    }
    while (tokenizerM.nextToken());

    if (posSelectM != -1 && posFromM != -1 && posFromEndM == -1)
        posFromEndM = tokenizerM.getCurrentTokenPosition();
}
//-----------------------------------------------------------------------------
QString SelectStatement::getStatement()
{
    return sqlM;
}
//-----------------------------------------------------------------------------
// start from SELECT position and look for COMMA
void SelectStatement::getColumns(std::vector<QString>& columns)
{
    // reset to tokenizer to SELECT position (inefficient, but what to do)
    tokenizerM.setStatement(sqlM);
    while (posSelectM != tokenizerM.getCurrentTokenPosition())
        if (!tokenizerM.nextToken())
            return; // throw?

    QString columnName;
    while (tokenizerM.jumpToken(true /* skip parenthesis */))
    {
        SqlTokenType stt = tokenizerM.getCurrentToken();
        if (stt == kwFROM)
            break;  // we're done here, no more tables
        if (columnName.isEmpty() && stt == tkIDENTIFIER)
        {
            columnName = tokenizerM.getCurrentTokenString();
            columns.push_back(columnName);
            continue;
        }
        if (stt == tkCOMMA)
            columnName.clear();
    }
}
//-----------------------------------------------------------------------------
/*
SELECT ...
FROM t1,t2 alias,t3
JOIN t4 ON x=y and z=g and ()
LEFT JOIN t5 alias2 ON ...
*/
// start from FROM position and look for either COMMA or JOIN
// FIXME: support table aliases should be added
void SelectStatement::getTables(std::vector<QString>& tables)
{
    // reset to tokenizer to FROM position (inefficient, but what to do)
    tokenizerM.setStatement(sqlM);
    while (posFromM != tokenizerM.getCurrentTokenPosition())
        if (!tokenizerM.nextToken())
            return; // throw?

    // find SELECT and FROM position
    QString tableName;
    while (tokenizerM.jumpToken(true /* skip parenthesis */))
    {
        SqlTokenType stt = tokenizerM.getCurrentToken();
        if (stt == kwWHERE || stt == kwGROUP || stt == kwORDER
            || stt == kwPLAN || stt == kwROWS)
        {
            break;  // we're done here, no more tables
        }
        if (tableName.isEmpty() && stt == tkIDENTIFIER)
        {
            tableName = tokenizerM.getCurrentTokenString();
            tables.push_back(tableName);
            continue;
        }
        if (stt == tkCOMMA || stt == kwJOIN)
            tableName.clear();
    }
}
//-----------------------------------------------------------------------------
void SelectStatement::add(const QString& toAdd, int position)
{
    QString s(sqlM.left(position));

    // always add extra space in case we're adding to the end of the
    // statement
    // s += wxTextBuffer::GetEOL() + toAdd;
	s += QChar('\n') + toAdd;

    s += sqlM.mid(position);
    setStatement(s);
}
//-----------------------------------------------------------------------------
void SelectStatement::addTable(const QString& name, const QString& joinType,
    const QString& joinList)
{
    if (joinType == QString::fromLatin1("CARTESIAN"))
    {
        std::vector<QString> s;
        getTables(s);
        if (s.empty())
            add(name + QString::fromLatin1(" "), posFromM + 5); // 5 = strlen("FROM ");
        else
            add(name + QString::fromLatin1(", "), posFromM + 5);    // 5 = strlen("FROM ");
    }
    else
    {
        add(joinType + QString::fromLatin1(" ") + name + QString::fromLatin1(" ON ") + joinList + QString::fromLatin1(" "),
            posFromEndM);
    }
}
//-----------------------------------------------------------------------------
void SelectStatement::addColumn(const QString& columnList)
{
    std::vector<QString> s;
    getColumns(s);
    if (s.empty())
        add(columnList + QString::fromLatin1(" "), posSelectM + 7); // 7 = strlen("SELECT ");
    else
        add(columnList + QString::fromLatin1(", "), posSelectM + 7);
}
//-----------------------------------------------------------------------------
// covers only the most basic cases
void SelectStatement::orderBy(int column)
{
    // look for ORDER BY.
    tokenizerM.setStatement(sqlM);
    bool hasOrder = false;
    int pos = -1;
    while (tokenizerM.jumpToken(true /* skip parenthesis */))
    {
        SqlTokenType stt = tokenizerM.getCurrentToken();
        if (stt == kwORDER)
            hasOrder = true;
        if (hasOrder && stt == kwBY)
        {
            pos = tokenizerM.getCurrentTokenPosition() + 2;
            break;
        }
    }
    QString coltoadd = QString::number(column);
    // coltoadd.Printf(QString::fromLatin1("%d"), column);

    // if !found, add ORDER BY at the end
    if (pos == -1)
    {
        // sqlM += wxTextBuffer::GetEOL() + QString(QString::fromLatin1("ORDER BY "))
        //     + coltoadd;
		sqlM += QLatin1Char('\n') + QString(QString::fromLatin1("ORDER BY "))
            + coltoadd;
    }
    else
    {
        // Are we already using that column?
        tokenizerM.jumpToken(true);
        if (coltoadd != tokenizerM.getCurrentTokenString())
        {
            add(coltoadd + QString::fromLatin1(", "), pos); // No. Add it
            return;
        }

        // Yes. Remove ASC or DESC and add the opposite
        tokenizerM.jumpToken(true);
        SqlTokenType stt = tokenizerM.getCurrentToken();
        int p = tokenizerM.getCurrentTokenPosition();
        QString s = tokenizerM.getCurrentTokenString();
        if (stt == kwDESCENDING || stt == kwASCENDING) // remove
            sqlM.remove(p, s.length());
        if (stt != kwDESCENDING)    // add desc if there wasn't
            add(QString::fromLatin1("DESC "), p);
    }
}

//-----------------------------------------------------------------------------

} //namespace fr
