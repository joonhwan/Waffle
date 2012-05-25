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


  $Id: privilege.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
#include "metadata/database.h"
#include "metadata/privilege.h"
#include "metadata/procedure.h"
#include "metadata/relation.h"
#include "metadata/role.h"

namespace fr {

//-----------------------------------------------------------------------------
PrivilegeItem::PrivilegeItem(const QString& grantorName,
    bool withGrantOption, const QString& fieldName)
    :grantor(grantorName), grantOption(withGrantOption)
{
    if (!fieldName.isEmpty())
        columns.push_back(fieldName);
}
//-----------------------------------------------------------------------------
Privilege::Privilege(MetadataItem *parent, const QString& grantee,
    int granteeType)
    : parentObjectM(parent), granteeM(grantee), granteeTypeM(granteeType),
     ProcessableObject()
{
}
//-----------------------------------------------------------------------------
void Privilege::addPrivilege(char privilege, const QString& grantor,
    bool withGrantOption, const QString& field)
{
    QString p;
    switch (privilege)
    {
        case 'S':   p = QString::fromLatin1("SELECT");      break;
        case 'I':   p = QString::fromLatin1("INSERT");      break;
        case 'U':   p = QString::fromLatin1("UPDATE");      break;
        case 'D':   p = QString::fromLatin1("DELETE");      break;
        case 'R':   p = QString::fromLatin1("REFERENCES");  break;
        case 'X':   p = QString::fromLatin1("EXECUTE");     break;
        case 'M':   p = QString::fromLatin1("MEMBER OF");   break;
        default:
            return;
    };

    // iterate all of this type
    PMap::iterator it;
    for (it = privilegeItemsM.lower_bound(p);
        it != privilegeItemsM.upper_bound(p); ++it)
    {
        if ((*it).second.grantor == grantor &&
            (*it).second.grantOption == withGrantOption)    // got it
        {
            std::vector<QString> *cols = 0;
            if (p == QString::fromLatin1("UPDATE") || p == QString::fromLatin1("REFERENCES"))
                cols = &((*it).second.columns);
            if (!field.isEmpty() && cols && cols->end() ==
                std::find(cols->begin(), cols->end(), field))
            {
                cols->push_back(field);
            }
            return;
        }
    }

    // not found, so add it
    PrivilegeItem pi(grantor, withGrantOption, field);
    privilegeItemsM.insert(std::pair<QString,PrivilegeItem>(p,pi));
}
//-----------------------------------------------------------------------------
QString granteeTypeToString(int type)
{
    if (type == 1)
        return QString::fromLatin1("VIEW");
    if (type == 2)
        return QString::fromLatin1("TRIGGER");
    if (type == 5)
        return QString::fromLatin1("PROCEDURE");
    if (type == 13)
        return QString::fromLatin1("ROLE");
    return QString();
}
//-----------------------------------------------------------------------------
QString Privilege::getSql(bool withGrantOption) const
{
    QString ret;
    for (PMap::const_iterator c = privilegeItemsM.begin();
        c != privilegeItemsM.end(); ++c)
    {
        if ((*c).second.grantOption != withGrantOption)
            continue;
        if (!ret.isEmpty())
            ret += QString::fromLatin1(", ");
        ret += (*c).first;
        const std::vector<QString>& cols = (*c).second.columns;
        if (cols.size())
        {
            ret += QString::fromLatin1("(");
            for (std::vector<QString>::const_iterator ci = cols.begin();
                ci != cols.end(); ++ci)
            {
                if (ci != cols.begin())
                    ret += QString::fromLatin1(",");
                Identifier id(*ci);
                ret += id.getQuoted();
            }
            ret += QString::fromLatin1(")");
        }
    }

    if (ret.isEmpty())          // no privileges found
        return QString();

    ret = QString::fromLatin1("GRANT ") + ret + QString::fromLatin1("\n ON ");
    if (dynamic_cast<Procedure *>(parentObjectM))
        ret += QString::fromLatin1("PROCEDURE ");
    Identifier id(granteeM);
    ret += parentObjectM->getQuotedName() + QString::fromLatin1(" TO ")
        + granteeTypeToString(granteeTypeM) + QString::fromLatin1(" ") + id.getQuoted();

    if (withGrantOption)
        ret += QString::fromLatin1(" WITH GRANT OPTION");
    ret += QString::fromLatin1(";\n");
    return ret;
}
//-----------------------------------------------------------------------------
QString Privilege::getSql() const
{
    Role *r = dynamic_cast<Role *>(parentObjectM);
    if (!r)
        return getSql(true) + getSql(false);

    QString ret = QString::fromLatin1("GRANT ") + r->getQuotedName() + QString::fromLatin1(" TO ") + granteeM;
    for (PMap::const_iterator c = privilegeItemsM.begin();
        c != privilegeItemsM.end(); ++c)
    {
        if ((*c).second.grantOption)
        {
            ret += QString::fromLatin1(" WITH ADMIN OPTION");
            break;
        }
    }
    ret += QString::fromLatin1(";\n");
    return ret;
}
//-----------------------------------------------------------------------------
QString Privilege::getGrantee() const
{
    QString gt = granteeTypeToString(granteeTypeM);
    if (!gt.isEmpty())
        gt += QString::fromLatin1(" ");
    return gt + granteeM;
}
//-----------------------------------------------------------------------------
void Privilege::getPrivilegeItems(const QString& type,
    std::vector<PrivilegeItem>& list) const
{
    PMap::const_iterator it;
    for (it = privilegeItemsM.lower_bound(type);
        it != privilegeItemsM.upper_bound(type); ++it)
    {
        list.push_back((*it).second);
    }
}
//-----------------------------------------------------------------------------

} // namespace fr
