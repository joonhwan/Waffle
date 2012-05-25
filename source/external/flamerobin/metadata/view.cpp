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


  $Id: view.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
#include "metadata/column.h"
#include "metadata/MetadataItemVisitor.h"
#include "metadata/view.h"
#include "sql/StatementBuilder.h"

namespace fr {

//-----------------------------------------------------------------------------
View::View(DatabasePtr database, const QString& name)
    : Relation(ntView, database, name)
{
}
//-----------------------------------------------------------------------------
QString View::getSource()
{
    ensurePropertiesLoaded();
    return sourceM;
}
//-----------------------------------------------------------------------------
void View::setSource(const QString& value)
{
    sourceM = value;
}
//-----------------------------------------------------------------------------
QString View::getCreateSql()
{
    ensureChildrenLoaded();

    StatementBuilder sb;
    sb << kwCREATE << ' ' << kwVIEW << ' ' << getQuotedName() << QString::fromLatin1(" (")
        << StatementBuilder::IncIndent;

    // make sure that line breaking occurs after comma, not before
    ColumnPtrs::const_iterator it = columnsM.begin();
    QString colName = (*it)->getQuotedName();
    for (++it; it != columnsM.end(); ++it)
    {
        sb << colName + QString::fromLatin1(", ");
        colName = (*it)->getQuotedName();
    }
    sb << colName;

    sb << ')' << StatementBuilder::DecIndent << StatementBuilder::NewLine
        << kwAS << ' ' << StatementBuilder::DisableLineWrapping
        << getSource() << ';' << StatementBuilder::NewLine;
    return sb;
}
//-----------------------------------------------------------------------------
const QString View::getTypeName() const
{
    return QString::fromLatin1("VIEW");
}
//-----------------------------------------------------------------------------
void View::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitView(*this);
}
//-----------------------------------------------------------------------------
// Views collection
Views::Views(DatabasePtr database)
    : MetadataCollection<View>(ntViews, database, _("Views"))
{
}
//-----------------------------------------------------------------------------
void Views::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitViews(*this);
}
//-----------------------------------------------------------------------------
void Views::load(ProgressIndicator* progressIndicator)
{
    QString stmt = QString::fromLatin1("select rdb$relation_name from rdb$relations")
				   + QString::fromLatin1(" where (rdb$system_flag = 0 or rdb$system_flag is null)")
				   + QString::fromLatin1(" and rdb$view_source is not null order by 1");
    setItems(getDatabase()->loadIdentifiers(stmt, progressIndicator));
}
//-----------------------------------------------------------------------------
void Views::loadChildren()
{
    load(0);
}
//-----------------------------------------------------------------------------
const QString Views::getTypeName() const
{
    return QString::fromLatin1("VIEW_COLLECTION");
}
//-----------------------------------------------------------------------------

} // namespace fr
