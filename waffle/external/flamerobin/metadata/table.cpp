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


  $Id: table.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
#include "ibpp/ibpp.h"

#include "core/FRError.h"
#include "core/StringUtils.h"
#include "engine/MetadataLoader.h"
#include "frutils.h"
#include "metadata/column.h"
#include "metadata/domain.h"
#include "metadata/MetadataItemVisitor.h"
#include "metadata/table.h"

namespace fr {

//-----------------------------------------------------------------------------
Table::Table(DatabasePtr database, const QString& name)
    : Relation((hasSystemPrefix(name) ? ntSysTable : ntTable), database, name),
        primaryKeyLoadedM(false), foreignKeysLoadedM(false),
        checkConstraintsLoadedM(false), uniqueConstraintsLoadedM(false),
        indicesLoadedM(false)
{
}
//-----------------------------------------------------------------------------
QString Table::getExternalPath()
{
    ensurePropertiesLoaded();
    return externalPathM;
}
//-----------------------------------------------------------------------------
void Table::setExternalFilePath(const QString& value)
{
    externalPathM = value;
}
//-----------------------------------------------------------------------------
void Table::invalidateIndices(const QString& forIndex)
{
    if (indicesLoadedM)
    {
        bool hasit = false;
        if (forIndex.isEmpty())
            hasit = true;
        else
        {
            for (std::vector<Index>::iterator it = indicesM.begin();
                it != indicesM.end(); ++it)
            {
                if ((*it).getName_() == forIndex)
                    hasit = true;
            }
        }
        if (hasit)
        {
            indicesLoadedM = false;
            notifyObservers();
        }
    }
}
//-----------------------------------------------------------------------------
void Table::loadChildren()
{
    // force info to be reloaded if asked
    primaryKeyLoadedM = false;
    foreignKeysLoadedM = false;
    checkConstraintsLoadedM = false;
    uniqueConstraintsLoadedM = false;
    indicesLoadedM = false;

    Relation::loadChildren();
}
//-----------------------------------------------------------------------------
QString Table::getProcedureTemplate()
{
    ensureChildrenLoaded();
    QString spname = QString::fromLatin1("SP_") + getName_();
    Identifier id(spname);
    QString sql = QString::fromLatin1("SET TERM !! ;\nCREATE PROCEDURE ") + id.getQuoted() +
        QString::fromLatin1("\nRETURNS (");
    QString collist, valist, parlist;

    DatabasePtr db = getDatabase();
    QString dbcharset = db->getDatabaseCharset();
    for (ColumnPtrs::iterator i = columnsM.begin();
        i != columnsM.end(); ++i)
    {
        QString datatype;
        DomainPtr d = (*i)->getDomain();
        if (!d)
            datatype = (*i)->getDatatype();
        else
        {
            datatype = d->getDatatypeAsString();
            QString charset(d->getCharset());
            if (!charset.isEmpty() && dbcharset != charset)
                datatype += QString::fromLatin1(" CHARACTER SET ") + charset;
        }

        if (!collist.isEmpty())
        {
            valist += QString::fromLatin1(", ");
            collist += QString::fromLatin1(", ");
            parlist += QString::fromLatin1(",");
        }
        parlist += QString::fromLatin1("\n\t") + (*i)->getQuotedName() + QString::fromLatin1(" ") + datatype;
        collist += QString::fromLatin1("a.") + (*i)->getQuotedName();
        valist += QString::fromLatin1(":") + (*i)->getQuotedName();
    }
    sql += parlist;
    sql += QString::fromLatin1(")\nAS\nBEGIN\n\tFOR SELECT ") + collist + QString::fromLatin1("\n\t    FROM ")
        + getQuotedName() + QString::fromLatin1(" a\n\t    INTO ") + valist
        + QString::fromLatin1("\n\tDO\n\tBEGIN\n\t\tSUSPEND;\n\tEND\nEND!!\nSET TERM ; !!\n");
    return sql;
}
//-----------------------------------------------------------------------------
//! reads checks info from database
void Table::loadCheckConstraints()
{
    if (checkConstraintsLoadedM)
        return;
    checkConstraintsM.clear();

    DatabasePtr db = getDatabase();
    QTextCodec* conv = db->getCharsetConverter();
    MetadataLoader* loader = db->getMetadataLoader();
    // first start a transaction for metadata loading, then lock the table
    // when objects go out of scope and are destroyed, table will be unlocked
    // before the transaction is committed - any update() calls on observers
    // can possibly use the same transaction
    MetadataLoaderTransaction tr(loader);
    SubjectLocker lock(this);

    IBPP::Statement& st1 = loader->getStatement(
        "select r.rdb$constraint_name, t.rdb$trigger_source, d.rdb$field_name "
        " from rdb$relation_constraints r "
        " join rdb$check_constraints c on r.rdb$constraint_name=c.rdb$constraint_name and r.rdb$constraint_type = 'CHECK'"
        " join rdb$triggers t on c.rdb$trigger_name=t.rdb$trigger_name and t.rdb$trigger_type = 1 "
        " left join rdb$dependencies d on t.rdb$trigger_name = d.rdb$dependent_name "
        "      and d.rdb$depended_on_name = r.rdb$relation_name "
        "      and d.rdb$depended_on_type = 0 "
        " where r.rdb$relation_name=? "
        " order by 1 "
    );

    st1->Set(1, wx2std(getName_(), conv));
    st1->Execute();
    CheckConstraint *cc = 0;
    while (st1->Fetch())
    {
        std::string s;
        st1->Get(1, s);
        QString cname(std2wxIdentifier(s, conv));
        if (!cc || cname != cc->getName_()) // new constraint
        {
            QString source;
            readBlob(st1, 2, source, conv);

            CheckConstraint c;
            c.setParent(this);
            c.setName_(cname);
            c.sourceM = source;
            checkConstraintsM.push_back(c);
            cc = &checkConstraintsM.back();
        }

        if (!st1->IsNull(3))
        {
            st1->Get(3, s);
            QString fname(std2wxIdentifier(s, conv));
            cc->columnsM.push_back(fname);
        }
    }
    checkConstraintsLoadedM = true;
}
//-----------------------------------------------------------------------------
//! reads primary key info from database
void Table::loadPrimaryKey()
{
    if (primaryKeyLoadedM)
        return;
    primaryKeyM.columnsM.clear();

    DatabasePtr db = getDatabase();
    QTextCodec* conv = db->getCharsetConverter();
    MetadataLoader* loader = db->getMetadataLoader();
    // first start a transaction for metadata loading, then lock the table
    // when objects go out of scope and are destroyed, table will be unlocked
    // before the transaction is committed - any update() calls on observers
    // can possibly use the same transaction
    MetadataLoaderTransaction tr(loader);
    SubjectLocker lock(this);

    IBPP::Statement& st1 = loader->getStatement(
        "select r.rdb$constraint_name, i.rdb$field_name, r.rdb$index_name "
        "from rdb$relation_constraints r, rdb$index_segments i "
        "where r.rdb$relation_name=? and r.rdb$index_name=i.rdb$index_name and "
        "(r.rdb$constraint_type='PRIMARY KEY') order by r.rdb$constraint_name, i.rdb$field_position"
    );

    st1->Set(1, wx2std(getName_(), conv));
    st1->Execute();
    while (st1->Fetch())
    {
        std::string s;
        st1->Get(1, s);
        QString cname(std2wxIdentifier(s, conv));
        st1->Get(2, s);
        QString fname(std2wxIdentifier(s, conv));
        st1->Get(3, s);
        QString ixname(std2wxIdentifier(s, conv));

        primaryKeyM.setName_(cname);
        primaryKeyM.columnsM.push_back(fname);
        primaryKeyM.indexNameM = ixname;
    }
    primaryKeyM.setParent(this);
    primaryKeyLoadedM = true;
}
//-----------------------------------------------------------------------------
//! reads uniques from database
void Table::loadUniqueConstraints()
{
    if (uniqueConstraintsLoadedM)
        return;
    uniqueConstraintsM.clear();

    DatabasePtr db = getDatabase();
    QTextCodec* conv = db->getCharsetConverter();
    MetadataLoader* loader = db->getMetadataLoader();
    // first start a transaction for metadata loading, then lock the table
    // when objects go out of scope and are destroyed, table will be unlocked
    // before the transaction is committed - any update() calls on observers
    // can possibly use the same transaction
    MetadataLoaderTransaction tr(loader);
    SubjectLocker lock(this);

    IBPP::Statement& st1 = loader->getStatement(
        "select r.rdb$constraint_name, i.rdb$field_name, r.rdb$index_name "
        "from rdb$relation_constraints r, rdb$index_segments i "
        "where r.rdb$relation_name=? and r.rdb$index_name=i.rdb$index_name and "
        "(r.rdb$constraint_type='UNIQUE') order by r.rdb$constraint_name, i.rdb$field_position"
    );

    st1->Set(1, wx2std(getName_(), conv));
    st1->Execute();
    UniqueConstraint *cc = 0;
    while (st1->Fetch())
    {
        std::string s;
        st1->Get(1, s);
        QString cname(std2wxIdentifier(s, conv));
        st1->Get(2, s);
        QString fname(std2wxIdentifier(s, conv));
        st1->Get(3, s);
        QString ixname(std2wxIdentifier(s, conv));

        if (cc && cc->getName_() == cname)
            cc->columnsM.push_back(fname);
        else
        {
            UniqueConstraint c;
            uniqueConstraintsM.push_back(c);
            cc = &uniqueConstraintsM.back();
            cc->indexNameM = ixname;
            cc->setName_(cname);
            cc->columnsM.push_back(fname);
            cc->setParent(this);
        }
    }
    uniqueConstraintsLoadedM = true;
}
//-----------------------------------------------------------------------------
PrimaryKeyConstraint *Table::getPrimaryKey()
{
    loadPrimaryKey();
    if (primaryKeyM.columnsM.empty())  // no PK
        return 0;
    return &primaryKeyM;
}
//-----------------------------------------------------------------------------
std::vector<ForeignKey> *Table::getForeignKeys()
{
    loadForeignKeys();
    return &foreignKeysM;
}
//-----------------------------------------------------------------------------
std::vector<CheckConstraint> *Table::getCheckConstraints()
{
    loadCheckConstraints();
    return &checkConstraintsM;
}
//-----------------------------------------------------------------------------
std::vector<UniqueConstraint> *Table::getUniqueConstraints()
{
    loadUniqueConstraints();
    return &uniqueConstraintsM;
}
//-----------------------------------------------------------------------------
std::vector<Index> *Table::getIndices()
{
    loadIndices();
    return &indicesM;
}
//-----------------------------------------------------------------------------
//! reads foreign keys info from database
void Table::loadForeignKeys()
{
    if (foreignKeysLoadedM)
        return;
    foreignKeysM.clear();

    DatabasePtr db = getDatabase();
    QTextCodec* conv = db->getCharsetConverter();
    MetadataLoader* loader = db->getMetadataLoader();
    // first start a transaction for metadata loading, then lock the table
    // when objects go out of scope and are destroyed, table will be unlocked
    // before the transaction is committed - any update() calls on observers
    // can possibly use the same transaction
    MetadataLoaderTransaction tr(loader);
    SubjectLocker lock(this);

    IBPP::Statement& st1 = loader->getStatement(
        "select r.rdb$constraint_name, i.rdb$field_name, c.rdb$update_rule, "
        " c.rdb$delete_rule, c.RDB$CONST_NAME_UQ, r.rdb$index_name "
        "from rdb$relation_constraints r, rdb$index_segments i, rdb$ref_constraints c "
        "where r.rdb$relation_name=? and r.rdb$index_name=i.rdb$index_name  "
        "and r.rdb$constraint_name = c.rdb$constraint_name "
        "and (r.rdb$constraint_type='FOREIGN KEY') order by 1, i.rdb$field_position"
    );

    IBPP::Statement& st2 = loader->getStatement(
        "select r.rdb$relation_name, i.rdb$field_name"
        " from rdb$relation_constraints r"
        " join rdb$index_segments i on i.rdb$index_name = r.rdb$index_name "
        " where r.rdb$constraint_name = ?"
        " order by i.rdb$field_position "
    );

    st1->Set(1, wx2std(getName_(), conv));
    st1->Execute();
    ForeignKey *fkp = 0;
    while (st1->Fetch())
    {
        std::string s;
        st1->Get(1, s);
        QString cname(std2wxIdentifier(s, conv));
        st1->Get(2, s);
        QString fname(std2wxIdentifier(s, conv));
        st1->Get(3, s);
        QString update_rule(std2wxIdentifier(s, conv));
        st1->Get(4, s);
        QString delete_rule(std2wxIdentifier(s, conv));
        std::string ref_constraint;
        st1->Get(5, ref_constraint);
        st1->Get(6, s);
        QString ixname(std2wxIdentifier(s, conv));

        if (fkp && fkp->getName_() == cname) // add column
            fkp->columnsM.push_back(fname);
        else
        {
            ForeignKey fk;
            foreignKeysM.push_back(fk);
            fkp = &foreignKeysM.back();
            fkp->setName_(cname);
            fkp->setParent(this);
            fkp->updateActionM = update_rule;
            fkp->deleteActionM = delete_rule;
            fkp->indexNameM = ixname;

            st2->Set(1, ref_constraint);
            st2->Execute();
            std::string rtable;
            while (st2->Fetch())
            {
                st2->Get(1, rtable);
                st2->Get(2, s);
                fkp->referencedColumnsM.push_back(std2wxIdentifier(s, conv));
            }
            fkp->referencedTableM = std2wxIdentifier(rtable, conv);
            fkp->columnsM.push_back(fname);
        }
    }
    foreignKeysLoadedM = true;
}
//-----------------------------------------------------------------------------
//! reads indices from database
void Table::loadIndices()
{
    if (indicesLoadedM)
        return;
    indicesM.clear();

    DatabasePtr db = getDatabase();
    QTextCodec* conv = db->getCharsetConverter();
    MetadataLoader* loader = db->getMetadataLoader();
    // first start a transaction for metadata loading, then lock the table
    // when objects go out of scope and are destroyed, table will be unlocked
    // before the transaction is committed - any update() calls on observers
    // can possibly use the same transaction
    MetadataLoaderTransaction tr(loader);
    SubjectLocker lock(this);

    IBPP::Statement& st1 = loader->getStatement(
        "SELECT i.rdb$index_name, i.rdb$unique_flag, i.rdb$index_inactive, "
        " i.rdb$index_type, i.rdb$statistics, "
        " s.rdb$field_name, rc.rdb$constraint_name, i.rdb$expression_source "
        " from rdb$indices i "
        " left join rdb$index_segments s on i.rdb$index_name = s.rdb$index_name "
        " left join rdb$relation_constraints rc "
        "   on rc.rdb$index_name = i.rdb$index_name "
        " where i.rdb$relation_name = ? "
        " order by i.rdb$index_name, s.rdb$field_position "
    );

    st1->Set(1, wx2std(getName_(), conv));
    st1->Execute();
    Index* i = 0;
    while (st1->Fetch())
    {
        std::string s;
        st1->Get(1, s);
        QString ixname(std2wxIdentifier(s, conv));

        short unq, inactive, type;
        if (st1->IsNull(2))     // null = non-unique
            unq = 0;
        else
            st1->Get(2, unq);
        if (st1->IsNull(3))     // null = active
            inactive = 0;
        else
            st1->Get(3, inactive);
        if (st1->IsNull(4))     // null = ascending
            type = 0;
        else
            st1->Get(4, type);
        double statistics;
        if (st1->IsNull(5))     // this can happen, see bug #1825725
            statistics = -1;
        else
            st1->Get(5, statistics);

        st1->Get(6, s);
        QString fname(std2wxIdentifier(s, conv));
        QString expression;
        readBlob(st1, 8, expression, conv);

        if (i && i->getName_() == ixname)
            i->getSegments()->push_back(fname);
        else
        {
            Index x(
                unq == 1,
                inactive == 0,
                type == 0,
                statistics,
                !st1->IsNull(7),
                expression
            );
            indicesM.push_back(x);
            i = &indicesM.back();
            i->setName_(ixname);
            i->getSegments()->push_back(fname);
            i->setParent(this);
        }
    }
    indicesLoadedM = true;
}
//-----------------------------------------------------------------------------
const QString Table::getTypeName() const
{
    return QString::fromLatin1("TABLE");
}
//-----------------------------------------------------------------------------
// find all tables from "tables" which have foreign keys with "table"
// and return them in "list"
bool Table::tablesRelate(const std::vector<QString>& tables, Table* table,
                          std::vector<ForeignKey>& list)
{
    // see if "table" references some of the "tables"
    std::vector<ForeignKey> *fks = table->getForeignKeys();
    for (std::vector<ForeignKey>::iterator it = fks->begin(); it != fks->end(); ++it)
    {
        ForeignKey& fk = (*it);
        for (std::vector<QString>::const_iterator i2 = tables.begin(); i2 != tables.end(); ++i2)
            if ((*i2) == fk.referencedTableM)
                list.push_back(fk);
    }

    // see if some of the "tables" reference the "table"
    std::vector<Dependency> deplist;
    table->getDependencies(deplist, false);
    for (std::vector<Dependency>::iterator it = deplist.begin(); it != deplist.end(); ++it)
    {
        if ((*it).getType() == ntTable)
        {
            for (std::vector<QString>::const_iterator i2 = tables.begin(); i2 != tables.end(); ++i2)
            {
                if ((*i2) == (*it).getName_())
                {
                    // find foreign keys for that table
                    DatabasePtr db = table->getDatabase();
                    Table* other_table = dynamic_cast<Table*>(db->findByNameAndType(ntTable, (*i2)));
                    if (!other_table)
                        break;

                    std::vector<ForeignKey>* fks = other_table->getForeignKeys();
                    for (std::vector<ForeignKey>::iterator it = fks->begin(); it != fks->end(); ++it)
                    {
                        ForeignKey& fk = (*it);
                        if (table->getName_() == fk.referencedTableM)
                        {
                            list.push_back(fk);
                            break;  // no need for more
                        }
                    }
                }
            }
        }
    }

    return !list.empty();
}
//-----------------------------------------------------------------------------
void Table::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitTable(*this);
}
//-----------------------------------------------------------------------------
// System tables collection
SysTables::SysTables(DatabasePtr database)
    : MetadataCollection<Table>(ntSysTables, database, _("System tables"))
{
}
//-----------------------------------------------------------------------------
void SysTables::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitSysTables(*this);
}
//-----------------------------------------------------------------------------
bool SysTables::isSystem() const
{
    return true;
}
//-----------------------------------------------------------------------------
void SysTables::load(ProgressIndicator* progressIndicator)
{
    QString stmt = QString::fromLatin1(
			"select rdb$relation_name from rdb$relations"
			" where rdb$system_flag = 1"
			" and rdb$view_source is null order by 1"
		);
    setItems(getDatabase()->loadIdentifiers(stmt, progressIndicator));
}
//-----------------------------------------------------------------------------
void SysTables::loadChildren()
{
    load(0);
}
//-----------------------------------------------------------------------------
const QString SysTables::getTypeName() const
{
    return QString::fromLatin1("SYSTABLE_COLLECTION");
}
//-----------------------------------------------------------------------------
// Tables collection
Tables::Tables(DatabasePtr database)
    : MetadataCollection<Table>(ntTables, database, _("Tables"))
{
}
//-----------------------------------------------------------------------------
void Tables::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitTables(*this);
}
//-----------------------------------------------------------------------------
void Tables::load(ProgressIndicator* progressIndicator)
{
    QString stmt = QString::fromLatin1(
			"select rdb$relation_name from rdb$relations"
			" where (rdb$system_flag = 0 or rdb$system_flag is null)"
			" and rdb$view_source is null order by 1"
		);
    setItems(getDatabase()->loadIdentifiers(stmt, progressIndicator));
}
//-----------------------------------------------------------------------------
void Tables::loadChildren()
{
    load(0);
}
//-----------------------------------------------------------------------------
const QString Tables::getTypeName() const
{
    return QString::fromLatin1("TABLE_COLLECTION");
}
//-----------------------------------------------------------------------------

} // namespace fr
