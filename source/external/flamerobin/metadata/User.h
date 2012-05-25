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


  $Id: User.h 2200 2012-01-20 08:31:01Z mghie $

*/

//-----------------------------------------------------------------------------
#ifndef FR_USER_H
#define FR_USER_H

#include "ibpp/ibpp.h"

#include "metadata/MetadataClasses.h"
#include "metadata/metadataitem.h"

namespace fr {

class User: public MetadataItem,
    public boost::enable_shared_from_this<User>
{
private:
    ServerWeakPtr serverM;
    QString usernameM;
    QString passwordM;
    QString firstnameM;
    QString middlenameM;
    QString lastnameM;
    uint32_t useridM;
    uint32_t groupidM;
public:
    User(ServerPtr server);
    User(ServerPtr server, const IBPP::User& src);

    ServerPtr getServer() const;
    virtual bool isSystem() const;

    QString getUsername() const;
    QString getPassword() const;
    QString getFirstName() const;
    QString getMiddleName() const;
    QString getLastName() const;
    uint32_t getUserId() const;
    uint32_t getGroupId() const;

    void setUsername(const QString& value);
    void setPassword(const QString& value);
    void setFirstName(const QString& value);
    void setMiddleName(const QString& value);
    void setLastName(const QString& value);
    void setUserId(uint32_t value);
    void setGroupId(uint32_t value);

    void assignTo(IBPP::User& dest) const;
};
//-----------------------------------------------------------------------------
} // namespace fr

#endif
