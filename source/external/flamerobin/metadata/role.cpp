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


  $Id: role.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
#include "ibpp/ibpp.h"

#include "core/FRError.h"
#include "core/StringUtils.h"
#include "engine/MetadataLoader.h"
#include "metadata/database.h"
#include "metadata/MetadataItemVisitor.h"
#include "metadata/role.h"
#include "flamerobin-port-helper.h"

namespace fr {

//-----------------------------------------------------------------------------
Role::Role(DatabasePtr database, const QString& name)
    : MetadataItem(hasSystemPrefix(name) ? ntSysRole : ntRole, database.get(),
        name)
{
}
//-----------------------------------------------------------------------------
std::vector<Privilege>* Role::getPrivileges()
{
    // load privileges from database and return the pointer to collection
    DatabasePtr db = getDatabase();
    MetadataLoader* loader = db->getMetadataLoader();
    // first start a transaction for metadata loading, then lock the role
    // when objects go out of scope and are destroyed, role will be
    // unlocked before the transaction is committed - any update() calls on
    // observers can possibly use the same transaction
    MetadataLoaderTransaction tr(loader);
    SubjectLocker lock(this);

    privilegesM.clear();

    IBPP::Statement st1 = loader->getStatement(
        "select RDB$USER, RDB$USER_TYPE, RDB$GRANTOR, RDB$PRIVILEGE, "
        "RDB$GRANT_OPTION "
        "from RDB$USER_PRIVILEGES "
        "where RDB$RELATION_NAME = ? and rdb$object_type = 13 "
        "order by rdb$user, rdb$user_type, rdb$privilege"
    );
    st1->Set(1, wx2std(getName_(), db->getCharsetConverter()));
    st1->Execute();
    std::string lastuser;
    int lasttype = -1;
    Privilege *pr = 0;
    while (st1->Fetch())
    {
        std::string user, grantor, privilege;
        int usertype, grantoption = 0;
        st1->Get(1, user);
        st1->Get(2, usertype);
        st1->Get(3, grantor);
        st1->Get(4, privilege);
        if (!st1->IsNull(5))
            st1->Get(5, grantoption);
        if (!pr || user != lastuser || usertype != lasttype)
        {
            Privilege p(this, helper::stripString(std2wx(user)), usertype);
            privilegesM.push_back(p);
            pr = &privilegesM.back();
            lastuser = user;
            lasttype = usertype;
        }
        pr->addPrivilege(privilege[0], helper::stripString(std2wx(grantor)),
						 grantoption != 0);  // ADMIN OPTION = 2
    }
    return &privilegesM;
}
//-----------------------------------------------------------------------------
QString Role::getOwner()
{
    DatabasePtr db = getDatabase();
    MetadataLoader* loader = db->getMetadataLoader();
    MetadataLoaderTransaction tr(loader);

    IBPP::Statement st1 = loader->getStatement(
        "select rdb$owner_name from rdb$roles where rdb$role_name = ?");
    st1->Set(1, wx2std(getName_(), db->getCharsetConverter()));
    st1->Execute();
    st1->Fetch();
    std::string name;
    st1->Get(1, name);
    return helper::stripString(std2wx(name));
}
//-----------------------------------------------------------------------------
const QString Role::getTypeName() const
{
    return QString::fromLatin1("ROLE");
}
//-----------------------------------------------------------------------------
void Role::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitRole(*this);
}
//-----------------------------------------------------------------------------
// System roles collection
SysRoles::SysRoles(DatabasePtr database)
    : MetadataCollection<Role>(ntSysRoles, database, _("System roles"))
{
}
//-----------------------------------------------------------------------------
void SysRoles::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitSysRoles(*this);
}
//-----------------------------------------------------------------------------
bool SysRoles::isSystem() const
{
    return true;
}
//-----------------------------------------------------------------------------
void SysRoles::load(ProgressIndicator* progressIndicator)
{
    DatabasePtr db = getDatabase();
    if (db && db->getInfo().getODSVersionIsHigherOrEqualTo(11, 1))
    {
        QString stmt = QString::fromLatin1(
				"select rdb$role_name from rdb$roles"
				" where (rdb$system_flag > 0) order by 1"
			);
        setItems(getDatabase()->loadIdentifiers(stmt, progressIndicator));
    }
}
//-----------------------------------------------------------------------------
void SysRoles::loadChildren()
{
    load(0);
}
//-----------------------------------------------------------------------------
const QString SysRoles::getTypeName() const
{
    return QString::fromLatin1("SYSROLE_COLLECTION");
}
//-----------------------------------------------------------------------------
// Roles collection
Roles::Roles(DatabasePtr database)
    : MetadataCollection<Role>(ntRoles, database, _("Roles"))
{
}
//-----------------------------------------------------------------------------
void Roles::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitRoles(*this);
}
//-----------------------------------------------------------------------------
void Roles::load(ProgressIndicator* progressIndicator)
{
    QString stmt = QString::fromLatin1("select rdb$role_name from rdb$roles");
    DatabasePtr db = getDatabase();
    if (db && db->getInfo().getODSVersionIsHigherOrEqualTo(11, 1))
        stmt += QString::fromLatin1(" where (rdb$system_flag = 0 or rdb$system_flag is null)");
    stmt += QString::fromLatin1(" order by 1");
    setItems(getDatabase()->loadIdentifiers(stmt, progressIndicator));
}
//-----------------------------------------------------------------------------
void Roles::loadChildren()
{
    load(0);
}
//-----------------------------------------------------------------------------
const QString Roles::getTypeName() const
{
    return QString::fromLatin1("ROLE_COLLECTION");
}
//-----------------------------------------------------------------------------

} // namespace fr

