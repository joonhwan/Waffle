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


  $Id: procedure.cpp 2205 2012-01-21 11:42:57Z mghie $

*/
#include <string>

#include <boost/function.hpp>

#include "ibpp/ibpp.h"

#include "core/FRError.h"
#include "core/StringUtils.h"
#include "engine/MetadataLoader.h"
#include "frutils.h"
// #include "gui/AdvancedMessageDialog.h"
#include "metadata/domain.h"
#include "metadata/MetadataItemVisitor.h"
#include "metadata/parameter.h"
#include "metadata/procedure.h"
#include "flamerobin-port-helper.h"
#include "flamerobin-ui-wrapper.h"

namespace fr {

//-----------------------------------------------------------------------------
Procedure::Procedure(DatabasePtr database, const QString& name)
    : MetadataItem(ntProcedure, database.get(), name)
{
}
//-----------------------------------------------------------------------------
void Procedure::loadChildren()
{
    // in case an exception is thrown this should be repeated
    setChildrenLoaded(false);

    DatabasePtr db = getDatabase();
    MetadataLoader* loader = db->getMetadataLoader();
    // first start a transaction for metadata loading, then lock the procedure
    // when objects go out of scope and are destroyed, procedure will be
    // unlocked before the transaction is committed - any update() calls on
    // observers can possibly use the same transaction
    // when objects go out of scope and are destroyed, object will be unlocked
    // before the transaction is committed - any update() calls on observers
    // can possibly use the same transaction
    MetadataLoaderTransaction tr(loader);
    SubjectLocker lock(db.get());
    QTextCodec* converter = db->getCharsetConverter();

    std::string sql(
        "select p.rdb$parameter_name, p.rdb$field_source, "
        "p.rdb$parameter_type, "
    );
    if (db->getInfo().getODSVersionIsHigherOrEqualTo(11, 1))
        sql += "p.rdb$default_source, p.rdb$parameter_mechanism ";
    else
        sql += "null, -1 ";
    sql +=  "from rdb$procedure_parameters p "
            "where p.rdb$PROCEDURE_name = ? "
            "order by p.rdb$parameter_type, p.rdb$PARAMETER_number";

    IBPP::Statement st1 = loader->getStatement(sql);
    st1->Set(1, wx2std(getName_(), converter));
    st1->Execute();

    ParameterPtrs parameters;
    while (st1->Fetch())
    {
        std::string s;
        st1->Get(1, s);
        QString param_name(std2wxIdentifier(s, converter));
        st1->Get(2, s);
        QString source(std2wxIdentifier(s, converter));

        short partype, mechanism = -1;
        st1->Get(3, &partype);
        bool hasDefault = false;
        QString defaultSrc;
        if (!st1->IsNull(4))
        {
            hasDefault = true;
            st1->Get(4, s);
            defaultSrc = std2wxIdentifier(s, converter);
        }
        if (!st1->IsNull(5))
            st1->Get(5, mechanism);

        ParameterPtr par = findParameter(param_name);
        if (!par)
        {
            par.reset(new Parameter(this, param_name));
            for (unsigned i = getLockCount(); i > 0; i--)
                par->lockSubject();
        }
        parameters.push_back(par);
        par->initialize(source, partype, mechanism, defaultSrc, hasDefault);
    }

    setChildrenLoaded(true);
    if (parametersM != parameters)
    {
        parametersM.swap(parameters);
        notifyObservers();
    }
}
//-----------------------------------------------------------------------------
bool Procedure::getChildren(std::vector<MetadataItem *>& temp)
{
    if (parametersM.empty())
        return false;
    std::transform(parametersM.begin(), parametersM.end(),
        std::back_inserter(temp), boost::mem_fn(&ParameterPtr::get));
    return !parametersM.empty();
}
//-----------------------------------------------------------------------------
void Procedure::lockChildren()
{
    std::for_each(parametersM.begin(), parametersM.end(),
        boost::mem_fn(&Parameter::lockSubject));
}
//-----------------------------------------------------------------------------
void Procedure::unlockChildren()
{
    std::for_each(parametersM.begin(), parametersM.end(),
        boost::mem_fn(&Parameter::unlockSubject));
}
//-----------------------------------------------------------------------------
ParameterPtrs::iterator Procedure::begin()
{
    // please - don't load here
    // this code is used to get columns we want to alert about changes
    // but if there aren't any columns, we don't want to waste time
    // loading them
    return parametersM.begin();
}
//-----------------------------------------------------------------------------
ParameterPtrs::iterator Procedure::end()
{
    // please see comment for begin()
    return parametersM.end();
}
//-----------------------------------------------------------------------------
ParameterPtrs::const_iterator Procedure::begin() const
{
    return parametersM.begin();
}
//-----------------------------------------------------------------------------
ParameterPtrs::const_iterator Procedure::end() const
{
    return parametersM.end();
}
//-----------------------------------------------------------------------------
ParameterPtr Procedure::findParameter(const QString& name) const
{
    for (ParameterPtrs::const_iterator it = parametersM.begin();
        it != parametersM.end(); ++it)
    {
        if ((*it)->getName_() == name)
            return *it;
    }
    return ParameterPtr();
}
//-----------------------------------------------------------------------------
size_t Procedure::getParamCount() const
{
    return parametersM.size();
}
//-----------------------------------------------------------------------------
QString Procedure::getOwner()
{
    DatabasePtr db = getDatabase();
    MetadataLoader* loader = db->getMetadataLoader();
    MetadataLoaderTransaction tr(loader);

    IBPP::Statement st1 = loader->getStatement(
        "select rdb$owner_name from rdb$procedures where rdb$procedure_name = ?");
    st1->Set(1, wx2std(getName_(), db->getCharsetConverter()));
    st1->Execute();
    st1->Fetch();
    std::string name;
    st1->Get(1, name);
    return std2wxIdentifier(name, db->getCharsetConverter());
}
//-----------------------------------------------------------------------------
QString Procedure::getSource()
{
    DatabasePtr db = getDatabase();
    MetadataLoader* loader = db->getMetadataLoader();
    MetadataLoaderTransaction tr(loader);

    IBPP::Statement st1 = loader->getStatement(
        "select rdb$procedure_source from rdb$procedures"
        " where rdb$procedure_name = ?");
    st1->Set(1, wx2std(getName_(), db->getCharsetConverter()));
    st1->Execute();
    st1->Fetch();
    QString source;
    readBlob(st1, 1, source, db->getCharsetConverter());
    // source.Trim(false);     // remove leading whitespace
    // return source;
	return helper::stripString(source, helper::leading);
}
//-----------------------------------------------------------------------------
QString Procedure::getDefinition()
{
    ensureChildrenLoaded();
    QString collist, parlist;
    ParameterPtrs::const_iterator lastInput, lastOutput;
    for (ParameterPtrs::const_iterator it = parametersM.begin();
        it != parametersM.end(); ++it)
    {
        if ((*it)->isOutputParameter())
            lastOutput = it;
        else
            lastInput = it;
    }
    for (ParameterPtrs::const_iterator it =
        parametersM.begin(); it != parametersM.end(); ++it)
    {
        // No need to quote domains, as currently only regular datatypes can be
        // used for SP parameters
        if ((*it)->isOutputParameter())
        {
            collist += QString::fromLatin1("    ") + (*it)->getQuotedName() + QString::fromLatin1(" ")
                + (*it)->getDomain()->getDatatypeAsString();
            if (it != lastOutput)
                collist += QString::fromLatin1(",");
            collist += QString::fromLatin1("\n");
        }
        else
        {
            parlist += QString::fromLatin1("    ") + (*it)->getQuotedName() + QString::fromLatin1(" ")
                + (*it)->getDomain()->getDatatypeAsString();
            if (it != lastInput)
                parlist += QString::fromLatin1(",");
            parlist += QString::fromLatin1("\n");
        }
    }
    QString retval = getQuotedName();
    if (!parlist.isEmpty())
        retval += QString::fromLatin1("(\n") + parlist + QString::fromLatin1(")");
    retval += QString::fromLatin1("\n");
    if (!collist.isEmpty())
        retval += QString::fromLatin1("returns:\n") + collist;
    return retval;
}
//-----------------------------------------------------------------------------
QString Procedure::getAlterSql(bool full)
{
    ensureChildrenLoaded();

    DatabasePtr db = getDatabase();

    QString sql = QString::fromLatin1("SET TERM ^ ;\nALTER PROCEDURE ") + getQuotedName();
    if (!parametersM.empty())
    {
        QString input, output;
        for (ParameterPtrs::const_iterator it = parametersM.begin();
            it != parametersM.end(); ++it)
        {
            bool mechanismIsTypeOf = false;
            QString charset;
            QString param = (*it)->getQuotedName() + QString::fromLatin1(" ");
            if (DomainPtr dm = (*it)->getDomain())
            {
                if (dm->isSystem()) // autogenerated domain -> use datatype
                {
                    param += dm->getDatatypeAsString();
                    charset = dm->getCharset();
                    if (!charset.isEmpty())
                    {
                        if (charset != db->getDatabaseCharset())
                            charset = QString::fromLatin1(" CHARACTER SET ") + charset;
                        else
                            charset = QString::fromLatin1("");
                    }
                }
                else
                {
                    mechanismIsTypeOf = (*it)->getMechanism() == 1;
                    if (mechanismIsTypeOf)
                        param += QString::fromLatin1("TYPE OF ");
                    param += dm->getQuotedName();
                }
            }
            else
                param += (*it)->getSource();

            if ((*it)->isOutputParameter())
            {
                if (output.isEmpty())
                    output += QString::fromLatin1("\nRETURNS (\n    ");
                else
                    output += QString::fromLatin1(",\n    ");
                output += param + charset;
            }
            else
            {
                if (input.isEmpty())
                    input += QString::fromLatin1(" (\n    ");
                else
                    input += QString::fromLatin1(",\n    ");
                input += param;
                if ((*it)->hasDefault() && !mechanismIsTypeOf)
                    input += QString::fromLatin1(" DEFAULT ") + (*it)->getDefault();
                input += charset;
            }
        }

        if (!input.isEmpty())
            sql += input + QString::fromLatin1(" )");
        if (!output.isEmpty())
            sql += output + QString::fromLatin1(" )");
    }
    sql += QString::fromLatin1("\nAS\n");
    if (full)
        sql += getSource();
    else
        sql += QString::fromLatin1("BEGIN SUSPEND; END");
    sql += QString::fromLatin1("^\nSET TERM ; ^\n");
    return sql;
}
//-----------------------------------------------------------------------------
void Procedure::checkDependentProcedures()
{
    // check dependencies and parameters
    ensureChildrenLoaded();
    std::vector<Dependency> deps;
    getDependencies(deps, false);

    // if there is a dependency, but parameter doesn't exist, warn the user
    int count = 0;
    QString missing;
    for (std::vector<Dependency>::iterator it = deps.begin();
        it != deps.end(); ++it)
    {
        std::vector<QString> fields;
        (*it).getFields(fields);
        for (std::vector<QString>::const_iterator ci = fields.begin();
            ci != fields.end(); ++ci)
        {
            bool found = false;
            for (ParameterPtrs::iterator i2 = begin();
                i2 != end(); ++i2)
            {
                if ((*i2)->getName_() == (*ci))
                {
                    found = true;
                    break;
                }
            }
            if (!found && ++count < 20)
            {
                missing += QString::fromLatin1("Procedure %1 depends on parameter %2.%3")
						   .arg((*it).getName_())
						   .arg(*ci)
						   .arg(QChar('\n')) // wxTextBuffer::GetEOL()
								;
            }
        }
    }
    if (count > 0)
    {
        if (count > 19)
        {
            missing += QChar('\n') // wxTextBuffer::GetEOL()
					   + QString::fromLatin1("%1 total dependencies (20 shown).")
					   .arg(count);
        }
        // showWarningDialog(0,
		// 				  _("Dependencies broken"),
		// 				  QString::fromLatin1("Some other procedures depend on %1:%2%3%4")
		// 				  .arg(getName_())
		// 				  .arg(QChar('\n')) //wxTextBuffer::GetEOL(),
		// 				  .arg(QChar('\n')) //wxTextBuffer::GetEOL(),
		// 				  .arg(missing),
		// 				  AdvancedMessageDialogButtonsOk()
        // );
		showWarningDialog(QString::fromLatin1("Dependencies broken"),
						  QString::fromLatin1("Some other procedures depend on %1:%2%3%4")
						  .arg(getName_())
						  .arg(QChar('\n')) //wxTextBuffer::GetEOL(),
						  .arg(QChar('\n')) //wxTextBuffer::GetEOL(),
						  .arg(missing));
    }
}
//-----------------------------------------------------------------------------
std::vector<Privilege>* Procedure::getPrivileges()
{
    // load privileges from database and return the pointer to collection
    DatabasePtr db = getDatabase();
    MetadataLoader* loader = db->getMetadataLoader();
    // first start a transaction for metadata loading, then lock the procedure
    // when objects go out of scope and are destroyed, procedure will be
    // unlocked before the transaction is committed - any update() calls on
    // observers can possibly use the same transaction
    MetadataLoaderTransaction tr(loader);
    SubjectLocker lock(this);
    QTextCodec* converter = db->getCharsetConverter();

    privilegesM.clear();

    IBPP::Statement st1 = loader->getStatement(
        "select RDB$USER, RDB$USER_TYPE, RDB$GRANTOR, RDB$PRIVILEGE, "
        "RDB$GRANT_OPTION, RDB$FIELD_NAME "
        "from RDB$USER_PRIVILEGES "
        "where RDB$RELATION_NAME = ? and rdb$object_type = 5 "
        "order by rdb$user, rdb$user_type, rdb$privilege"
    );
    st1->Set(1, wx2std(getName_(), converter));
    st1->Execute();
    std::string lastuser;
    int lasttype = -1;
    Privilege *pr = 0;
    while (st1->Fetch())
    {
        std::string user, grantor, privilege, field;
        int usertype, grantoption = 0;
        st1->Get(1, user);
        st1->Get(2, usertype);
        st1->Get(3, grantor);
        st1->Get(4, privilege);
        if (!st1->IsNull(5))
            st1->Get(5, grantoption);
        st1->Get(6, field);
        if (!pr || user != lastuser || usertype != lasttype)
        {
            Privilege p(this, std2wxIdentifier(user, converter),
                usertype);
            privilegesM.push_back(p);
            pr = &privilegesM.back();
            lastuser = user;
            lasttype = usertype;
        }
        pr->addPrivilege(privilege[0], std2wx(grantor, converter),
            grantoption == 1);
    }
    return &privilegesM;
}
//-----------------------------------------------------------------------------
const QString Procedure::getTypeName() const
{
    return QString::fromLatin1("PROCEDURE");
}
//-----------------------------------------------------------------------------
void Procedure::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitProcedure(*this);
}
//-----------------------------------------------------------------------------
// Procedures collection
Procedures::Procedures(DatabasePtr database)
    : MetadataCollection<Procedure>(ntProcedures, database, _("Procedures"))
{
}
//-----------------------------------------------------------------------------
void Procedures::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitProcedures(*this);
}
//-----------------------------------------------------------------------------
void Procedures::load(ProgressIndicator* progressIndicator)
{
    QString stmt = QString::fromLatin1(
			"select rdb$procedure_name from rdb$procedures"
			" where (rdb$system_flag = 0 or rdb$system_flag is null)"
			" order by 1"
		);
    setItems(getDatabase()->loadIdentifiers(stmt, progressIndicator));
}
//-----------------------------------------------------------------------------
void Procedures::loadChildren()
{
    load(0);
}
//-----------------------------------------------------------------------------
const QString Procedures::getTypeName() const
{
    return QString::fromLatin1("PROCEDURE_COLLECTION");
}
//-----------------------------------------------------------------------------

} // namespace fr

