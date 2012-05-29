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


  $Id: constraints.h 2200 2012-01-20 08:31:01Z mghie $

*/

#ifndef FR_CONSTRAINTS_H
#define FR_CONSTRAINTS_H
//-----------------------------------------------------------------------------
#include <vector>

#include "metadata/metadataitem.h"

namespace fr {

class Relation;
class Table;
//-----------------------------------------------------------------------------
class Constraint: public MetadataItem
{
public:
    virtual Table* getTable() const;
    virtual bool isSystem() const;
    virtual const QString getTypeName() const;
};
//-----------------------------------------------------------------------------
class ColumnConstraint: public Constraint
{
    friend class Relation;
    friend class Table;
public:
    typedef std::vector<QString>::const_iterator const_iterator;
    std::vector<QString>::const_iterator begin() const { return columnsM.begin(); };
    std::vector<QString>::const_iterator end() const { return columnsM.end(); };
	int size() const { return (int)columnsM.size(); };
    QString getColumnList(const QString& separator = QLatin1String(", "),
        const QString& suffix = QLatin1String("")) const;
    bool hasColumn(const QString& column) const;
    const QString& getIndexName() const { return indexNameM; };
    std::vector<QString>& getColumns() { return columnsM; };
protected:
    std::vector<QString> columnsM;
private:
    QString indexNameM;
};
//-----------------------------------------------------------------------------
//! uniques
class UniqueConstraint: public ColumnConstraint
{
public:
    virtual void acceptVisitor(MetadataItemVisitor* visitor);
};
//-----------------------------------------------------------------------------
//! primary keys
class PrimaryKeyConstraint: public UniqueConstraint
{
public:
    virtual void acceptVisitor(MetadataItemVisitor* visitor);
};
//-----------------------------------------------------------------------------
//! checks
class CheckConstraint: public ColumnConstraint
{
    friend class Relation;
    friend class Table;
public:
    const QString& getSource() const { return sourceM; }
private:
    QString sourceM;
};
//-----------------------------------------------------------------------------
//! foreign keys
class ForeignKey: public ColumnConstraint
{
    friend class Relation;
    friend class Table;
public:
    virtual void acceptVisitor(MetadataItemVisitor* visitor);
    QString getReferencedColumnList() const;
    QString getJoin(bool quoted) const;
    const QString& getReferencedTable() const { return referencedTableM; };
    const QString& getUpdateAction() const { return updateActionM; };
    const QString& getDeleteAction() const { return deleteActionM; };
    const std::vector<QString>& getReferencedColumns() const { return referencedColumnsM; };
private:
    QString referencedTableM;
    std::vector<QString> referencedColumnsM;
    QString updateActionM;
    QString deleteActionM;
};
//-----------------------------------------------------------------------------

}

#endif
