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


  $Id: database.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
#include <algorithm>
#include <functional>

#include <boost/function.hpp>

#include "config/Config.h"
#include "config/DatabaseConfig.h"
#include "core/FRError.h"
#include "core/ProgressIndicator.h"
#include "core/StringUtils.h"
#include "engine/MetadataLoader.h"
// #include "MasterPassword.h"
#include "metadata/column.h"
#include "metadata/database.h"
#include "metadata/domain.h"
#include "metadata/exception.h"
#include "metadata/function.h"
#include "metadata/generator.h"
#include "metadata/MetadataItemVisitor.h"
#include "metadata/parameter.h"
#include "metadata/procedure.h"
#include "metadata/role.h"
#include "metadata/root.h"
#include "metadata/server.h"
#include "metadata/table.h"
#include "metadata/trigger.h"
#include "metadata/view.h"
#include "sql/SqlStatement.h"
#include "sql/SqlTokenizer.h"
#include "flamerobin-port-helper.h"

namespace fr {

//-----------------------------------------------------------------------------
// CharacterSet class
CharacterSet::CharacterSet(const QString& name, int id, int bytesPerChar)
    : nameM(name), idM(id), bytesPerCharM(bytesPerChar)
{
}
//-----------------------------------------------------------------------------
bool CharacterSet::operator< (const CharacterSet& other) const
{
    return nameM < other.nameM;
}
//-----------------------------------------------------------------------------
int CharacterSet::getBytesPerChar() const
{
    return bytesPerCharM;
}
//-----------------------------------------------------------------------------
int CharacterSet::getId() const
{
    return idM;
}
//-----------------------------------------------------------------------------
QString CharacterSet::getName() const
{
    return nameM;
}
//-----------------------------------------------------------------------------
// Credentials class
void Credentials::setCharset(const QString& value)
{
    charsetM = value;
}
//-----------------------------------------------------------------------------
void Credentials::setUsername(const QString& value)
{
    usernameM = value;
}
//-----------------------------------------------------------------------------
void Credentials::setPassword(const QString& value)
{
    passwordM = value;
}
//-----------------------------------------------------------------------------
void Credentials::setRole(const QString& value)
{
    roleM = value;
}
//-----------------------------------------------------------------------------
QString Credentials::getCharset() const
{
    return charsetM;
}
//-----------------------------------------------------------------------------
QString Credentials::getUsername() const
{
    return usernameM;
}
//-----------------------------------------------------------------------------
QString Credentials::getPassword() const
{
    return passwordM;
}
//-----------------------------------------------------------------------------
QString Credentials::getRole() const
{
    return roleM;
}
//-----------------------------------------------------------------------------
int DatabaseInfo::getBuffers() const
{
    return buffersM;
}
//-----------------------------------------------------------------------------
int DatabaseInfo::getDialect() const
{
    return dialectM;
}
//-----------------------------------------------------------------------------
bool DatabaseInfo::getForcedWrites() const
{
    return forcedWritesM;
}
//-----------------------------------------------------------------------------
bool DatabaseInfo::getReserve() const
{
    return reserveM;
}
//-----------------------------------------------------------------------------
int DatabaseInfo::getNextTransaction() const
{
    return nextTransactionM;
}
//-----------------------------------------------------------------------------
int DatabaseInfo::getODS() const
{
    return odsM;
}
//-----------------------------------------------------------------------------
int DatabaseInfo::getODSMinor() const
{
    return odsMinorM;
}
//-----------------------------------------------------------------------------
bool DatabaseInfo::getODSVersionIsHigherOrEqualTo(int versionMajor) const
{
    return odsM >= versionMajor;
}
//-----------------------------------------------------------------------------
bool DatabaseInfo::getODSVersionIsHigherOrEqualTo(int versionMajor,
    int versionMinor) const
{
    return odsM > versionMajor
        || (odsM == versionMajor && odsMinorM >= versionMinor);
}
//-----------------------------------------------------------------------------
int DatabaseInfo::getOldestActiveTransaction() const
{
    return oldestActiveTransactionM;
}
//-----------------------------------------------------------------------------
int DatabaseInfo::getOldestSnapshot() const
{
    return oldestSnapshotM;
}
//-----------------------------------------------------------------------------
int DatabaseInfo::getOldestTransaction() const
{
    return oldestTransactionM;
}
//-----------------------------------------------------------------------------
int DatabaseInfo::getPageSize() const
{
    return pageSizeM;
}
//-----------------------------------------------------------------------------
int DatabaseInfo::getPages() const
{
    return pagesM;
}
//-----------------------------------------------------------------------------
bool DatabaseInfo::getReadOnly() const
{
    return readOnlyM;
}
//-----------------------------------------------------------------------------
int64_t DatabaseInfo::getSizeInBytes() const
{
    return static_cast<int64_t>(getPages()) * getPageSize();
}
//-----------------------------------------------------------------------------
int DatabaseInfo::getSweep() const
{
    return sweepM;
}
//-----------------------------------------------------------------------------
void DatabaseInfo::load(const IBPP::Database& database)
{
    database->Info(&odsM, &odsMinorM, &pageSizeM, &pagesM,
        &buffersM, &sweepM, &forcedWritesM, &reserveM, &readOnlyM);
    database->TransactionInfo(&oldestTransactionM, &oldestActiveTransactionM,
        &oldestSnapshotM, &nextTransactionM);
    dialectM = database->Dialect();
    // loadTimeMillisM = ::wxGetLocalTimeMillis();
	loadTimer.start();
}
//-----------------------------------------------------------------------------
void DatabaseInfo::reloadIfNecessary(const IBPP::Database& database)
{
    // wxLongLong millisNow = ::wxGetLocalTimeMillis();
    // // value may jump or even actually decrease, for instance on timezone
    // // change or when daylight saving time ends...
    // wxLongLong millisDelta = millisNow - loadTimeMillisM;
	qint64 millisDelta = loadTimer.elapsed();
    if (millisDelta >= 1000 || millisDelta <= -1000)
        load(database);
}
//-----------------------------------------------------------------------------
// DatabaseAuthenticationMode class
DatabaseAuthenticationMode::DatabaseAuthenticationMode()
    : modeM(UseSavedPassword)
{
}
//-----------------------------------------------------------------------------
int DatabaseAuthenticationMode::getMode() const
{
    return int(modeM);
}
//-----------------------------------------------------------------------------
void DatabaseAuthenticationMode::setMode(int mode)
{
    switch (mode)
    {
        case UseSavedPassword:
        case UseSavedEncryptedPwd:
        case AlwaysEnterPassword:
        case TrustedUser:
            modeM = Mode(mode);
            break;
        default:
            Q_ASSERT(false);
    }
}
//-----------------------------------------------------------------------------
QString DatabaseAuthenticationMode::getConfigValue() const
{
    switch (modeM)
    {
        case UseSavedEncryptedPwd:
            return QString::fromLatin1("encpwd");
        case AlwaysEnterPassword:
            return QString::fromLatin1("askpwd");
        case TrustedUser:
            return QString::fromLatin1("trusted");
        default:
            return QString::fromLatin1("pwd");
    }
}
//-----------------------------------------------------------------------------
void DatabaseAuthenticationMode::setConfigValue(const QString& value)
{
    if (value == QString::fromLatin1("pwd"))
        modeM = UseSavedPassword;
    else if (value == QString::fromLatin1("encpwd"))
        modeM = UseSavedEncryptedPwd;
    else if (value == QString::fromLatin1("askpwd"))
        modeM = AlwaysEnterPassword;
    else if (value == QString::fromLatin1("trusted"))
        modeM = TrustedUser;
    else
        Q_ASSERT(false);
}
//-----------------------------------------------------------------------------
void DatabaseAuthenticationMode::setStoreEncryptedPassword()
{
    // ignore if old setting found after new mode has been set already
    if (modeM == UseSavedPassword)
        modeM = UseSavedEncryptedPwd;
}
//-----------------------------------------------------------------------------
bool DatabaseAuthenticationMode::getAlwaysAskForPassword() const
{
    return modeM == AlwaysEnterPassword;
}
//-----------------------------------------------------------------------------
bool DatabaseAuthenticationMode::getIgnoreUsernamePassword() const
{
    return modeM == TrustedUser;
}
//-----------------------------------------------------------------------------
bool DatabaseAuthenticationMode::getUseEncryptedPassword() const
{
    // return modeM == UseSavedEncryptedPwd;
	return false; // TODO currently no support encryption.
}
//-----------------------------------------------------------------------------
// Database class
Database::Database()
    : MetadataItem(ntDatabase), metadataLoaderM(0), connectedM(false),
        connectionCredentialsM(0), charsetConverterM(0), idM(0)
{
}
//-----------------------------------------------------------------------------
Database::~Database()
{
    resetCredentials();
}
//-----------------------------------------------------------------------------
void Database::prepareTemporaryCredentials()
{
    resetCredentials();
    connectionCredentialsM = new Credentials;
    connectionCredentialsM->setCharset(credentialsM.getCharset()); // default to database charset
}
//-----------------------------------------------------------------------------
void Database::resetCredentials()
{
    if (connectionCredentialsM)  // i.e. there is some other
    {
        delete connectionCredentialsM;
        connectionCredentialsM = 0;
    }
}
//----------------------------------------------------------------------------
void Database::getIdentifiers(std::vector<Identifier>& temp)
{
    checkConnected(_("getIdentifiers"));
    std::transform(tablesM->begin(), tablesM->end(),
        std::back_inserter(temp), boost::mem_fn(&MetadataItem::getIdentifier));
    std::transform(sysTablesM->begin(), sysTablesM->end(),
        std::back_inserter(temp), boost::mem_fn(&MetadataItem::getIdentifier));
    std::transform(viewsM->begin(), viewsM->end(),
        std::back_inserter(temp), boost::mem_fn(&MetadataItem::getIdentifier));
    std::transform(proceduresM->begin(), proceduresM->end(),
        std::back_inserter(temp), boost::mem_fn(&MetadataItem::getIdentifier));
    std::transform(triggersM->begin(), triggersM->end(),
        std::back_inserter(temp), boost::mem_fn(&MetadataItem::getIdentifier));
    std::transform(rolesM->begin(), rolesM->end(),
        std::back_inserter(temp), boost::mem_fn(&MetadataItem::getIdentifier));
    std::transform(generatorsM->begin(), generatorsM->end(),
        std::back_inserter(temp), boost::mem_fn(&MetadataItem::getIdentifier));
    std::transform(functionsM->begin(), functionsM->end(),
        std::back_inserter(temp), boost::mem_fn(&MetadataItem::getIdentifier));
    std::transform(userDomainsM->begin(), userDomainsM->end(),
        std::back_inserter(temp), boost::mem_fn(&MetadataItem::getIdentifier));
    std::transform(exceptionsM->begin(), exceptionsM->end(),
        std::back_inserter(temp), boost::mem_fn(&MetadataItem::getIdentifier));
}
//-----------------------------------------------------------------------------
// This could be moved to Column class
QString Database::loadDomainNameForColumn(const QString& table,
    const QString& field)
{
    MetadataLoader* loader = getMetadataLoader();
    MetadataLoaderTransaction tr(loader);
    QTextCodec* converter = getCharsetConverter();

    IBPP::Statement& st1 = loader->getStatement(
        "select rdb$field_source from rdb$relation_fields"
        " where rdb$relation_name = ? and rdb$field_name = ?"
    );
    st1->Set(1, wx2std(table, converter));
    st1->Set(2, wx2std(field, converter));
    st1->Execute();
    st1->Fetch();
    std::string domain;
    st1->Get(1, domain);
    return std2wxIdentifier(domain, converter);
}
//-----------------------------------------------------------------------------
void Database::getDatabaseTriggers(std::vector<Trigger *>& list)
{
    MetadataLoader* loader = getMetadataLoader();
    MetadataLoaderTransaction tr(loader);
    QTextCodec* converter = getCharsetConverter();

    IBPP::Statement& st1 = loader->getStatement(
        "select rdb$trigger_name from rdb$triggers "
        "where rdb$relation_name is null "
        "order by rdb$trigger_sequence"
    );
    st1->Execute();
    while (st1->Fetch())
    {
        std::string name;
        st1->Get(1, name);
        Trigger* t = dynamic_cast<Trigger*>(findByNameAndType(ntTrigger,
            std2wxIdentifier(name, converter)));
        if (t)
            list.push_back(t);
    }
}
//-----------------------------------------------------------------------------
CharacterSet Database::getCharsetById(int id)
{
    // if it contains both charset and collation as 2 bytes
    id %= 256;

    loadCollations();
    for (std::multimap<CharacterSet, QString>::iterator it =
        collationsM.begin(); it != collationsM.end(); ++it)
    {
        if ((*it).first.getId() == id)
            return (*it).first;
    }
    throw FRError(QObject::tr("Character set ID %1 not found.")
				  .arg(id));
}
//-----------------------------------------------------------------------------
//! returns all collations for a given charset
QStringList Database::getCollations(const QString& charset)
{
    loadCollations();
    QStringList collations;
    std::multimap<CharacterSet, QString>::iterator low, high;
    high = collationsM.upper_bound(charset);
    for (low = collationsM.lower_bound(charset); low != high; ++low)
        collations.push_back((*low).second);
    return collations;
}
//-----------------------------------------------------------------------------
DomainPtr Database::getDomain(const QString& name)
{
    // return domain if already loaded
    if (MetadataItem::hasSystemPrefix(name))
    {
        if (DomainPtr domain = sysDomainsM->findByName(name))
            return domain;
    }
    else
    {
        if (DomainPtr domain = userDomainsM->findByName(name))
            return domain;
    }

    MetadataLoader* loader = getMetadataLoader();
    MetadataLoaderTransaction tr(loader);

    IBPP::Statement& st1 = loader->getStatement(
        "select count(*) from rdb$fields f"
        " left outer join rdb$types t on f.rdb$field_type=t.rdb$type"
        " where t.rdb$field_name='RDB$FIELD_TYPE' and f.rdb$field_name = ?"
    );
    st1->Set(1, wx2std(name, getCharsetConverter()));
    st1->Execute();
    if (st1->Fetch())
    {
        int c;
        st1->Get(1, c);
        if (c > 0)
        {
            if (MetadataItem::hasSystemPrefix(name))
                return sysDomainsM->insert(name);
            else
                return userDomainsM->insert(name);
        }
    }
    return DomainPtr();
}
//-----------------------------------------------------------------------------
bool Database::isDefaultCollation(const QString& charset,
    const QString& collate)
{
    loadCollations();
    // no collations for charset
    if (collationsM.lower_bound(charset) == collationsM.upper_bound(charset))
        return false;
    return ((*(collationsM.lower_bound(charset))).second == collate);
}
//-----------------------------------------------------------------------------
//! load charset-collation pairs if needed
void Database::loadCollations()
{
    if (!collationsM.empty())
        return;

    MetadataLoader* loader = getMetadataLoader();
    MetadataLoaderTransaction tr(loader);
    QTextCodec* converter = getCharsetConverter();

    IBPP::Statement& st1 = loader->getStatement(
        "select c.rdb$character_set_name, k.rdb$collation_name, "
        " c.RDB$CHARACTER_SET_ID, c.RDB$BYTES_PER_CHARACTER "
        " from rdb$character_sets c"
        " left outer join rdb$collations k "
        "   on c.rdb$character_set_id = k.rdb$character_set_id "
        " order by c.rdb$character_set_name, k.rdb$collation_id");
    st1->Execute();
    while (st1->Fetch())
    {
        std::string s;
        st1->Get(1, s);
        QString charset(std2wxIdentifier(s, converter));
        st1->Get(2, s);
        QString collation(std2wxIdentifier(s, converter));
        int charsetId, bytesPerChar;
        st1->Get(3, &charsetId);
        st1->Get(4, &bytesPerChar);
        CharacterSet cs(charset, charsetId, bytesPerChar);
        collationsM.insert(std::multimap<CharacterSet, QString>::value_type(
            cs, collation));
    }
}
//-----------------------------------------------------------------------------
QString Database::getTableForIndex(const QString& indexName)
{
    MetadataLoader* loader = getMetadataLoader();
    MetadataLoaderTransaction tr(loader);

    IBPP::Statement& st1 = loader->getStatement(
        "SELECT rdb$relation_name from rdb$indices where rdb$index_name = ?");
    st1->Set(1, wx2std(indexName, getCharsetConverter()));
    st1->Execute();

    QString tableName;
    if (st1->Fetch())
    {
        std::string s;
        st1->Get(1, s);
        tableName = std2wxIdentifier(s, getCharsetConverter());
    }
    return tableName;
}
//-----------------------------------------------------------------------------
void Database::loadGeneratorValues()
{
    MetadataLoader* loader = getMetadataLoader();
    // first start a transaction for metadata loading, then lock the database
    // when objects go out of scope and are destroyed, database will be
    // unlocked before the transaction is committed - any update() calls on
    // observers can possibly use the same transaction
    MetadataLoaderTransaction tr(loader);
    SubjectLocker lock(this);

    for (Generators::iterator it = generatorsM->begin();
        it != generatorsM->end(); ++it)
    {
        // make sure generator value is reloaded from database
        (*it)->invalidate();
        (*it)->ensurePropertiesLoaded();
    }
}
//-----------------------------------------------------------------------------
DatabasePtr Database::getDatabase() const
{
    return (const_cast<Database*>(this))->shared_from_this();
}
//-----------------------------------------------------------------------------
MetadataItem* Database::findByName(const QString& name)
{
    if (!isConnected())
        return 0;

    for (int n = (int)ntTable; n < (int)ntLastType; n++)
    {
        MetadataItem* m = findByNameAndType((NodeType)n, name);
        if (m)
            return m;
    }
    return 0;
}
//-----------------------------------------------------------------------------
MetadataItem* Database::findByNameAndType(NodeType nt, const QString& name)
{
    if (!isConnected())
        return 0;

    switch (nt)
    {
        case ntTable:
            return tablesM->findByName(name).get();
            break;
        case ntSysTable:
            return sysTablesM->findByName(name).get();
            break;
        case ntView:
            return viewsM->findByName(name).get();
            break;
        case ntTrigger:
            return triggersM->findByName(name).get();
            break;
        case ntProcedure:
            return proceduresM->findByName(name).get();
            break;
        case ntFunction:
            return functionsM->findByName(name).get();
            break;
        case ntGenerator:
            return generatorsM->findByName(name).get();
            break;
        case ntRole:
            return rolesM->findByName(name).get();
            break;
        case ntSysRole:
            return sysRolesM->findByName(name).get();
            break;
        case ntDomain:
            return userDomainsM->findByName(name).get();
            break;
        case ntSysDomain:
            return sysDomainsM->findByName(name).get();
            break;
        case ntException:
            return exceptionsM->findByName(name).get();
            break;
        default:
            return 0;
    };
}
//-----------------------------------------------------------------------------
Relation* Database::findRelation(const Identifier& name)
{
    QString s(name.get());
    if (TablePtr t = tablesM->findByName(s))
        return t.get();
    if (ViewPtr v = viewsM->findByName(s))
        return v.get();
    if (TablePtr t = sysTablesM->findByName(s))
        return t.get();
    return 0;
}
//-----------------------------------------------------------------------------
Relation* Database::getRelationForTrigger(Trigger* trigger)
{
    if (!trigger)
        return 0;
    QString relName = trigger->getTriggerRelation();
    if (relName.isEmpty())
        return 0;
    return findRelation(Identifier(relName));
}
//-----------------------------------------------------------------------------
void Database::dropObject(MetadataItem* object)
{
    // find the collection that contains it, and remove it
    NodeType nt = object->getType();
    switch (nt)
    {
        case ntTable:
            tablesM->remove((Table*)object);
            break;
        case ntSysTable:
            sysTablesM->remove((Table*)object);
            break;
        case ntView:
            viewsM->remove((View*)object);
            break;
        case ntTrigger:
            triggersM->remove((Trigger*)object);
            break;
        case ntProcedure:
            proceduresM->remove((Procedure*)object);
            break;
        case ntFunction:
            functionsM->remove((Function*)object);
            break;
        case ntGenerator:
            generatorsM->remove((Generator*)object);
            break;
        case ntRole:
            rolesM->remove((Role*)object);
            break;
        case ntSysRole:
            sysRolesM->remove((Role*)object);
            break;
        case ntDomain:
            userDomainsM->remove((Domain*)object);
            break;
        case ntSysDomain:
            sysDomainsM->remove((Domain*)object);
            break;
        case ntException:
            exceptionsM->remove((Exception*)object);
            break;
        default:
            return;
    };
}
//-----------------------------------------------------------------------------
void Database::addObject(NodeType type, const QString& name)
{
    switch (type)
    {
        case ntTable:
            tablesM->insert(name);
            break;
        case ntSysTable:
            sysTablesM->insert(name);
            break;
        case ntView:
            viewsM->insert(name);
            break;
        case ntProcedure:
            proceduresM->insert(name);
            break;
        case ntTrigger:
            triggersM->insert(name);
            break;
        case ntRole:
            rolesM->insert(name);
            break;
        case ntSysRole:
            sysRolesM->insert(name);
            break;
        case ntGenerator:
            generatorsM->insert(name);
            break;
        case ntFunction:
            functionsM->insert(name);
            break;
        case ntDomain:
            userDomainsM->insert(name);
            break;
        case ntSysDomain:
            sysDomainsM->insert(name);
            break;
        case ntException:
            exceptionsM->insert(name);
            break;
        default:
            break;
    }
}
//-----------------------------------------------------------------------------
//! reads a DDL statement and acts accordingly
//
// drop [object_type] [name]
// alter [object_type] [name]
// create [object_type] [name]
// alter table [name] alter [column] type [domain or datatype]
// declare external function [name]
// set null flag via system tables update
void Database::parseCommitedSql(const SqlStatement& stm)
{
    if (!stm.isDDL())
        return;    // return false only on IBPP exception

    if (stm.actionIs(actGRANT))
    {
        MetadataItem *obj = stm.getObject();
        if (obj)
            obj->notifyObservers();
        return;
    }

    if (stm.actionIs(actDROP, ntIndex))
    {
        // the affected table will recognize its index (if loaded)
        Tables::iterator it;
        for (it = tablesM->begin(); it != tablesM->end(); ++it)
            (*it)->invalidateIndices(stm.getName());
        return;
    }

    // handle "CREATE INDEX", "ALTER INDEX" and "SET STATISTICS INDEX"
    if (stm.getObjectType() == ntIndex && ( stm.actionIs(actCREATE)
        || stm.actionIs(actALTER) || stm.actionIs(actSET)))
    {
        QString tableName = getTableForIndex(stm.getName());
        MetadataItem* m = findByNameAndType(ntTable, tableName);
        if (Table* t = dynamic_cast<Table*>(m))
            t->invalidateIndices();
        return;
    }

    // update all TABLEs, VIEWs and DATABASE on "DROP TRIGGER"
    if (stm.actionIs(actDROP, ntTrigger))
    {
        Tables::iterator itt;
        for (itt = tablesM->begin(); itt != tablesM->end(); itt++)
            (*itt)->notifyObservers();
        Views::iterator itv;
        for (itv = viewsM->begin(); itv != viewsM->end(); itv++)
            (*itv)->notifyObservers();
        notifyObservers();
    }

    if (stm.actionIs(actCREATE) || stm.actionIs(actDECLARE))
    {
        addObject(stm.getObjectType(), stm.getName());
        // when trigger created: force relations to update their property pages
        Relation *r = stm.getCreateTriggerRelation();
        if (r)
            r->notifyObservers();
        else if (stm.getObjectType() == ntTrigger) // database trigger probably
            notifyObservers();
        return;
    }

    MetadataItem *object = stm.getObject();
    if (!object)
        return;

    if (stm.actionIs(actSET, ntGenerator) ||
        stm.actionIs(actALTER, ntGenerator))
    {
        if (Generator* g = dynamic_cast<Generator*>(object))
        {
            // make sure value is reloaded from database
            g->invalidate();
            g->ensurePropertiesLoaded();
        }
        return;
    }

    if (stm.actionIs(actDROP))
    {
        dropObject(object);
        if (stm.getObjectType() == ntTable || stm.getObjectType() == ntView)
        {
            Triggers::iterator it = triggersM->begin();
            while (it != triggersM->end())
            {
                Relation* r = getRelationForTrigger((*it).get());
                if (!r || r->getIdentifier().equals(stm.getIdentifier()))
                {
                    dropObject((*it).get());
                    it = triggersM->begin();
                }
                else
                    it++;
            }
        }
        return;
    }

    if (stm.isAlterColumn())
    {
        if (stm.isDatatype())
        {
            QString domainName(loadDomainNameForColumn(stm.getName(),
                stm.getFieldName()));
            if (MetadataItem::hasSystemPrefix(domainName))
            {
                DomainPtr d = sysDomainsM->findByName(domainName);
                if (!d)     // domain does not exist in DBH
                    d = sysDomainsM->insert(domainName);
                d->invalidate();
            }
            else
            {
                DomainPtr d = userDomainsM->findByName(domainName);
                if (!d)     // domain does not exist in DBH
                    d = userDomainsM->insert(domainName);
                d->invalidate();
            }
        }
        else
        {
            // there is (maybe) an extra RDB$domain in domainsM, but we can leave it there
            // as it is not going to hurt anyone
            // Besides, it appears that domain is left in database too (not cleared)
            // so we won't call this: loadObjects(ntDomain);
        }
    }

    if (stm.actionIs(actCOMMENT))
    {
        object->invalidateDescription();
        return;
    }

    if (stm.actionIs(actALTER))
    {
        // TODO: this is a place where we would simply call virtual invalidate() function
        // and object would do whatever it needs to
        switch (stm.getObjectType())
        {
            case ntProcedure:
                object->invalidate();
                dynamic_cast<Procedure*>(object)->checkDependentProcedures();
                break;
            case ntTrigger:
                {
                    Trigger* tr = dynamic_cast<Trigger*>(object);
                    if (tr)
                    {
                        tr->invalidate();
                        Relation* r = getRelationForTrigger(tr);
                        if (r)
                            r->notifyObservers();
                        else  // database trigger
                            notifyObservers();
                    }
                    break;
                }
            case ntDomain:
                object->invalidate();
                // notify all table columns with that domain
                for (Tables::iterator it = tablesM->begin();
                    it != tablesM->end(); ++it)
                {
                    for (ColumnPtrs::iterator itColumn = (*it)->begin();
                        itColumn != (*it)->end(); ++itColumn)
                    {
                        if ((*itColumn)->getSource() == stm.getName())
                            (*itColumn)->invalidate();
                    }
                }
                break;
            default:
                // calls notifyObservers() only in the base class
                // descendent classes are free to put there whatever it takes...
                object->invalidate();
                break;
        }
    }
}
//-----------------------------------------------------------------------------
void Database::drop()
{
    databaseM->Drop();
    setDisconnected();
}
//-----------------------------------------------------------------------------
void Database::reconnect()
{
    // must recreate, because IBPP::Database member will become invalid
    delete metadataLoaderM;
    metadataLoaderM = 0;

    databaseM->Disconnect();
    databaseM->Connect();
}
//-----------------------------------------------------------------------------
// the caller of this function should check whether the database object has the
// password set, and if it does not, it should provide the password
//               and if it does, just provide that password
void Database::connect(const QString& password, ProgressIndicator* indicator)
{
    if (connectedM)
        return;

    try
    {
        createCharsetConverter();

        if (indicator)
        {
            indicator->doShow();
            indicator->initProgressIndeterminate(QString::fromLatin1("Establishing connection..."));
        }

        DatabasePtr me(shared_from_this());
        userDomainsM.reset(new Domains(me));
        sysDomainsM.reset(new SysDomains(me));
        exceptionsM.reset(new Exceptions(me));
        functionsM.reset(new Functions(me));
        generatorsM.reset(new Generators(me));
        proceduresM.reset(new Procedures(me));
        rolesM.reset(new Roles(me));
        sysRolesM.reset(new SysRoles(me));
        triggersM.reset(new Triggers(me));
        tablesM.reset(new Tables(me));
        sysTablesM.reset(new SysTables(me));
        viewsM.reset(new Views(me));

        bool useUserNamePwd = !authenticationModeM.getIgnoreUsernamePassword();
        databaseM = IBPP::DatabaseFactory("", wx2std(getConnectionString()),
            (useUserNamePwd ? wx2std(getUsername()) : ""),
            (useUserNamePwd ? wx2std(password) : ""),
            wx2std(getRole()), wx2std(getConnectionCharset()), "");

        databaseM->Connect();
        connectedM = true;

        // first start a transaction for metadata loading, then lock the
        // database
        // when objects go out of scope and are destroyed, database will be
        // unlocked before the transaction is committed - any update() calls
        // on observers can possibly use the same transaction
        MetadataLoader* loader = getMetadataLoader();
        MetadataLoaderTransaction tr(loader);
        SubjectLocker lock(this);

        try
        {
            checkProgressIndicatorCanceled(indicator);
            // load database charset
            IBPP::Statement& st1 = loader->getStatement(
                "select rdb$character_set_name from rdb$database");
            st1->Execute();
            if (st1->Fetch())
            {
                std::string s;
                st1->Get(1, s);
                databaseCharsetM = std2wxIdentifier(s, getCharsetConverter());
            }
            checkProgressIndicatorCanceled(indicator);
            // load database information
            setPropertiesLoaded(false);
            databaseInfoM.load(databaseM);
            setPropertiesLoaded(true);

            // load collections of metadata objects
            setChildrenLoaded(false);
            loadCollections(indicator);
            setChildrenLoaded(true);
            if (indicator)
                indicator->initProgress(_("Complete"), 100, 100);
        }
        catch (CancelProgressException&)
        {
            disconnect();
        }
        notifyObservers();
    }
    catch (...)
    {
        try
        {
            disconnect();
            databaseM.clear();
        }
        catch (...) // we don't care as we already have an error to report
        {
        }
        throw;
    }
}
//-----------------------------------------------------------------------------
void Database::loadCollections(ProgressIndicator* progressIndicator)
{
    // use a small helper to cut down on the repetition...
    struct ProgressIndicatorHelper
    {
    private:
        ProgressIndicator* progressIndicatorM;
    public:
        ProgressIndicatorHelper(ProgressIndicator* progressIndicator)
            : progressIndicatorM(progressIndicator) {};
        void init(QString collectionName, int stepsTotal, int currentStep)
        {
            if (progressIndicatorM)
            {
                QString msg(QObject::tr("Loading %1...")
							.arg(collectionName));
                progressIndicatorM->initProgress(msg, stepsTotal,
                    currentStep, 1);
            }
        }
    };

    const int collectionCount = 11;
    std::string loadStmt;
    ProgressIndicatorHelper pih(progressIndicator);

    MetadataLoader* loader = getMetadataLoader();
    MetadataLoaderTransaction tr(loader);
    SubjectLocker lock(this);

    pih.init(_("tables"), collectionCount, 0);
    tablesM->load(progressIndicator);

    pih.init(_("system tables"), collectionCount, 1);
    sysTablesM->load(progressIndicator);

    pih.init(_("views"), collectionCount, 2);
    viewsM->load(progressIndicator);

    pih.init(_("procedures"), collectionCount, 3);
    proceduresM->load(progressIndicator);

    pih.init(_("triggers"), collectionCount, 4);
    triggersM->load(progressIndicator);

    pih.init(_("roles"), collectionCount, 5);
    rolesM->load(progressIndicator);

    pih.init(_("system roles"), collectionCount, 6);
    sysRolesM->load(progressIndicator);

    pih.init(_("domains"), collectionCount, 7);
    userDomainsM->load(progressIndicator);

    pih.init(_("functions"), collectionCount, 8);
    functionsM->load(progressIndicator);

    pih.init(_("generators"), collectionCount, 9);
    generatorsM->load(progressIndicator);

    pih.init(_("exceptions"), collectionCount, 10);
    exceptionsM->load(progressIndicator);
}
//-----------------------------------------------------------------------------
QStringList Database::loadIdentifiers(const QString& loadStatement,
    ProgressIndicator* progressIndicator)
{
    MetadataLoader* loader = getMetadataLoader();
    MetadataLoaderTransaction tr(loader);
    QTextCodec* converter = getCharsetConverter();

    IBPP::Statement& st1 = loader->getStatement(
        wx2std(loadStatement, getCharsetConverter()));
    st1->Execute();

    QStringList names;
    while (st1->Fetch())
    {
        checkProgressIndicatorCanceled(progressIndicator);
        if (!st1->IsNull(1))
        {
            std::string s;
            st1->Get(1, s);
            names.push_back(std2wxIdentifier(s, converter));
        }
    }
    return names;
}
//-----------------------------------------------------------------------------
void Database::disconnect()
{
    if (connectedM)
    {
        databaseM->Disconnect();
        setDisconnected();
    }
}
//-----------------------------------------------------------------------------
void Database::setDisconnected()
{
    delete metadataLoaderM;
    metadataLoaderM = 0;
    resetCredentials();     // "forget" temporary username/password
    connectedM = false;
    resetPendingLoadData();

    // remove entire DBH beneath
    userDomainsM.reset();
    sysDomainsM.reset();
    functionsM.reset();
    generatorsM.reset();
    proceduresM.reset();
    rolesM.reset();
    tablesM.reset();
    sysTablesM.reset();
    triggersM.reset();
    viewsM.reset();
    exceptionsM.reset();

    if (config().get(QString::fromLatin1("HideDisconnectedDatabases"), false))
        getServer()->notifyObservers();
    notifyObservers();
}
//-----------------------------------------------------------------------------
bool Database::isConnected() const
{
    return connectedM;
}
//-----------------------------------------------------------------------------
MetadataLoader* Database::getMetadataLoader()
{
    if (metadataLoaderM == 0)
        metadataLoaderM = new MetadataLoader(*this, 8);
    return metadataLoaderM;
}
//-----------------------------------------------------------------------------
bool Database::getChildren(std::vector<MetadataItem*>& temp)
{
    if (!connectedM)
        return false;

    getCollections(temp, true);
    return true;
}
//-----------------------------------------------------------------------------
DomainsPtr Database::getDomains()
{
    Q_ASSERT(userDomainsM);
    userDomainsM->ensureChildrenLoaded();
    return userDomainsM;
}
//-----------------------------------------------------------------------------
SysDomainsPtr Database::getSysDomains()
{
    Q_ASSERT(sysDomainsM);
    sysDomainsM->ensureChildrenLoaded();
    return sysDomainsM;
}
//-----------------------------------------------------------------------------
ExceptionsPtr Database::getExceptions()
{
    Q_ASSERT(exceptionsM);
    exceptionsM->ensureChildrenLoaded();
    return exceptionsM;
}
//-----------------------------------------------------------------------------
FunctionsPtr Database::getFunctions()
{
    Q_ASSERT(functionsM);
    functionsM->ensureChildrenLoaded();
    return functionsM;
}
//-----------------------------------------------------------------------------
GeneratorsPtr Database::getGenerators()
{
    Q_ASSERT(generatorsM);
    generatorsM->ensureChildrenLoaded();
    return generatorsM;
}
//-----------------------------------------------------------------------------
ProceduresPtr Database::getProcedures()
{
    Q_ASSERT(proceduresM);
    proceduresM->ensureChildrenLoaded();
    return proceduresM;
}
//-----------------------------------------------------------------------------
RolesPtr Database::getRoles()
{
    Q_ASSERT(rolesM);
    rolesM->ensureChildrenLoaded();
    return rolesM;
}
//-----------------------------------------------------------------------------
SysRolesPtr Database::getSysRoles()
{
    Q_ASSERT(sysRolesM);
    sysRolesM->ensureChildrenLoaded();
    return sysRolesM;
}
//-----------------------------------------------------------------------------
SysTablesPtr Database::getSysTables()
{
    Q_ASSERT(sysTablesM);
    sysTablesM->ensureChildrenLoaded();
    return sysTablesM;
}
//-----------------------------------------------------------------------------
TablesPtr Database::getTables()
{
    Q_ASSERT(tablesM);
    tablesM->ensureChildrenLoaded();
    return tablesM;
}
//-----------------------------------------------------------------------------
TriggersPtr Database::getTriggers()
{
    Q_ASSERT(triggersM);
    triggersM->ensureChildrenLoaded();
    return triggersM;
}
//-----------------------------------------------------------------------------
ViewsPtr Database::getViews()
{
    Q_ASSERT(viewsM);
    viewsM->ensureChildrenLoaded();
    return viewsM;
}
//-----------------------------------------------------------------------------
// returns vector of all subitems
void Database::getCollections(std::vector<MetadataItem*>& temp, bool system)
{
    if (!isConnected())
        return;

    ensureChildrenLoaded();

    temp.push_back(userDomainsM.get());
    temp.push_back(exceptionsM.get());
    temp.push_back(functionsM.get());
    temp.push_back(generatorsM.get());
    temp.push_back(proceduresM.get());
    temp.push_back(rolesM.get());
    // Only push back system objects when they should be shown
    if (system && showSystemRoles())
        temp.push_back(sysRolesM.get());
    if (system && showSystemTables())
        temp.push_back(sysTablesM.get());
    temp.push_back(tablesM.get());
    temp.push_back(triggersM.get());
    temp.push_back(viewsM.get());
}
//-----------------------------------------------------------------------------
void Database::loadChildren()
{
    // TODO: show progress dialog while reloading child object collections?
    loadCollections(0);
}
//-----------------------------------------------------------------------------
void Database::lockChildren()
{
    if (isConnected())
    {
        userDomainsM->lockSubject();
        sysDomainsM->lockSubject();
        exceptionsM->lockSubject();
        functionsM->lockSubject();
        generatorsM->lockSubject();
        proceduresM->lockSubject();
        rolesM->lockSubject();
        tablesM->lockSubject();
        sysTablesM->lockSubject();
        triggersM->lockSubject();
        viewsM->lockSubject();
    }
}
//-----------------------------------------------------------------------------
void Database::unlockChildren()
{
    // unlock in reverse order of locking - that way domains will still
    // be locked when relation and procedure updates happen - that way not
    // every added domain will cause all collection observers to update
    if (isConnected())
    {
        viewsM->unlockSubject();
        triggersM->unlockSubject();
        sysTablesM->unlockSubject();
        tablesM->unlockSubject();
        rolesM->unlockSubject();
        proceduresM->unlockSubject();
        generatorsM->unlockSubject();
        functionsM->unlockSubject();
        exceptionsM->unlockSubject();
        sysDomainsM->unlockSubject();
        userDomainsM->unlockSubject();
    }
}
//-----------------------------------------------------------------------------
QString Database::getPath() const
{
    return pathM;
}
//-----------------------------------------------------------------------------
QString Database::getDatabaseCharset() const
{
    return databaseCharsetM;
}
//-----------------------------------------------------------------------------
QString Database::getConnectionCharset() const
{
    if (connectionCredentialsM)
        return connectionCredentialsM->getCharset();
    else
        return credentialsM.getCharset();
}
//-----------------------------------------------------------------------------
QString Database::getConnectionInfoString() const
{
    QString info;
    if (authenticationModeM.getIgnoreUsernamePassword())
        info = _("[Trusted user]");
    else
        info = getUsername();
    info += QString::fromLatin1("@") + getConnectionString() + QString::fromLatin1(" (")
        + getConnectionCharset() + QString::fromLatin1(")");
    return info;
}
//-----------------------------------------------------------------------------
bool Database::usesDifferentConnectionCharset() const
{
    QString charset(getConnectionCharset().toUpper());
    if (databaseCharsetM.isEmpty() && charset == QString::fromLatin1("NONE"))
        return false;
    return (charset.compare(databaseCharsetM.toUpper()) != 0);
}
//-----------------------------------------------------------------------------
QString Database::getUsername() const
{
    if (connectionCredentialsM)
        return connectionCredentialsM->getUsername();
    else
        return credentialsM.getUsername();
}
//-----------------------------------------------------------------------------
QString Database::getRawPassword() const
{
    if (connectionCredentialsM)
        return connectionCredentialsM->getPassword();
    else
        return credentialsM.getPassword();
}
//-----------------------------------------------------------------------------
QString Database::getDecryptedPassword() const
{
    // if we already have an established connection return that password
    if (databaseM != 0 && databaseM->Connected())
        return std2wx(databaseM->UserPassword());

    // temporary connection
    if (connectionCredentialsM)
        return connectionCredentialsM->getPassword();
    QString raw = credentialsM.getPassword();
    if (raw.isEmpty())
        return QString();

    if (authenticationModeM.getUseEncryptedPassword()) {
        // return decryptPassword(raw, getUsername() + getConnectionString());
		Q_ASSERT(false);
		return raw;
	} else {
        return raw;
	}
}
//-----------------------------------------------------------------------------
DatabaseAuthenticationMode& Database::getAuthenticationMode()
{
    return authenticationModeM;
}
//-----------------------------------------------------------------------------
QString Database::getRole() const
{
    if (connectionCredentialsM)
        return connectionCredentialsM->getRole();
    else
        return credentialsM.getRole();
}
//-----------------------------------------------------------------------------
IBPP::Database& Database::getIBPPDatabase()
{
    return databaseM;
}
//-----------------------------------------------------------------------------
void Database::setPath(const QString& value)
{
    pathM = value;
}
//-----------------------------------------------------------------------------
void Database::setConnectionCharset(const QString& value)
{
    if (connectionCredentialsM)
        connectionCredentialsM->setCharset(value);
    else
        credentialsM.setCharset(value);
}
//-----------------------------------------------------------------------------
void Database::setUsername(const QString& value)
{
    if (connectionCredentialsM)
        connectionCredentialsM->setUsername(value);
    else
        credentialsM.setUsername(value);
}
//-----------------------------------------------------------------------------
void Database::setRawPassword(const QString& value)
{
    if (connectionCredentialsM)
        connectionCredentialsM->setPassword(value);
    else
        credentialsM.setPassword(value);
}
//-----------------------------------------------------------------------------
void Database::setEncryptedPassword(const QString& value)
{
    // temporary credentials -> use password as entered
    if (connectionCredentialsM)
    {
        connectionCredentialsM->setPassword(value);
        return;
    }

    // password must not be isEmpty to be encrypted

	// TODO : support encryption
    // if (authenticationModeM.getUseEncryptedPassword() && !value.isEmpty())
    // {
    //     credentialsM.setPassword(
    //         encryptPassword(value, getUsername() + getConnectionString()));
    // }
    // else
    {
        credentialsM.setPassword(value);
    }
}
//-----------------------------------------------------------------------------
void Database::setRole(const QString& value)
{
    if (connectionCredentialsM)
        connectionCredentialsM->setRole(value);
    else
        credentialsM.setRole(value);
}
//-----------------------------------------------------------------------------
const QString Database::getTypeName() const
{
    return QString::fromLatin1("DATABASE");
}
//-----------------------------------------------------------------------------
void Database::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitDatabase(*this);
}
//-----------------------------------------------------------------------------
ServerPtr Database::getServer() const
{
    return ServerPtr(serverM);
}
//-----------------------------------------------------------------------------
void Database::setServer(ServerPtr server)
{
    Q_ASSERT(server);
    serverM = server;
    setParent(server.get());
}
//-----------------------------------------------------------------------------
QString Database::getConnectionString() const
{
    QString serverConnStr = getServer()->getConnectionString();
    if (!serverConnStr.isEmpty())
        return serverConnStr + QString::fromLatin1(":") + pathM;
    else
        return pathM;
}
//-----------------------------------------------------------------------------
/* static */
QString Database::extractNameFromConnectionString(const QString& path)
{
    QString name = path;
    int p = name.lastIndexOf(QString::fromLatin1("/\\:"));
    if (p != -1) {
        // name.erase(0, p + 1);
		name.remove(0, p + 1);
	}
    p = name.lastIndexOf(QString::fromLatin1("."));
    if (p != -1) {
        // name.erase(p, name.length());
		name = name.left(p);
	}
    return name;
}
//-----------------------------------------------------------------------------
const QString Database::getId() const
{
    if (idM == 0)
        idM = getUniqueId();
    return QString::number(idM);
}
//-----------------------------------------------------------------------------
unsigned uniqueDatabaseId = 1;
//-----------------------------------------------------------------------------
void Database::setId(unsigned id)
{
    idM = id;
    // generator to be higher than ids of existing databases
    if (id >= uniqueDatabaseId)
        uniqueDatabaseId = id + 1;
}
//-----------------------------------------------------------------------------
/*static*/
unsigned Database::getUniqueId()
{
    return uniqueDatabaseId++;
}
//-----------------------------------------------------------------------------
/*static*/
unsigned Database::getUIDGeneratorValue()
{
    return uniqueDatabaseId;
}
//-----------------------------------------------------------------------------
/*static*/
void Database::setUIDGeneratorValue(unsigned value)
{
    uniqueDatabaseId = value;
}
//-----------------------------------------------------------------------------
const DatabaseInfo& Database::getInfo()
{
    databaseInfoM.reloadIfNecessary(databaseM);
    return databaseInfoM;
}
//-----------------------------------------------------------------------------
void Database::loadInfo()
{
    databaseInfoM.load(databaseM);
    notifyObservers();
}
//-----------------------------------------------------------------------------
bool Database::showSystemRoles()
{
    if (!getInfo().getODSVersionIsHigherOrEqualTo(11, 1))
        return false;

    const QString SHOW_SYSROLES = QString::fromLatin1("ShowSystemRoles");

    bool b;
    if (!DatabaseConfig(this, config()).getValue(SHOW_SYSROLES, b))
        b = config().get(SHOW_SYSROLES, false);

    return b;
}
//-----------------------------------------------------------------------------
bool Database::showSystemTables()
{
    const QString SHOW_SYSTABLES = QString::fromLatin1("ShowSystemTables");

    bool b;
    if (!DatabaseConfig(this, config()).getValue(SHOW_SYSTABLES, b))
        b = config().get(SHOW_SYSTABLES, true);

    return b;
}
//-----------------------------------------------------------------------------
QString mapConnectionCharsetToSystemCharset(const QString& connectionCharset)
{
    QString charset(helper::stripString(connectionCharset, helper::both).toUpper());

    // fixes hang when character set name isEmpty (invalid encoding is returned)
    if (charset.isEmpty())
        charset = QString::fromLatin1("NONE");

    // Firebird charsets WIN125X need to be replaced with either
    // WINDOWS125X or CP125X - we take the latter
    if (charset.mid(0, 5) == QString::fromLatin1("WIN12"))
        return QString::fromLatin1("CP12") + charset.mid(5);

    // Firebird charsets ISO8859_X
    if (charset.mid(0, 8) == QString::fromLatin1("ISO8859_"))
        return QString::fromLatin1("ISO 8859-") + charset.mid(8);

    // all other mappings need to be added here...
    struct CharsetMapping { const char* connCS; const char* convCS; };
    static const CharsetMapping mappings[] = {
        {
			"UTF8",
			"UTF-8"
		},
		{
			"UNICODE_FSS",
			"UTF-8"
		}
    };
    int mappingCount = sizeof(mappings) / sizeof(CharsetMapping);
    for (int i = 0; i < mappingCount; i++)
    {
        if (0==charset.compare(QLatin1String(mappings[i].connCS))) {
            return mappings[i].convCS;
		}
    }

    return charset;
}
//-----------------------------------------------------------------------------
void Database::createCharsetConverter()
{
    // charsetConverterM.reset();

    QString cs(mapConnectionCharsetToSystemCharset(getConnectionCharset()));
    // wxFontEncoding fe = wxFontMapperBase::Get()->CharsetToEncoding(cs, false);
    // if (fe != wxFONTENCODING_SYSTEM)
    //     charsetConverterM.reset(new wxCSConv(fe));
	charsetConverterM = QTextCodec::codecForName(cs.toLatin1());
}
//-----------------------------------------------------------------------------
QTextCodec* Database::getCharsetConverter() const
{
    // if (QTextCodec* conv = charsetConverterM.get())
    //     return conv;
    // return wxConvCurrent;
	if (charsetConverterM) {
		return charsetConverterM;
	} else {
		return QTextCodec::codecForCStrings();
	}
}
//-----------------------------------------------------------------------------
void Database::getConnectedUsers(QStringList& users) const
{
    if (databaseM != 0 && databaseM->Connected())
    {
        std::vector<std::string> userNames;
        databaseM->Users(userNames);

        // replace multiple occurences of same user name by "username (N)"
        std::map<std::string, size_t> counts;
        for (std::vector<std::string>::const_iterator it = userNames.begin();
            it != userNames.end(); ++it)
        {
            counts[*it] += 1;
        }

        for (std::map<std::string, size_t>::iterator it = counts.begin();
            it != counts.end(); ++it)
        {
            QString name(std2wx((*it).first));
            if ((*it).second > 1)
                name += QString::fromLatin1(" (%1)").arg((*it).second);
            users.push_back(name);
        }
    }
}
//-----------------------------------------------------------------------------
void Database::checkConnected(const QString& operation) const
{
    if (!connectedM)
    {
        throw FRError(QObject::tr("Operation \"%1\" not allowed on a disconnected database.")
					  .arg(operation));
    }
}
//-----------------------------------------------------------------------------

}

