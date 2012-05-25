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


  $Id: privilege.h 2200 2012-01-20 08:31:01Z mghie $

*/

#ifndef FR_PRIVILEGE_H
#define FR_PRIVILEGE_H

#include <map>
#include <vector>

#include "core/ProcessableObject.h"

namespace fr {

class MetadataItem;
class PrivilegeItem;
//-----------------------------------------------------------------------------
typedef std::vector<PrivilegeItem> PrivilegeItems;
//-----------------------------------------------------------------------------
class PrivilegeItem: public ProcessableObject
{
public:
    QString grantor;
    bool grantOption;
    std::vector<QString> columns;
    PrivilegeItem(const QString& grantorName, bool withGrantOption,
        const QString& fieldName);
};
//-----------------------------------------------------------------------------
class Privilege: public ProcessableObject
{
private:
    MetadataItem* parentObjectM;
    int granteeTypeM;
    QString granteeM;

    // type (SEL, INS, ...), privilege
    typedef std::multimap<QString, PrivilegeItem> PMap;
    PMap privilegeItemsM;

    QString getSql(bool withGrantOption) const;

public:
    Privilege(MetadataItem *parent, const QString& grantee, int granteeType);
    void addPrivilege(char privilege, const QString& grantor,
        bool withGrantOption, const QString& field = QString());

    QString getSql() const;
    QString getGrantee() const;
    void getPrivilegeItems(const QString& type, PrivilegeItems& list) const;
};
//-----------------------------------------------------------------------------

} // namespace fr

#endif
