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


  $Id: column.cpp 2205 2012-01-21 11:42:57Z mghie $

*/
#include "config/Config.h"
#include "metadata/collection.h"
#include "metadata/column.h"
#include "metadata/constraints.h"
#include "metadata/domain.h"
#include "metadata/MetadataItemVisitor.h"
#include "metadata/table.h"
#include "flamerobin-port-helper.h"

namespace fr {

//-----------------------------------------------------------------------------
ColumnBase::ColumnBase(NodeType type, MetadataItem* parent,
        const QString& name)
    : MetadataItem(type, parent, name), hasDefaultM(false)
{
}
//-----------------------------------------------------------------------------
QString ColumnBase::getComputedSource() const
{
    return QString();
}
//-----------------------------------------------------------------------------
//! retrieve datatype from domain if possible
QString ColumnBase::getDatatype(bool useConfig)
{
    enum
    {
        showType = 0,
        showFormula,
        showAll
    };
    int flag = (useConfig ? showFormula : showType);
    if (useConfig)
        config().getValue(QString::fromLatin1("ShowComputed"), flag);
    // view columns are all computed and have their source empty
    if (flag == showFormula && !getComputedSource().isEmpty())
        return getComputedSource();

    QString ret;
    DomainPtr d = getDomain();
    QString datatype(d ? d->getDatatypeAsString() : sourceM);

    enum
    {
        showDatatype = 0,
        showDomain,
        showBoth
    };
    int show = (useConfig ? showBoth : showDatatype);
    if (useConfig)
        config().getValue(QString::fromLatin1("ShowDomains"), show);

    if (!d || d->isSystem() || show == showBoth || show == showDatatype)
        ret += datatype;

    if (d && !d->isSystem() && (show == showBoth || show == showDomain))
    {
        if (!ret.isEmpty())
            ret += QString::fromLatin1(" ");
        ret += QString::fromLatin1("(") + d->getName_() + QString::fromLatin1(")");
    }

    if (flag == showAll && !getComputedSource().isEmpty())
        ret += QString::fromLatin1(" (") + getComputedSource() + QString::fromLatin1(")");
    return ret;
}
//-----------------------------------------------------------------------------
DomainPtr ColumnBase::getDomain() const
{
    DatabasePtr db = getDatabase();
    return (db) ? db->getDomain(sourceM) : DomainPtr();
}
//-----------------------------------------------------------------------------
QString ColumnBase::getDefault() const
{
    if (hasDefaultM)
        return defaultM;
    if (DomainPtr d = getDomain())
        return d->getDefault();
    return QString();
}
//-----------------------------------------------------------------------------
bool ColumnBase::hasDefault() const
{
    if (hasDefaultM)
        return true;
    if (DomainPtr d = getDomain())
        return d->hasDefault();
    return false;
}
//-----------------------------------------------------------------------------
QString ColumnBase::getSource() const
{
    return sourceM;
}
//-----------------------------------------------------------------------------
void ColumnBase::initialize(const QString& source,
    const QString& defaultValue, bool hasDefault)
{
    bool changed = false;
    QString strippedSrc = helper::stripString(source, helper::both);
    if (sourceM != strippedSrc)
    {
        sourceM = strippedSrc;
        changed = true;
    }
    if (defaultM != defaultValue)
    {
        defaultM = defaultValue;
        changed = true;
    }
    if (hasDefaultM != hasDefault)
    {
        hasDefaultM = hasDefault;
        changed = true;
    }
    if (changed)
        notifyObservers();
}
//-----------------------------------------------------------------------------
bool ColumnBase::isNullable() const
{
    return true;
}
//-----------------------------------------------------------------------------
Column::Column(Relation* relation, const QString& name)
    : ColumnBase(ntColumn, relation, name)
{
}
//-----------------------------------------------------------------------------
void Column::initialize(bool notnull, const QString& source,
    const QString& computedSource, const QString& collation,
    const QString& defaultValue, bool hasDefault)
{
    SubjectLocker lock(this);

    ColumnBase::initialize(source, defaultValue, hasDefault);

    bool changed = false;
    if (notnullM != notnull)
    {
        notnullM = notnull;
        changed = true;
    }
    if (computedSourceM != computedSource)
    {
        computedSourceM = computedSource;
        changed = true;
    }
    QString strippedColl = helper::stripString(collation, helper::both);
    if (collationM != strippedColl)
    {
        collationM = strippedColl;
        changed = true;
    }
    if (changed)
        notifyObservers();
}
//-----------------------------------------------------------------------------
bool Column::isNullable() const
{
    if (notnullM)
        return false;
    if (DomainPtr d = getDomain())
        return d->isNullable();
    return true;
}
//-----------------------------------------------------------------------------
bool Column::hasNotNullConstraint() const
{
    return notnullM;
}
//-----------------------------------------------------------------------------
bool Column::isPrimaryKey() const
{
    Table* t = getTable();
    if (!t) // view/SP
        return false;
    ColumnConstraint *key = t->getPrimaryKey();
    if (!key)
        return false;
    for (ColumnConstraint::const_iterator it = key->begin(); it != key->end();
        ++it)
    {
        if ((*it) == getName_())
            return true;
    }
    return false;
}
//-----------------------------------------------------------------------------
bool Column::isForeignKey() const
{
    Table* t = getTable();
    if (!t) // view/SP
        return false;
    std::vector<ForeignKey> *fks = t->getForeignKeys();
    if (!fks)
        return false;
    for (std::vector<ForeignKey>::iterator it = fks->begin();
        it != fks->end(); ++it)
    {
        for (std::vector<QString>::const_iterator c2 = (*it).begin();
            c2 != (*it).end(); ++c2)
        {
            if ((*c2) == getName_())
                return true;
        }
    }
    return false;
}
//-----------------------------------------------------------------------------
bool Column::isString() const
{
    DomainPtr d = getDomain();
    return (d ? d->isString() : false);
}
//-----------------------------------------------------------------------------
Table* Column::getTable() const
{
    return dynamic_cast<Table*>(getParent());
}
//-----------------------------------------------------------------------------
QString Column::getComputedSource() const
{
    return computedSourceM;
}
//-----------------------------------------------------------------------------
QString Column::getCollation() const
{
    return collationM;
}
//-----------------------------------------------------------------------------
const QString Column::getTypeName() const
{
    return QString::fromLatin1("COLUMN");
}
//-----------------------------------------------------------------------------
QString Column::getDropSqlStatement() const
{
    return QString::fromLatin1("ALTER TABLE ") + getTable()->getQuotedName() + QString::fromLatin1(" DROP ") + getQuotedName();
}
//-----------------------------------------------------------------------------
void Column::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitColumn(*this);
}
//-----------------------------------------------------------------------------

} // namespace fr
