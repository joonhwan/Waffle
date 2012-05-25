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


  $Id: IncompleteStatement.h 2200 2012-01-20 08:31:01Z mghie $

*/
//-----------------------------------------------------------------------------
#ifndef FR_INCOMPLETE_STATEMENT_H
#define FR_INCOMPLETE_STATEMENT_H

namespace fr {

class Database;
class Relation;
//-----------------------------------------------------------------------------
//! Provides various information for incomplete (partial) sql statements
//! Used mostly for autocomplete stuff
class IncompleteStatement
{
private:
    Database* databaseM;
    QString sqlM;

    Relation* getCreateTriggerRelation(const QString& sql);
    Relation* getAlterTriggerRelation(const QString& sql);
    QString getColumnsForObject(const QString& sql,
        const QString& objectAlias, int cursorPos);
    QString extractBlockAtPosition(const QString& sql, int pos) const;

    template <class T>
    T* findObject(std::multimap<QString, QString>& aliases,
        const QString& alias, NodeType type);

public:
    IncompleteStatement(Database* db, const QString& sql);

    // position is offset at which user typed the dot character
    QString getObjectColumns(const QString& table, int position);
};
//-----------------------------------------------------------------------------

}; // namespace fr

#endif
