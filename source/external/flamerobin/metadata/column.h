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


  $Id: column.h 2205 2012-01-21 11:42:57Z mghie $

*/

//-----------------------------------------------------------------------------
#ifndef FR_COLUMN_H
#define FR_COLUMN_H

#include "metadata/metadataitem.h"

namespace fr {

//-----------------------------------------------------------------------------
class ColumnBase: public MetadataItem
{
private:
    bool hasDefaultM;
    QString defaultM;
    QString sourceM;
protected:
    virtual QString getComputedSource() const;
    void initialize(const QString& source, const QString& defaultValue,
        bool hasDefault);
public:
    ColumnBase(NodeType type, MetadataItem* parent, const QString& name);

    QString getDatatype(bool useConfig = true);
    DomainPtr getDomain() const;
    QString getDefault() const;
    bool hasDefault() const;
    QString getSource() const;
    virtual bool isNullable() const;
};
//-----------------------------------------------------------------------------
class Column: public ColumnBase
{
private:
    bool notnullM, computedM;
    QString sourceM, computedSourceM, collationM;
public:
    Column(Relation* relation, const QString& name);

    void initialize(bool notnull, const QString& source,
        const QString& computedSource, const QString& collation,
        const QString& defaultValue, bool hasDefault);
    virtual const QString getTypeName() const;
    virtual QString getDropSqlStatement() const;

    // isNullable() checks the underlying domain (if any) as well,
    // while hasNotNullConstraint() doesn't (necessary for DDL creation)
    virtual bool isNullable() const;
    bool hasNotNullConstraint() const;
    bool isForeignKey() const;
    bool isPrimaryKey() const;
    bool isString() const;
    virtual QString getComputedSource() const;
    QString getCollation() const;
    Table* getTable() const;
    virtual void acceptVisitor(MetadataItemVisitor* visitor);
};
//-----------------------------------------------------------------------------

} // namespace fr

#endif
