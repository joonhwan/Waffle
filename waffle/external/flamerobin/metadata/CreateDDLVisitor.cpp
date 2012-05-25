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


  $Id: CreateDDLVisitor.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
//-----------------------------------------------------------------------------

#include "core/ProgressIndicator.h"
#include "metadata/column.h"
#include "metadata/constraints.h"
#include "metadata/CreateDDLVisitor.h"
#include "metadata/database.h"
#include "metadata/domain.h"
#include "metadata/exception.h"
#include "metadata/function.h"
#include "metadata/generator.h"
#include "metadata/Index.h"
#include "metadata/parameter.h"
#include "metadata/procedure.h"
#include "metadata/role.h"
#include "metadata/table.h"
#include "metadata/view.h"
#include "flamerobin-port-helper.h"
#include <QTextStream>
#include <vector>

namespace fr {

// forward declaration to keep compilers happy
void addIndex(std::vector<Index> *ix, QString& sql, ColumnConstraint *cc);
//-----------------------------------------------------------------------------
CreateDDLVisitor::CreateDDLVisitor(ProgressIndicator* progressIndicator)
    : MetadataItemVisitor()
{
    progressIndicatorM = progressIndicator;
}
//-----------------------------------------------------------------------------
CreateDDLVisitor::~CreateDDLVisitor()
{
}
//-----------------------------------------------------------------------------
QString CreateDDLVisitor::getSql() const
{
    return sqlM;
}
//-----------------------------------------------------------------------------
QString CreateDDLVisitor::getPrefixSql() const
{
    return preSqlM;
}
//-----------------------------------------------------------------------------
QString CreateDDLVisitor::getSuffixSql() const
{
    return postSqlM + grantSqlM;
}
//-----------------------------------------------------------------------------
// this one is not called from "outside", but from visit(Table) function
void CreateDDLVisitor::visitColumn(Column& c)
{
    QString computed = c.getComputedSource();
    if (!computed.isEmpty())
    {
        QString add = QString::fromLatin1("ALTER TABLE ") + c.getTable()->getQuotedName()
            + QString::fromLatin1(" ADD ") + c.getQuotedName() + QString::fromLatin1(" COMPUTED BY ")
            + computed + QString::fromLatin1(";\n");
        postSqlM += add;
        sqlM += add;
        return;
    }

    preSqlM += c.getQuotedName();
	preSqlM += QString::fromLatin1(" ");
    QString collate = c.getCollation();
    if (DomainPtr d = c.getDomain())
    {
        if (d->isSystem())
        {
            preSqlM += d->getDatatypeAsString();
            QString charset = d->getCharset();
            DatabasePtr db = d->getDatabase();
            if (!charset.isEmpty())
            {
                if (!db || db->getDatabaseCharset() != charset) {
                    preSqlM += QString::fromLatin1(" CHARACTER SET ");
					preSqlM += charset;
				}
                if (db && db->isDefaultCollation(charset, collate)) {
                    collate.clear();    // don't show default collations
				}
            }
        }
        else
            preSqlM += d->getQuotedName();
    } else {
		Q_ASSERT(false);
        preSqlM +=  c.getSource();  // shouldn't happen
	}

    QString defaultVal = c.getDefault();
    if (!defaultVal.isEmpty()) {
        preSqlM += QString::fromLatin1(" DEFAULT ");
		preSqlM += defaultVal;
	}
    if (c.hasNotNullConstraint()) {
        preSqlM += QString::fromLatin1(" NOT NULL");
	}
    if (!collate.isEmpty()) {
        preSqlM += QString::fromLatin1(" COLLATE ");
		preSqlM += collate;
    }
    QString description = c.getDescription();
    if (!description.isEmpty())
    {
        QString colname(c.getName_());
        QString tabname(c.getTable()->getName_());
        description.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
        colname.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
        tabname.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
        postSqlM +=
			QString::fromLatin1(
					"UPDATE RDB$RELATION_FIELDS set RDB$DESCRIPTION = '%1' "
					"where RDB$FIELD_NAME = '%2' and "
					"RDB$RELATION_NAME = '%3';\n"
				)
			.arg(description)
			.arg(colname)
			.arg(tabname);
	}
}
//-----------------------------------------------------------------------------
template <class C, class M>
void iterateit(CreateDDLVisitor* v, C mc, ProgressIndicator* pi)
{
    Q_ASSERT(mc);

    if (pi)
    {
        pi->setProgressMessage(_("Extracting ") + mc->getName_());
        pi->stepProgress();
        pi->initProgress(QString(), mc->getChildrenCount(), 0, 2);
    }

    for (typename MetadataCollection<M>::iterator it = mc->begin();
        it != mc->end(); ++it)
    {
        if (pi)
        {
            checkProgressIndicatorCanceled(pi);
            pi->setProgressMessage(_("Extracting ") + (*it)->getName_(), 2);
            pi->stepProgress(1, 2);
        }
        (*it)->acceptVisitor(v);
    }
}
//-----------------------------------------------------------------------------
// build the sql script for entire database
void CreateDDLVisitor::visitDatabase(Database& d)
{
    if (progressIndicatorM)
        progressIndicatorM->initProgress(QString(), 10, 0, 1);

    try
    {
        preSqlM += QString::fromLatin1("/********************* ROLES **********************/\n\n");
        iterateit<RolesPtr, Role>(this, d.getRoles(), progressIndicatorM);

        preSqlM += QString::fromLatin1("/********************* UDFS ***********************/\n\n");
        iterateit<FunctionsPtr, Function>(this, d.getFunctions(),
            progressIndicatorM);

        preSqlM += QString::fromLatin1("/****************** GENERATORS ********************/\n\n");
        iterateit<GeneratorsPtr, Generator>(this, d.getGenerators(),
            progressIndicatorM);

        preSqlM += QString::fromLatin1("/******************** DOMAINS *********************/\n\n");
        iterateit<DomainsPtr, Domain>(this, d.getDomains(),
            progressIndicatorM);

        preSqlM += QString::fromLatin1("/******************* PROCEDURES ******************/\n\n");
        iterateit<ProceduresPtr, Procedure>(this, d.getProcedures(),
            progressIndicatorM);

        preSqlM += QString::fromLatin1("/******************** TABLES **********************/\n\n");
        iterateit<TablesPtr, Table>(this, d.getTables(), progressIndicatorM);

        preSqlM += QString::fromLatin1("/********************* VIEWS **********************/\n\n");
        // TODO: build dependecy tree first, and order views by it
        //       also include computed columns of tables?
        iterateit<ViewsPtr, View>(this, d.getViews(), progressIndicatorM);

        preSqlM += QString::fromLatin1("/******************* EXCEPTIONS *******************/\n\n");
        iterateit<ExceptionsPtr, Exception>(this, d.getExceptions(),
            progressIndicatorM);

        preSqlM += QString::fromLatin1("/******************** TRIGGERS ********************/\n\n");
        iterateit<TriggersPtr, Trigger>(this, d.getTriggers(),
            progressIndicatorM);
    }
    catch (CancelProgressException&)
    {
        // this is expected if user cancels the extraction
        sqlM = _("Extraction canceled");
        return;
    }

    sqlM = preSqlM + QString::fromLatin1("\n") + postSqlM + grantSqlM;
    if (progressIndicatorM)
    {
        progressIndicatorM->setProgressMessage(_("Extraction complete."));
        progressIndicatorM->setProgressPosition(10);
        progressIndicatorM->setProgressMessage(_("Done."), 2);
    }
}
//-----------------------------------------------------------------------------
void CreateDDLVisitor::visitDomain(Domain& d)
{
    preSqlM += QString::fromLatin1("CREATE DOMAIN ") + d.getQuotedName() + QString::fromLatin1("\n AS ") +
            d.getDatatypeAsString();
    QString charset = d.getCharset();
    DatabasePtr db = d.getDatabase();
    if (!charset.isEmpty() && (!db || db->getDatabaseCharset() != charset))
        preSqlM += QString::fromLatin1(" CHARACTER SET ") + charset;
    preSqlM += QString::fromLatin1("\n");
    QString dflt(d.getDefault());
    if (!dflt.isEmpty())
        preSqlM += QString::fromLatin1(" DEFAULT ") + dflt + QString::fromLatin1("\n");
    if (!d.isNullable())
        preSqlM += QString::fromLatin1(" NOT NULL\n");
    QString check = d.getCheckConstraint();
    if (!check.isEmpty())
        preSqlM += QString::fromLatin1(" ") + check + QString::fromLatin1("\n");  // already contains CHECK keyword
    QString collate = d.getCollation();
    if (!collate.isEmpty())
        preSqlM += QString::fromLatin1(" COLLATE ") + collate;
    preSqlM += QString::fromLatin1(";\n");

    QString description = d.getDescription();
    if (!description.isEmpty())
    {
		QTextStream s(&postSqlM);

        QString colname(d.getName_());
        description.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
        colname.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
        s << QString::fromLatin1("UPDATE RDB$FIELDS set\n  RDB$DESCRIPTION = '")
		  << description << QString::fromLatin1("'\n  where RDB$FIELD_NAME = '")
		  << colname << QString::fromLatin1("';\n");
    }
    sqlM = preSqlM + postSqlM;
}
//-----------------------------------------------------------------------------
void CreateDDLVisitor::visitException(Exception& e)
{
    QString ms(e.getMessage());
    ms.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));    // escape quotes
    preSqlM += QString::fromLatin1("CREATE EXCEPTION ") + e.getQuotedName() + QString::fromLatin1("\n'") +
        ms + QString::fromLatin1("';\n");

    QString description = e.getDescription();
    if (!description.isEmpty())
    {
		QTextStream s(&postSqlM);

        QString name(e.getName_());
        description.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
        name.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
        s << QString::fromLatin1("UPDATE RDB$EXCEPTIONS set\n  RDB$DESCRIPTION = '")
		  << description << QString::fromLatin1("'\n  where RDB$EXCEPTION_NAME = '")
		  << name << QString::fromLatin1("';\n");
    }
    sqlM = preSqlM + postSqlM;
}
//-----------------------------------------------------------------------------
void CreateDDLVisitor::visitForeignKey(ForeignKey& fk)
{
    Identifier reftab(fk.getReferencedTable());
    QString src_col, dest_col;
    for (std::vector<QString>::const_iterator it = fk.begin(); it != fk.end(); ++it)
    {
        if (it != fk.begin())
            src_col += QString::fromLatin1(",");
        Identifier id(*it);
        src_col += id.getQuoted();
    }
    for (std::vector<QString>::const_iterator it = fk.getReferencedColumns().begin();
        it != fk.getReferencedColumns().end(); ++it)
    {
        if (it != fk.getReferencedColumns().begin())
            dest_col += QString::fromLatin1(",");
        Identifier id(*it);
        dest_col += id.getQuoted();
    }
    postSqlM += QString::fromLatin1("ALTER TABLE ") + fk.getTable()->getQuotedName() + QString::fromLatin1(" ADD");
    if (!fk.isSystem())
        postSqlM += QString::fromLatin1(" CONSTRAINT ") + fk.getQuotedName();
    postSqlM += QString::fromLatin1("\n  FOREIGN KEY (") + src_col + QString::fromLatin1(") REFERENCES ")
        + reftab.getQuoted() + QString::fromLatin1(" (") + dest_col + QString::fromLatin1(")");
    QString upd = fk.getUpdateAction();
    if (!upd.isEmpty() && upd != QString::fromLatin1("RESTRICT"))
        postSqlM += QString::fromLatin1(" ON UPDATE ") + upd;
    QString del = fk.getDeleteAction();
    if (!del.isEmpty() && del != QString::fromLatin1("RESTRICT"))
        postSqlM += QString::fromLatin1(" ON DELETE ") + del;
    addIndex(fk.getTable()->getIndices(), postSqlM, &fk);
    postSqlM += QString::fromLatin1(";\n");
    sqlM = postSqlM;
}
//-----------------------------------------------------------------------------
void CreateDDLVisitor::visitFunction(Function& f)
{
	{
		QTextStream s(&preSqlM);
		s << f.getCreateSql() << QString::fromLatin1("\n");
	}
    QString description = f.getDescription();
    if (!description.isEmpty())
    {
        QString name(f.getName_());
        description.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
        name.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));

		QTextStream s(&postSqlM);
        s << QString::fromLatin1("UPDATE RDB$FUNCTIONS set\n  RDB$DESCRIPTION = '")
		  << description << QString::fromLatin1("'\n  where RDB$FUNCTION_NAME = '")
		  << name << QString::fromLatin1("';\n");
    }
    sqlM = preSqlM + postSqlM;
}
//-----------------------------------------------------------------------------
void CreateDDLVisitor::visitGenerator(Generator& g)
{
	{
		QTextStream s(&preSqlM);
		s << QString::fromLatin1("CREATE GENERATOR ") << g.getQuotedName() <<  QString::fromLatin1(";\n");
	}
    QString description = g.getDescription();
    if (!description.isEmpty())
    {
		QTextStream s(&postSqlM);

        QString name(g.getName_());
        description.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
        name.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
        s << QString::fromLatin1("UPDATE RDB$GENERATORS set\n  RDB$DESCRIPTION = '")
		  << description << QString::fromLatin1("'\n  where RDB$GENERATOR_NAME = '")
		  << name << QString::fromLatin1("';\n");
    }
    sqlM = preSqlM + postSqlM;
}
//-----------------------------------------------------------------------------
void CreateDDLVisitor::visitPrimaryKeyConstraint(PrimaryKeyConstraint& pk)
{
    QString sql;
    if (!pk.isSystem())     // system one, noname
        sql += QString::fromLatin1(" CONSTRAINT ") + pk.getQuotedName();
    sql += QString::fromLatin1(" PRIMARY KEY (");

    for (std::vector<QString>::const_iterator it = pk.begin(); it != pk.end(); ++it)
    {
        if (it != pk.begin())
            sql += QString::fromLatin1(",");
        Identifier id(*it);
        sql += id.getQuoted();
    }
    sql += QString::fromLatin1(")");
    addIndex(pk.getTable()->getIndices(), sql, &pk);
    preSqlM += QString::fromLatin1(",\n ") + sql;
    sqlM = QString::fromLatin1("ALTER TABLE ") + pk.getTable()->getQuotedName() + QString::fromLatin1(" ADD") +
        sql + QString::fromLatin1(";\n");
}
//-----------------------------------------------------------------------------
void CreateDDLVisitor::visitProcedure(Procedure& p)
{
    QString tempString(p.getAlterSql());
	QTextStream temp(&tempString);
    temp << QString::fromLatin1("\n");

    // grant execute on [name] to [user/role]
    const std::vector<Privilege>* priv = p.getPrivileges();
    if (priv)
    {
        for (std::vector<Privilege>::const_iterator ci = priv->begin();
            ci != priv->end(); ++ci)
        {
            grantSqlM += (*ci).getSql() + QString::fromLatin1("\n");
        }
    }

    /* description of procedure and parameters */
    QString name(p.getName_());
    name.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
    QString description = p.getDescription();
    if (!description.isEmpty())
    {
        description.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
        temp << QString::fromLatin1("UPDATE RDB$PROCEDURES set\n  RDB$DESCRIPTION = '")
             << description << QString::fromLatin1("'\n  where RDB$PROCEDURE_NAME = '")
             << name << QString::fromLatin1("';\n");
    }
    for (ParameterPtrs::iterator it = p.begin(); it != p.end(); ++it)
    {
        QString description = (*it)->getDescription();
        if (!description.isEmpty())
        {
            QString pname((*it)->getName_());
            description.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
            pname.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
            temp <<
            QString::fromLatin1("UPDATE RDB$PROCEDURE_PARAMETERS set RDB$DESCRIPTION = '")
            << description << QString::fromLatin1("'\n  where RDB$PARAMETER_NAME = '")
            << pname << QString::fromLatin1("' AND RDB$PROCEDURE_NAME = '") << name
            << QString::fromLatin1("';\n");
        }
    }
	temp.flush();

    postSqlM += tempString;
	postSqlM += QString::fromLatin1("\n");

    // tempString.replace(QString::fromLatin1("ALTER"), QString::fromLatin1("CREATE"), false);   // just first
	helper::replaceString(tempString, QString::fromLatin1("ALTER"), QString::fromLatin1("CREATE"),
					   helper::replaceFirstOnly);   // just first
    sqlM += tempString;
	sqlM += grantSqlM;

    // create empty procedure body (for database DDL dump)
    tempString = p.getAlterSql(false);    // false = only headers
    // tempString.replace(QString::fromLatin1("ALTER"), QString::fromLatin1("CREATE"), false);   // just first
	helper::replaceString(tempString, QString::fromLatin1("ALTER"), QString::fromLatin1("CREATE"),
					   helper::replaceFirstOnly);   // just first
    preSqlM += tempString;
	preSqlM += QString::fromLatin1("\n");
}
//-----------------------------------------------------------------------------
void CreateDDLVisitor::visitRole(Role& r)
{
    preSqlM += QString::fromLatin1("CREATE ROLE ") + r.getQuotedName() + QString::fromLatin1(";\n");

    // grant execute on [name] to [user/role]
    const std::vector<Privilege>* priv = r.getPrivileges();
    if (priv)
    {
        for (std::vector<Privilege>::const_iterator ci = priv->begin();
            ci != priv->end(); ++ci)
        {
            grantSqlM += (*ci).getSql();
        }
    }
    QString description = r.getDescription();
    if (!description.isEmpty())
    {
		QTextStream s(&postSqlM);

        QString name(r.getName_());
        description.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
        name.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
        s << QString::fromLatin1("UPDATE RDB$ROLES set\nRDB$DESCRIPTION = '")
		  << description << QString::fromLatin1("'\nwhere RDB$ROLE_NAME = '")
		  << name << QString::fromLatin1("';\n");
    }
    sqlM = preSqlM + QString::fromLatin1("\n") + postSqlM + grantSqlM;
}
//-----------------------------------------------------------------------------
// used by visit(Table)
void addIndex(std::vector<Index> *ix, QString& sql, ColumnConstraint *cc)
{
    // only for FB 1.5+
    if (!ix || cc->getIndexName().startsWith(QString::fromLatin1("RDB$")) || cc->getName_() == cc->getIndexName())
        return;
    for (std::vector<Index>::iterator it = ix->begin(); it != ix->end(); ++it)
    {
        if ((*it).getName_() == cc->getIndexName())
        {
            sql += QString::fromLatin1("\n  USING ");
            if ((*it).getIndexType() == Index::itDescending)
                sql += QString::fromLatin1("DESC ");
            sql += QString::fromLatin1("INDEX ") + (*it).getQuotedName();
        }
    }
}
//-----------------------------------------------------------------------------
void CreateDDLVisitor::visitTable(Table& t)
{
    int type = t.getRelationType();
    preSqlM += QString::fromLatin1("CREATE ");
    if (type == 4 || type == 5)
        preSqlM += QString::fromLatin1("GLOBAL TEMPORARY ");
    preSqlM += QString::fromLatin1("TABLE ") + t.getQuotedName();
    QString external = t.getExternalPath();
    if (!external.isEmpty())
    {
        external.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
        preSqlM += QString::fromLatin1(" EXTERNAL '") + external + QString::fromLatin1("'");
    }
    preSqlM += QString::fromLatin1("\n(\n  ");
    t.ensureChildrenLoaded();
    for (ColumnPtrs::iterator it=t.begin(); it!=t.end(); ++it)
    {
        if (it != t.begin() && (*it)->getComputedSource().isEmpty())
            preSqlM += QString::fromLatin1(",\n  ");
        visitColumn(*(*it).get());
    }

    std::vector<Index> *ix = t.getIndices();

    // primary keys (detect the name and use CONSTRAINT name PRIMARY KEY... or PRIMARY KEY(col)
    PrimaryKeyConstraint *pk = t.getPrimaryKey();
    if (pk)
        visitPrimaryKeyConstraint(*pk);

    // unique constraints
    std::vector<UniqueConstraint> *uc = t.getUniqueConstraints();
    if (uc)
        for (std::vector<UniqueConstraint>::iterator it = uc->begin(); it != uc->end(); ++it)
            visitUniqueConstraint(*it);

    // foreign keys
    std::vector<ForeignKey> *fk = t.getForeignKeys();
    if (fk)
        for (std::vector<ForeignKey>::iterator it = fk->begin(); it != fk->end(); ++it)
            visitForeignKey(*it);

    // check constraints
    std::vector<CheckConstraint> *chk = t.getCheckConstraints();
    if (chk)
    {
        for (std::vector<CheckConstraint>::iterator ci = chk->begin(); ci != chk->end(); ++ci)
        {
            postSqlM += QString::fromLatin1("ALTER TABLE ") + t.getQuotedName() + QString::fromLatin1(" ADD ");
            if (!(*ci).isSystem())
                postSqlM += QString::fromLatin1("CONSTRAINT ") + (*ci).getQuotedName();
            postSqlM += QString::fromLatin1("\n  ") + (*ci).getSource() + QString::fromLatin1(";\n");
        }
    }

    // indices
    if (ix)
    {
        for (std::vector<Index>::iterator ci = ix->begin(); ci != ix->end(); ++ci)
        {
            if ((*ci).isSystem())
                continue;
            postSqlM += QString::fromLatin1("CREATE ");
            if ((*ci).isUnique())
                postSqlM += QString::fromLatin1("UNIQUE ");
            if ((*ci).getIndexType() == Index::itDescending)
                postSqlM += QString::fromLatin1("DESCENDING ");
            postSqlM += QString::fromLatin1("INDEX ") + (*ci).getQuotedName() + QString::fromLatin1(" ON ") + t.getQuotedName();
            QString expre = (*ci).getExpression();
            if (!expre.isEmpty())
                postSqlM += QString::fromLatin1(" COMPUTED BY ") + expre;
            else
            {
                postSqlM += QString::fromLatin1(" (");
                std::vector<QString> *cols = (*ci).getSegments();
                for (std::vector<QString>::const_iterator it = cols->begin(); it != cols->end(); ++it)
                {
                    if (it != cols->begin())
                        postSqlM += QString::fromLatin1(",");
                    Identifier id(*it);
                    postSqlM += id.getQuoted();
                }
                postSqlM += QString::fromLatin1(")");
            }
            postSqlM += QString::fromLatin1(";\n");
        }
    }

    // grant sel/ins/upd/del/ref/all ON [name] to [SP,user,role]
    const std::vector<Privilege>* priv = t.getPrivileges();
    if (priv)
        for (std::vector<Privilege>::const_iterator ci = priv->begin(); ci != priv->end(); ++ci)
            grantSqlM += (*ci).getSql() + QString::fromLatin1("\n");

    preSqlM += QString::fromLatin1("\n)");
    if (type == 4)
        preSqlM += QString::fromLatin1("\nON COMMIT PRESERVE ROWS");
    preSqlM += QString::fromLatin1(";\n");

    QString description = t.getDescription();
    if (!description.isEmpty())
    {
		QTextStream s(&postSqlM);

        QString name(t.getName_());
        description.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
        name.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
        s << QString::fromLatin1("UPDATE RDB$RELATIONS set\nRDB$DESCRIPTION = '")
		  << description << QString::fromLatin1("'\nwhere RDB$RELATION_NAME = '")
		  << name << QString::fromLatin1("';\n");
    }

    sqlM = preSqlM + QString::fromLatin1("\n") + postSqlM + grantSqlM;
}
//-----------------------------------------------------------------------------
void CreateDDLVisitor::visitTrigger(Trigger& t)
{
    QString object, type;
    bool active, db;
    int position;
    t.getTriggerInfo(object, active, position, type, db);
    QString source = t.getSource();
    QString relation = t.getTriggerRelation();

	{
		QTextStream s(&preSqlM);
		s << QString::fromLatin1("SET TERM ^ ;\nCREATE TRIGGER ") << t.getQuotedName();
		if (!db)
		{
			Identifier id(relation);
			s << QString::fromLatin1(" FOR ") << id.getQuoted();
		}
		if (active)
			s << QString::fromLatin1(" ACTIVE\n");
		else
			s << QString::fromLatin1(" INACTIVE\n");
		s << type;
		s << QString::fromLatin1(" POSITION ");
		s << position << QString::fromLatin1("\n");
		s << source;
		s << QString::fromLatin1("^\nSET TERM ; ^\n");
	}

    QString description = t.getDescription();
    if (!description.isEmpty())
    {
		QTextStream s(&postSqlM);

        QString name(t.getName_());
        description.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
        name.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
        s << QString::fromLatin1("UPDATE RDB$TRIGGERS set\n  RDB$DESCRIPTION = '")
             << description << QString::fromLatin1("'\n  where RDB$TRIGGER_NAME = '")
             << name << QString::fromLatin1("';\n");
    }
    sqlM = preSqlM + postSqlM;
}
//-----------------------------------------------------------------------------
void CreateDDLVisitor::visitUniqueConstraint(UniqueConstraint& unq)
{
    QString sql;
    if (!unq.isSystem())
        sql += QString::fromLatin1(" CONSTRAINT ") + unq.getQuotedName();
    sql += QString::fromLatin1(" UNIQUE (");
    for (std::vector<QString>::const_iterator it = unq.begin(); it != unq.end(); ++it)
    {
        if (it != unq.begin())
            sql += QString::fromLatin1(",");
        Identifier id(*it);
        sql += id.getQuoted();
    }
    sql += QString::fromLatin1(")");
    addIndex(unq.getTable()->getIndices(), sql, &unq);
    preSqlM += QString::fromLatin1(",\n ") + sql;
    sqlM = QString::fromLatin1("ALTER TABLE ") + unq.getTable()->getQuotedName()
        + QString::fromLatin1(" ADD") + sql + QString::fromLatin1(";\n");
}
//-----------------------------------------------------------------------------
void CreateDDLVisitor::visitView(View& v)
{
    preSqlM += v.getCreateSql();

    // grant sel/ins/upd/del/ref/all ON [name] to [SP,user,role]
    const std::vector<Privilege>* priv = v.getPrivileges();
    if (priv)
    {
        for (std::vector<Privilege>::const_iterator ci = priv->begin();
            ci != priv->end(); ++ci)
        {
            grantSqlM += (*ci).getSql() + QString::fromLatin1("\n");
        }
    }
    QString name(v.getName_());
    name.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
    QString description = v.getDescription();
    if (!description.isEmpty())
    {
		QTextStream s(&postSqlM);

        description.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
        s << QString::fromLatin1("UPDATE RDB$RELATIONS set\n  RDB$DESCRIPTION = '")
		  << description << QString::fromLatin1("'\n  where RDB$RELATION_NAME = '")
		  << name << QString::fromLatin1("';\n");
    }

    // description for columns
    for (ColumnPtrs::iterator it = v.begin(); it != v.end();
        ++it)
    {
        QString description = (*it)->getDescription();
        if (!description.isEmpty())
        {
			QTextStream s(&postSqlM);

            QString cname((*it)->getName_());
            description.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
            cname.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));
            s << QString::fromLatin1("UPDATE RDB$RELATION_FIELDS set\n  RDB$DESCRIPTION = '")
			  << description << QString::fromLatin1("'\n  where RDB$FIELD_NAME = '")
			  << cname << QString::fromLatin1("' AND RDB$RELATION_NAME = '") << name
			  << QString::fromLatin1("';\n");
        }
    }

    sqlM += preSqlM + QString::fromLatin1("\n") + postSqlM + grantSqlM;
}
//-----------------------------------------------------------------------------

} // namespace fr
