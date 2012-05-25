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


  $Id: database.h 2200 2012-01-20 08:31:01Z mghie $

*/

#ifndef FR_DATABASE_H
#define FR_DATABASE_H
//-----------------------------------------------------------------------------
#include <map>

#include <boost/enable_shared_from_this.hpp>
#include "flamerobin/ibpp/ibpp.h"
#include "metadata/MetadataClasses.h"
#include "metadata/metadataitem.h"

#include <QString>
#include <QStringList>
#include <QElapsedTimer>

namespace fr {

//-----------------------------------------------------------------------------
class MetadataLoader;
class ProgressIndicator;
class SqlStatement;
//-----------------------------------------------------------------------------
class CharacterSet
{
private:
    QString nameM;
    int idM;
    int bytesPerCharM;
public:
    CharacterSet(const QString& name, int id = -1, int bytesPerChar = -1);

    bool operator< (const CharacterSet& other) const;
    int getBytesPerChar() const;
    int getId() const;
    QString getName() const;
};
//-----------------------------------------------------------------------------
class Credentials
{
private:
    QString charsetM;
    QString roleM;
    QString usernameM;
    QString passwordM;

public:
    QString getCharset() const;
    QString getUsername() const;
    QString getPassword() const;
    QString getRole() const;
    void setCharset(const QString& value);
    void setUsername(const QString& value);
    void setPassword(const QString& value);
    void setRole(const QString& value);
};
//-----------------------------------------------------------------------------
class DatabaseInfo
{
    friend class Database;
private:
    int odsM;
    int odsMinorM;
    int dialectM;

    int pageSizeM;
    int buffersM;
    int pagesM;

    int oldestTransactionM;
    int oldestActiveTransactionM;
    int oldestSnapshotM;
    int nextTransactionM;

    int sweepM;

    bool readOnlyM;
    bool forcedWritesM;
    bool reserveM;

	mutable QElapsedTimer loadTimer;
    void load(const IBPP::Database& database);
    void reloadIfNecessary(const IBPP::Database& database);
public:
    int getODS() const;
    int getODSMinor() const;
    bool getODSVersionIsHigherOrEqualTo(int versionMajor) const;
    bool getODSVersionIsHigherOrEqualTo(int versionMajor, int versionMinor) const;

    int getDialect() const;

    int getPageSize() const;
    int getBuffers() const;
    int getPages() const;
    int64_t getSizeInBytes() const;

    int getOldestTransaction() const;
    int getOldestActiveTransaction() const;
    int getOldestSnapshot() const;
    int getNextTransaction() const;

    int getSweep() const;

    bool getReadOnly() const;
    bool getForcedWrites() const;
    bool getReserve() const;
};
//-----------------------------------------------------------------------------
class DatabaseAuthenticationMode
{
public:
    DatabaseAuthenticationMode();

    enum Mode { UseSavedPassword, UseSavedEncryptedPwd, AlwaysEnterPassword,
        TrustedUser };
    int getMode() const;
    void setMode(int mode);

    QString getConfigValue() const;
    void setConfigValue(const QString& value);
    // support for old "encrypted password" setting
    void setStoreEncryptedPassword();

    bool getAlwaysAskForPassword() const;
    bool getIgnoreUsernamePassword() const;
    bool getUseEncryptedPassword() const;
private:
    Mode modeM;
};

//-----------------------------------------------------------------------------
class Database: public MetadataItem,
    public boost::enable_shared_from_this<Database>
{
private:
    ServerWeakPtr serverM;
	IBPP::Database databaseM;
    MetadataLoader* metadataLoaderM;

    bool connectedM;
    QString databaseCharsetM;

    QString pathM;
    Credentials credentialsM;
    Credentials* connectionCredentialsM;
    DatabaseAuthenticationMode authenticationModeM;

    QTextCodec* charsetConverterM;
    void createCharsetConverter();

    DatabaseInfo databaseInfoM;

    DomainsPtr userDomainsM;
    SysDomainsPtr sysDomainsM;
    ExceptionsPtr exceptionsM;
    FunctionsPtr functionsM;
    GeneratorsPtr generatorsM;
    ProceduresPtr proceduresM;
    RolesPtr rolesM;
    SysRolesPtr sysRolesM;
    TablesPtr tablesM;
    SysTablesPtr sysTablesM;
    TriggersPtr triggersM;
    ViewsPtr viewsM;

    // copy constructor implementation removed since it's no longer needed
    // (Server uses a vector of boost::shared_ptr<Database> now)
    Database(const Database& rhs);

    void setDisconnected();

    std::multimap<CharacterSet, QString> collationsM;
    void loadCollations();

    void loadCollections(ProgressIndicator* progressIndicator);

    // small help for parser
    QString getTableForIndex(const QString& indexName);

    mutable unsigned idM;

    bool showSystemRoles();
    bool showSystemTables();

    inline void checkConnected(const QString& operation) const;
protected:
    virtual void loadChildren();
    virtual void lockChildren();
    virtual void unlockChildren();

public:
    Database();
    ~Database();

    virtual bool getChildren(std::vector<MetadataItem *>& temp);
    void getCollections(std::vector<MetadataItem *>& temp, bool system);

    DomainsPtr getDomains();
    SysDomainsPtr getSysDomains();
    ExceptionsPtr getExceptions();
    FunctionsPtr getFunctions();
    GeneratorsPtr getGenerators();
    ProceduresPtr getProcedures();
    RolesPtr getRoles();
    SysRolesPtr getSysRoles();
    TablesPtr getTables();
    SysTablesPtr getSysTables();
    TriggersPtr getTriggers();
    ViewsPtr getViews();

    bool isConnected() const;
    void connect(const QString& password, ProgressIndicator* indicator = 0);
    void disconnect();
    void reconnect();
    void prepareTemporaryCredentials();
    void resetCredentials();
    void drop();

    MetadataLoader* getMetadataLoader();

    QStringList loadIdentifiers(const QString& loadStatement,
        ProgressIndicator* progressIndicator = 0);

    QString loadDomainNameForColumn(const QString& table,
        const QString& field);
    DomainPtr getDomain(const QString& name);

    void loadGeneratorValues();
    Relation* getRelationForTrigger(Trigger* trigger);

    virtual DatabasePtr getDatabase() const;
    MetadataItem* findByNameAndType(NodeType nt, const QString& name);
    MetadataItem* findByName(const QString& name);
    Relation* findRelation(const Identifier& name);
    void dropObject(MetadataItem *object);
    void addObject(NodeType type, const QString& name);
    void parseCommitedSql(const SqlStatement& stm);     // reads a DDL statement and does accordingly

    CharacterSet getCharsetById(int id);
    QStringList getCollations(const QString& charset);
    bool isDefaultCollation(const QString& charset, const QString& collate);

    //! fill vector with names of all tables, views, etc.
    void getIdentifiers(std::vector<Identifier>& temp);

    //! gets the database triggers (FB2.1+)
    void getDatabaseTriggers(std::vector<Trigger *>& list);

    QString getPath() const;
    QString getDatabaseCharset() const;
    QString getConnectionCharset() const;
    QString getConnectionInfoString() const;
    bool usesDifferentConnectionCharset() const;
    QString getUsername() const;
    QString getRawPassword() const;
    QString getDecryptedPassword() const;
    DatabaseAuthenticationMode& getAuthenticationMode();
    QString getRole() const;
    IBPP::Database& getIBPPDatabase();
    void setPath(const QString& value);
    void setConnectionCharset(const QString& value);
    void setUsername(const QString& value);
    void setRawPassword(const QString& value);
    void setEncryptedPassword(const QString& value);
    void setRole(const QString& value);
    virtual const QString getTypeName() const;
    ServerPtr getServer() const;
    void setServer(ServerPtr server);
    // returns the complete connection QString.
    QString getConnectionString() const;
    // returns a candidate name based on the connection QString. Example:
    // path is "C:\data\database.fdb" -> returns "database".
    static QString extractNameFromConnectionString(const QString& path);
    virtual const QString getId() const;
    void setId(unsigned id);

    // returns the value of the Id generator and increments it afterwards.
    static unsigned getUniqueId();
    // returns the current value of the Id generator.
    static unsigned getUIDGeneratorValue();
    // sets the current value of the Id generator.
    static void setUIDGeneratorValue(unsigned value);

    virtual void acceptVisitor(MetadataItemVisitor* visitor);

    const DatabaseInfo& getInfo();
    void loadInfo();

    void getConnectedUsers(QStringList& users) const;

    // wxMBConv* getCharsetConverter() const;
	QTextCodec* getCharsetConverter() const;
};
//----------------------------------------------------------------------------

} // namespace fr

#endif
