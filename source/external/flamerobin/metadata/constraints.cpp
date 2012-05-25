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


  $Id: constraints.cpp 2200 2012-01-20 08:31:01Z mghie $

*/

#include <vector>

#include "metadata/constraints.h"
#include "metadata/database.h"
#include "metadata/MetadataItemVisitor.h"
#include "metadata/table.h"

namespace fr {

//-----------------------------------------------------------------------------
bool Constraint::isSystem() const
{
    Table* t = getTable();
    if (t)
        return t->isSystem();
    else
        return false;
}
//-----------------------------------------------------------------------------
const QString Constraint::getTypeName() const
{
    return QString::fromLatin1("CONSTRAINT");
}
//-----------------------------------------------------------------------------
QString ColumnConstraint::getColumnList(const QString& separator,
    const QString& suffix) const
{
    QString result;
    for (std::vector<QString>::const_iterator it = columnsM.begin(); it != columnsM.end(); ++it)
    {
        if (it != columnsM.begin())
            result += separator;
        result += (*it) + suffix;
    }
    return result;
};
//-----------------------------------------------------------------------------
bool ColumnConstraint::hasColumn(const QString& column) const
{
    return columnsM.end() != std::find(columnsM.begin(), columnsM.end(),
        column);
}
//-----------------------------------------------------------------------------
QString ForeignKey::getReferencedColumnList() const
{
    QString result;
    for (std::vector<QString>::const_iterator it = referencedColumnsM.begin();
         it != referencedColumnsM.end(); ++it)
    {
        if (it != referencedColumnsM.begin())
            result += QString::fromLatin1(", ");
        result += (*it);
    }
    return result;
};
//-----------------------------------------------------------------------------
QString ForeignKey::getJoin(bool quoted) const
{
    Identifier reftab(referencedTableM);
    QString rtab = (quoted ? reftab.getQuoted() : reftab.get());
    QString table = (quoted ? getTable()->getQuotedName() : getTable()->getName_());
    QString result;
    std::vector<QString>::const_iterator im = columnsM.begin();
    for (std::vector<QString>::const_iterator it = referencedColumnsM.begin();
         it != referencedColumnsM.end(); ++it, ++im)
    {
        if (!result.isEmpty())
            result += QString::fromLatin1(" AND ");
        Identifier col1(*im);
        Identifier col2(*it);
        QString c1 = (quoted ? col1.getQuoted() : col1.get());
        QString c2 = (quoted ? col2.getQuoted() : col2.get());
        result += table + QString::fromLatin1(".") + c1 + QString::fromLatin1(" = ") + rtab + QString::fromLatin1(".") + c2;
    }
    return result;
}
//-----------------------------------------------------------------------------
void ForeignKey::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitForeignKey(*this);
}
//-----------------------------------------------------------------------------
void UniqueConstraint::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitUniqueConstraint(*this);
}
//-----------------------------------------------------------------------------
void PrimaryKeyConstraint::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitPrimaryKeyConstraint(*this);
}
//-----------------------------------------------------------------------------
Table* Constraint::getTable() const
{
    MetadataItem* m = getParent();
    while (m)
    {
        if (Table* t = dynamic_cast<Table*>(m))
            return t;
        m = m->getParent();
    }
    return 0;
}
//-----------------------------------------------------------------------------

} // namespace fr
