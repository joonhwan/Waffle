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


  $Id: server.h 2200 2012-01-20 08:31:01Z mghie $

*/

//-----------------------------------------------------------------------------
#ifndef FR_SERVER_H
#define FR_SERVER_H

#include <boost/enable_shared_from_this.hpp>

#include "ibpp/ibpp.h"

#include "metadata/collection.h"
#include "metadata/database.h"
#include "metadata/MetadataClasses.h"
#include "metadata/metadataitem.h"
#include "metadata/User.h"

namespace fr {

//-----------------------------------------------------------------------------
class Server : public MetadataItem,
    public boost::enable_shared_from_this<Server>
{
private:
    QString hostnameM;
    QString portM;

    DatabasePtrs databasesM;
    UserPtrs usersM;

    QString serviceUserM;
    QString servicePasswordM;
    QString serviceSysdbaPasswordM;
protected:
    virtual void lockChildren();
    virtual void unlockChildren();
public:
    Server();

    DatabasePtrs getDatabases() const;
    virtual bool getChildren(std::vector<MetadataItem *>& temp);

    DatabasePtr addDatabase();
    void addDatabase(DatabasePtr database);
    void removeDatabase(DatabasePtr database);

    void createDatabase(DatabasePtr db, int pagesize, int dialect);

    // returns *connected* service
    bool getService(IBPP::Service& svc, ProgressIndicator* progressind,
        bool sysdba);
    void setServiceCredentials(const QString& user, const QString& pass);
    void setServiceSysdbaPassword(const QString& pass);

    UserPtrs getUsers(ProgressIndicator* progressind);

    // setters/getters
    QString getHostname() const;
    QString getPort() const;
    // returns the server-related portion of the connection QString,
    // that is server name and port number if specified.
    QString getConnectionString() const;
    static QString makeConnectionString(const QString& hostname,
        const QString& port);

    void setHostname(QString hostname);
    void setPort(QString port);
    virtual const QString getTypeName() const;

    bool hasConnectedDatabase() const;
    virtual const QString getItemPath() const;
    virtual void acceptVisitor(MetadataItemVisitor* visitor);
};
//-----------------------------------------------------------------------------

} // namespace fr

#endif
