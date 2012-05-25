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


  $Id: server.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
#include <algorithm>

#include <boost/function.hpp>

#include "config/Config.h"
#include "core/ProgressIndicator.h"
#include "core/StringUtils.h"
#include "frutils.h"
#include "metadata/MetadataItemVisitor.h"
#include "metadata/root.h"
#include "metadata/server.h"

namespace fr {

//-----------------------------------------------------------------------------
Server::Server()
    : MetadataItem(ntServer)
{
    setChildrenLoaded(true);
}
//-----------------------------------------------------------------------------
DatabasePtrs Server::getDatabases() const
{
    return databasesM;
}
//-----------------------------------------------------------------------------
void Server::lockChildren()
{
    std::for_each(databasesM.begin(), databasesM.end(),
        boost::mem_fn(&Database::lockSubject));
}
//-----------------------------------------------------------------------------
void Server::unlockChildren()
{
    std::for_each(databasesM.begin(), databasesM.end(),
        boost::mem_fn(&Database::unlockSubject));
}
//-----------------------------------------------------------------------------
bool Server::getChildren(std::vector<MetadataItem*>& temp)
{
    if (databasesM.empty())
        return false;
    std::transform(databasesM.begin(), databasesM.end(),
        std::back_inserter(temp), boost::mem_fn(&DatabasePtr::get));
    return !databasesM.empty();
}
//-----------------------------------------------------------------------------
DatabasePtr Server::addDatabase()
{
    DatabasePtr database(new Database());
    addDatabase(database);
    return database;
}
//-----------------------------------------------------------------------------
void Server::addDatabase(DatabasePtr database)
{
    if (database)
    {
        databasesM.push_back(database);
        database->setServer(shared_from_this());
        notifyObservers();
    }
}
//-----------------------------------------------------------------------------
void Server::removeDatabase(DatabasePtr database)
{
    DatabasePtrs::iterator it = std::remove(databasesM.begin(),
        databasesM.end(), database);
    if (it != databasesM.end())
    {
        databasesM.erase(it, databasesM.end());
        notifyObservers();
    }
}
//-----------------------------------------------------------------------------
void Server::createDatabase(DatabasePtr db, int pagesize, int dialect)
{
    QString extra_params;
    if (pagesize)
        extra_params += (QString::fromLatin1(" PAGE_SIZE ") + pagesize);

    QString charset(db->getConnectionCharset());
    if (!charset.isEmpty())
        extra_params += (QString::fromLatin1(" DEFAULT CHARACTER SET ") + charset);

    IBPP::Database db1;
    db1 = IBPP::DatabaseFactory(wx2std(getConnectionString()),
        wx2std(db->getPath()), wx2std(db->getUsername()),
        wx2std(db->getDecryptedPassword()), "", wx2std(charset),
        wx2std(extra_params));
    db1->Create(dialect);
}
//-----------------------------------------------------------------------------
QString Server::getHostname() const
{
    return hostnameM;
}
//-----------------------------------------------------------------------------
QString Server::getPort() const
{
    return portM;
}
//-----------------------------------------------------------------------------
bool Server::hasConnectedDatabase() const
{
    DatabasePtrs::const_iterator it = std::find_if(
        databasesM.begin(), databasesM.end(),
        boost::mem_fn(&Database::isConnected));
    return it != databasesM.end();
}
//-----------------------------------------------------------------------------
void Server::setHostname(QString hostname)
{
    hostnameM = hostname;
}
//-----------------------------------------------------------------------------
void Server::setPort(QString port)
{
    portM = port;
}
//-----------------------------------------------------------------------------
const QString Server::getTypeName() const
{
    return QString::fromLatin1("SERVER");
}
//-----------------------------------------------------------------------------
void Server::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitServer(*this);
}
//-----------------------------------------------------------------------------
/* static */
QString Server::makeConnectionString(const QString& hostname,
    const QString& port)
{
    if (!hostname.isEmpty() && !port.isEmpty())
        return hostname + QString::fromLatin1("/") + port;
    else
        return hostname;
}
//-----------------------------------------------------------------------------
QString Server::getConnectionString() const
{
    return makeConnectionString(getHostname(), getPort());
}
//-----------------------------------------------------------------------------
const QString Server::getItemPath() const
{
    // Since database Ids are already unique, let's shorten the item paths
    // by not including the server part. Even more so if this class is bound
    // to disappear in the future.
    return QString::fromLatin1("");
}
//-----------------------------------------------------------------------------
struct SortUsers
{
    bool operator() (UserPtr user1, UserPtr user2)
    {
        return user1->getUsername() < user2->getUsername();
    }
};
UserPtrs Server::getUsers(ProgressIndicator* progressind)
{
    usersM.clear();
    IBPP::Service svc;
    if (!fr::getService(this, svc, progressind, true))   // true = SYSDBA
        return usersM;

    std::vector<IBPP::User> usr;
    svc->GetUsers(usr);
    for (std::vector<IBPP::User>::iterator it = usr.begin();
        it != usr.end(); ++it)
    {
        UserPtr u(new User(shared_from_this(), *it));
        usersM.push_back(u);
    }


    std::sort(usersM.begin(), usersM.end(), SortUsers());
    return usersM;
}
//-----------------------------------------------------------------------------
void Server::setServiceCredentials(const QString& user, const QString& pass)
{
    serviceUserM = user;
    servicePasswordM = pass;
}
//-----------------------------------------------------------------------------
void Server::setServiceSysdbaPassword(const QString& pass)
{
    serviceSysdbaPasswordM = pass;
}
//-----------------------------------------------------------------------------
bool Server::getService(IBPP::Service& svc, ProgressIndicator* progressind,
    bool sysdba)
{
    if (progressind)
    {
        progressind->initProgress(_("Connecting..."),
            databasesM.size() + 2, 0, 1);
    }

    // check if we already had some successful connections
    if (!serviceSysdbaPasswordM.isEmpty())  // we have sysdba pass
    {
        if (progressind)
        {
            progressind->setProgressMessage(_("Using current SYSDBA password"));
            progressind->stepProgress();
        }
        try
        {
            svc = IBPP::ServiceFactory(wx2std(getConnectionString()),
                "SYSDBA", wx2std(serviceSysdbaPasswordM));
            svc->Connect();
            return true;
        }
        catch(IBPP::Exception&)   // keep going if connect fails
        {
            serviceSysdbaPasswordM.clear();
        }
    }
    if (progressind && progressind->isCanceled())
        return false;
    // check if we have non-sysdba connection
    if (!sysdba && !serviceUserM.isEmpty())
    {
        if (progressind)
        {
            progressind->setProgressMessage(QString::fromLatin1("Using current %1 password")
											.arg(serviceUserM));
            progressind->stepProgress();
        }
        try
        {
            svc = IBPP::ServiceFactory(wx2std(getConnectionString()),
                wx2std(serviceUserM), wx2std(servicePasswordM));
            svc->Connect();
            return true;
        }
        catch(IBPP::Exception&)   // keep going if connect fails
        {
            serviceUserM.clear();
            servicePasswordM.clear();
        }
    }

    // first try connected databases
    for (DatabasePtrs::iterator ci = databasesM.begin();
        ci != databasesM.end(); ++ci)
    {
        if (progressind && progressind->isCanceled())
            return false;
        if (!(*ci)->isConnected())
            continue;
        // Use the user name and password of the connected user
        // instead of the stored ones.
        IBPP::Database& db = (*ci)->getIBPPDatabase();
        if (sysdba && std2wx(db->Username()).toUpper() != QString::fromLatin1("SYSDBA"))
            continue;
        if (progressind)
        {
            progressind->setProgressMessage(_("Using password of: ") +
                std2wx(db->Username()) + QString::fromLatin1("@") + (*ci)->getName_());
            progressind->stepProgress();
        }
        try
        {
            svc = IBPP::ServiceFactory(wx2std(getConnectionString()),
                db->Username(), db->UserPassword());
            svc->Connect();
            if (sysdba)
                serviceSysdbaPasswordM = std2wx(db->UserPassword());
            else
            {
                serviceUserM = std2wx(db->Username());
                servicePasswordM = std2wx(db->UserPassword());
            }
            return true;
        }
        catch(IBPP::Exception&)   // keep going if connect fails
        {
        }
    }

    // when the operation is not canceled try to user/pass of disconnected DBs
    for (DatabasePtrs::const_iterator ci = databasesM.begin();
        ci != databasesM.end(); ++ci)
    {
        if (progressind && progressind->isCanceled())
            return false;
        if ((*ci)->isConnected())
            continue;
        QString user = (*ci)->getUsername();
        QString pwd = (*ci)->getDecryptedPassword();
        if (pwd.isEmpty() || sysdba && user.toUpper() != QString::fromLatin1("SYSDBA"))
            continue;
        if (progressind)
        {
            progressind->setProgressMessage(_("Using password of: ") +
                user + QString::fromLatin1("@") + (*ci)->getName_());
            progressind->stepProgress();
        }
        try
        {
            svc = IBPP::ServiceFactory(wx2std(getConnectionString()),
                wx2std(user), wx2std(pwd));
            svc->Connect();
            if (sysdba)
                serviceSysdbaPasswordM = pwd;
            else
            {
                serviceUserM = user;
                servicePasswordM = pwd;
            }
            return true;
        }
        catch(IBPP::Exception&)   // keep going if connect fails
        {
        }
    }
    return false;
}
//-----------------------------------------------------------------------------

} // namespace fr
