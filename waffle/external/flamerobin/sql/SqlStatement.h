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


  $Id: SqlStatement.h 2200 2012-01-20 08:31:01Z mghie $

*/
//-----------------------------------------------------------------------------
#ifndef FR_SQL_STATEMENT_H
#define FR_SQL_STATEMENT_H

#include <vector>
#include <map>
#include "metadata/metadataitem.h"
#include "sql/SqlTokenizer.h"

namespace fr {

class Database;
class Relation;
//-----------------------------------------------------------------------------
typedef enum
{
    actNONE, actALTER, actCREATE, actCREATE_OR_ALTER, actDECLARE, actDROP,
    actRECREATE, actSET, actUPDATE, actGRANT, actCOMMENT
} SqlAction;
//-----------------------------------------------------------------------------
class TokenList
{
private:
    std::vector<SqlTokenType> tokensM;
public:
    size_t size() const;
    void add(const SqlTokenType& item);
    const SqlTokenType& operator[](const size_t& index) const;
};
//-----------------------------------------------------------------------------
class SqlStatement
{
public:
    SqlStatement(const QString& sql, Database* db,
        const QString& terminator = QLatin1String(";"));

    bool isDDL() const;
    SqlAction getAction() const;
    bool actionIs(const SqlAction& act, NodeType nt = ntUnknown) const;
    NodeType getObjectType() const;
    MetadataItem* getObject() const;
    Identifier getIdentifier() const;
    QString getName() const;
    QString getFieldName() const;
    QString getTerminator() const;
    QString getStatement() const;
    bool isAlterColumn() const;
    bool isDatatype() const;
    Relation* getCreateTriggerRelation() const;

protected:
    TokenList tokensM;
    int identifierTokenIndexM;
    std::map<int, QString> tokenStringsM;

    MetadataItem* objectM;
    Database* databaseM;
    NodeType objectTypeM;
    SqlAction actionM;
    Identifier nameM;
    Identifier fieldNameM;  // table columns
    bool isAlterColumnM;
    bool isDatatypeM;
    QString terminatorM;
    QString statementM;
};
//-----------------------------------------------------------------------------

} // namespace fr
#endif
