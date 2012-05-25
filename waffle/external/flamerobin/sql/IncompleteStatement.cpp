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


  $Id: IncompleteStatement.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
#include "config/Config.h"
#include "metadata/column.h"
#include "metadata/database.h"
#include "metadata/parameter.h"
#include "metadata/procedure.h"
#include "metadata/relation.h"
#include "sql/Identifier.h"
#include "sql/IncompleteStatement.h"
#include "sql/MultiStatement.h"
#include "sql/SqlTokenizer.h"
#include "flamerobin-port-helper.h"

namespace fr {

//-----------------------------------------------------------------------------
IncompleteStatement::IncompleteStatement(Database *db, const QString& sql)
    :databaseM(db), sqlM(sql)
{
}
//-----------------------------------------------------------------------------
// position is offset at which user typed the dot character
QString IncompleteStatement::getObjectColumns(const QString& table,
    int position)
{
    MultiStatement ms(sqlM);
    int offset;
    SingleStatement st = ms.getStatementAt(position, offset);
    if (!st.isValid())
        return QString();

    // feed st to tokenizer, find UNION section we're in
    QString sql = st.getSql();
    SqlTokenizer stok(sql);
    int pstart = 0, pend = sql.length();
    do
    {
        if (stok.getCurrentToken() == kwUNION)
        {
            int upos = stok.getCurrentTokenPosition();
            if (position - offset > upos)    // before cursor position
                pstart = upos;
            if (position - offset < upos)    // after cursor position
            {
                pend = upos;
                break;
            }
        }
    }
    while (stok.nextToken());
    position -= (offset + pstart);
    sql = sql.mid(pstart, pend-pstart);
    return getColumnsForObject(sql, table, position);
}
//-----------------------------------------------------------------------------
typedef std::pair<QString, QString> IdAlias;
typedef std::multimap<QString, QString> IdAliasMap;

template <class T>
T* IncompleteStatement::findObject(IdAliasMap& aliases, const QString& alias,
    NodeType type)
{
    for (IdAliasMap::iterator i = aliases.lower_bound(alias);
        i != aliases.upper_bound(alias); ++i)
    {
        T* t = dynamic_cast<T *>(databaseM->findByNameAndType(type,
            (*i).second));
        if (t)
            return t;
    }
    // find by NAME in case user doesn't have from/into/etc. clause but
    // is using FULL_RELATION_NAME.column syntax
    Identifier id;
    id.setFromSql(alias);
    return dynamic_cast<T *>(databaseM->findByNameAndType(type, id.get()));
}
//-----------------------------------------------------------------------------
Relation *IncompleteStatement::getCreateTriggerRelation(const QString& sql)
{
    Relation* r = 0;
    SqlTokenType search[] = { kwCREATE, kwTRIGGER, tkIDENTIFIER, kwFOR };
    SqlTokenizer tokenizer(sql);
    SqlTokenType stt;
    QString relName;
    while (true)
    {
        int i=0;
        for (; i < 4; ++i)
        {
            stt = tokenizer.getCurrentToken();
            if (!tokenizer.jumpToken(false))
                return 0;
            if (stt != search[i])
                break;
        }
        if (i == 4)     // we have a match
        {
            if (tkIDENTIFIER == tokenizer.getCurrentToken())
            {
                relName = tokenizer.getCurrentTokenString();
                break;
            }
        }
    }
    if (!relName.isEmpty())
    {
        Identifier id;
        id.setFromSql(relName);
        r = dynamic_cast<Relation *>(databaseM->findByNameAndType(ntTable,
            id.get()));
        if (!r)
        {
            r = dynamic_cast<Relation *>(databaseM->findByNameAndType(ntView,
                id.get()));
        }
    }
    return r;
}
//-----------------------------------------------------------------------------
Relation *IncompleteStatement::getAlterTriggerRelation(const QString& sql)
{
    Relation* r = 0;
    SqlTokenType search[] = { kwALTER, kwTRIGGER };
    SqlTokenizer tokenizer(sql);
    SqlTokenType stt;
    QString trigName;
    while (true)
    {
        int i=0;
        for (; i < 2; ++i)
        {
            stt = tokenizer.getCurrentToken();
            if (!tokenizer.jumpToken(false))
                return 0;
            if (stt != search[i])
                break;
        }
        if (i == 2)     // we have a match
        {
            if (tkIDENTIFIER == tokenizer.getCurrentToken())
            {
                trigName = tokenizer.getCurrentTokenString();
                break;
            }
        }
    }
    if (!trigName.isEmpty())
    {
        Identifier id;
        id.setFromSql(trigName);
        Trigger *t = dynamic_cast<Trigger *>(databaseM->findByNameAndType(
            ntTrigger, id.get()));
        if (!t)
            return 0;
        QString relName = t->getTriggerRelation();
        if (relName.isEmpty())  // database trigger
            return 0;
        r = dynamic_cast<Relation *>(databaseM->findByNameAndType(ntTable,
            relName));
        if (!r)
        {
            r = dynamic_cast<Relation *>(databaseM->findByNameAndType(
                ntView, relName));
        }
    }
    return r;
}
//-----------------------------------------------------------------------------
QString IncompleteStatement::extractBlockAtPosition(const QString& sql,
    int pos) const
{
    // search for FOR..DO and BEGIN..END blocks
    // find block where cursorPos is and extract the sql from it
    SqlTokenizer tk(sql);
    int start = 0, end = sql.length();
    while (true)
    {
        SqlTokenType stt = tk.getCurrentToken();
        if (stt == tkEOF)
            break;
        if (stt == kwSUBSTRING) // skip FOR in: substring(x from y FOR z)
            tk.jumpToken(true);
        int cpos = tk.getCurrentTokenPosition();
        if (stt == kwFOR || stt == kwBEGIN || stt == kwEND || stt == kwDO)
        {
            if (cpos > pos)
            {
                end = cpos;
                break;
            }
            else
                start = cpos;
        }
        tk.jumpToken(false);
    }
    QString s;
    if (start != 0 || end != (int)sql.length())
        s = sql.mid(start, end - start);
    else
        s = sql;
    // take the SQL and split by statements
    MultiStatement mst(s);
    return mst.getStatementAt(pos - start).getSql();
}
//-----------------------------------------------------------------------------
QString IncompleteStatement::getColumnsForObject(const QString& sql,
    const QString& objectSqlAlias, int cursorPos)
{
    Identifier idAlias;
    idAlias.setFromSql(objectSqlAlias);
    QString objectAlias(idAlias.get());
    Relation *r = 0;
    if (objectAlias.toUpper() == QString::fromLatin1("OLD")
		|| objectAlias.toUpper() == QString::fromLatin1("NEW"))
    {
        r = getCreateTriggerRelation(sql);
        if (!r)
            r = getAlterTriggerRelation(sql);
        if (!r)
            return QString();
    }

    IdAliasMap aliases;
    if (!r)
    {
        SqlTokenizer tokenizer(extractBlockAtPosition(sql, cursorPos));
        SqlTokenType search[] = { kwFROM, kwJOIN, kwUPDATE, kwINSERT };
        SqlTokenType stt;
        while (tkEOF != (stt = tokenizer.getCurrentToken()))
        {
            //wxMessageBox(QString::Format(QString::fromLatin1("Tok: %d, String: %s"), stt,
            //  tokenizer.getCurrentTokenString().c_str()), QString::fromLatin1("TOKEN"));

            // skip FROM in: substring (x FROM y for z)
            if (stt == kwSUBSTRING)
            {
                tokenizer.jumpToken(true);  // skip parenthesis
                continue;
            }

            bool keepNextToken = false;
            // find all [DELETE] FROM, JOIN, UPDATE, INSERT INTO tokens
            for (int i = 0; i < sizeof(search) / sizeof(SqlTokenType); ++i)
            {
                if (search[i] != stt)
                    continue;

                if (stt == kwINSERT)    // find INTO
                {
                    tokenizer.jumpToken(false);
                    if (kwINTO != tokenizer.getCurrentToken())
                        break;
                }
                tokenizer.jumpToken(false);  // table/view/procedure name

                while (tkIDENTIFIER == tokenizer.getCurrentToken())
                {
                    Identifier id;
                    id.setFromSql(tokenizer.getCurrentTokenString());
                    QString alias;
                    tokenizer.jumpToken(true);
                    if (tkIDENTIFIER == tokenizer.getCurrentToken())
                    {   // aliases can also be quoted, and case insensitive
                        Identifier ida;
                        ida.setFromSql(tokenizer.getCurrentTokenString());
                        alias = ida.get();
                    }
                    else
                        alias = id.get();
                    //wxMessageBox(id.get()+QString::fromLatin1(" ")+alias);
                    aliases.insert(IdAlias(alias, id.get()));
                    tokenizer.jumpToken(false);
                    // allow for SELECT ... FROM TBL_FOO f, TBL_BAR b
                    if (tkCOMMA != tokenizer.getCurrentToken())
                    {
                        keepNextToken = true;
                        break;
                    }
                    tokenizer.jumpToken(false);
                }
                break;
            }
            if (!keepNextToken)
                tokenizer.jumpToken(false);
        }

        // find TABLE or VIEW in list of ALIASES
        r = findObject<Relation>(aliases, objectAlias, ntTable);
        if (!r)
            r = findObject<Relation>(aliases, objectAlias, ntView);
        if (!r)
            r = findObject<Relation>(aliases, objectAlias, ntSysTable);
    }
    std::list<QString> cols;
    if (r)
    {
        if (r->begin() == r->end())   // no columns, load if needed
        {
            if (config().get(QString::fromLatin1("autoCompleteLoadColumns"), true))
                r->ensureChildrenLoaded();
            else
                return QString();
        }
        for (ColumnPtrs::const_iterator c = r->begin();
            c != r->end(); ++c)
        {
            cols.push_back((*c)->getQuotedName());
        }
    }
    else    // find STORED PROCEDURE in list of ALIASES
    {
        Procedure* p = findObject<Procedure>(aliases, objectAlias,
            ntProcedure);
        if (!p) // give up, we couldn't match anything
            return QString();
        if (p->begin() == p->end())
        {
            if (config().get(QString::fromLatin1("autoCompleteLoadColumns"), true))
                p->ensureChildrenLoaded();
            else
                return QString();
        }
        for (ParameterPtrs::const_iterator c = p->begin();
            c != p->end(); ++c)
        {
            if ((*c)->isOutputParameter())
                cols.push_back((*c)->getQuotedName());
        }
    }

    cols.sort();
    QString columns;
    for (std::list<QString>::iterator i = cols.begin(); i != cols.end(); ++i)
        columns += (*i) + QString::fromLatin1(" ");
    return helper::stripString(columns);     // remove trailing space
}
//-----------------------------------------------------------------------------

} // namespace fr
