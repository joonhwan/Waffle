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


  $Id: trigger.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
//-----------------------------------------------------------------------------
#include "ibpp/ibpp.h"

#include "core/FRError.h"
#include "core/StringUtils.h"
#include "engine/MetadataLoader.h"
#include "frutils.h"
#include "metadata/database.h"
#include "metadata/MetadataItemVisitor.h"
#include "metadata/trigger.h"
#include <QTextStream>

namespace fr {

//-----------------------------------------------------------------------------
Trigger::Trigger(DatabasePtr database, const QString& name)
    : MetadataItem(ntTrigger, database.get(), name)
{
}
//-----------------------------------------------------------------------------
void Trigger::getTriggerInfo(QString& object, bool& active, int& position,
    QString& type, bool& isDatabaseTrigger)
{
    ensurePropertiesLoaded();
    isDatabaseTrigger = isDatabaseTriggerM;
    object = objectM;
    active = activeM;
    position = positionM;
    type = triggerTypeM;
}
//-----------------------------------------------------------------------------
QString Trigger::getTriggerRelation()
{
    ensurePropertiesLoaded();
    // gcc 4.4.3 on Ubuntu didn't accept QString()
    return (isDatabaseTriggerM ? QString::fromLatin1("") : objectM);
}
//-----------------------------------------------------------------------------
void Trigger::loadProperties()
{
    setPropertiesLoaded(false);

    DatabasePtr db = getDatabase();
    MetadataLoader* loader = db->getMetadataLoader();
    MetadataLoaderTransaction tr(loader);

    IBPP::Statement& st1 = loader->getStatement(
        "select t.rdb$relation_name, t.rdb$trigger_sequence, "
        "t.rdb$trigger_inactive, t.rdb$trigger_type "
        "from rdb$triggers t where rdb$trigger_name = ? "
    );

    st1->Set(1, wx2std(getName_(), db->getCharsetConverter()));
    st1->Execute();
    if (st1->Fetch())
    {
        isDatabaseTriggerM = st1->IsNull(1);
        if (!isDatabaseTriggerM)
        {
            std::string objname;
            st1->Get(1, objname);
            objectM = std2wxIdentifier(objname, db->getCharsetConverter());
        }
        st1->Get(2, &positionM);

        short temp;
        if (st1->IsNull(3))
            temp = 0;
        else
            st1->Get(3, &temp);
        activeM = (temp == 0);

        int ttype;
        st1->Get(4, &ttype);
        triggerTypeM = getTriggerType(ttype);
    }
    else    // maybe trigger was dropped?
    {
        //wxMessageBox("Trigger does not exist in database");
        objectM = QString();
    }

    setPropertiesLoaded(true);
}
//-----------------------------------------------------------------------------
QString Trigger::getSource() const
{
    DatabasePtr db = getDatabase();
    MetadataLoader* loader = db->getMetadataLoader();
    MetadataLoaderTransaction tr(loader);

    IBPP::Statement& st1 = loader->getStatement(
        "select rdb$trigger_source from rdb$triggers"
        " where rdb$trigger_name = ?"
		);
    st1->Set(1, wx2std(getName_(), db->getCharsetConverter()));
    st1->Execute();
    st1->Fetch();
    QString source;
    readBlob(st1, 1, source, db->getCharsetConverter());
    return source;
}
//-----------------------------------------------------------------------------
QString Trigger::getTriggerType(int type)
{
    if (type >= 8192 && type <= 8196)   // database triggers
    {
        QString ttype[] = {
            QString::fromLatin1("CONNECT"), QString::fromLatin1("DISCONNECT"), QString::fromLatin1("TRANSACTION START"),
            QString::fromLatin1("TRANSACTION COMMIT"), QString::fromLatin1("TRANSACTION ROLLBACK") };
        return QString(QString::fromLatin1("ON ")) + ttype[type - 8192];
    }

    // For explanation: read README.universal_triggers file in Firebird's
    //                  doc/sql.extensions directory
    QString result(type % 2 ? QString::fromLatin1("BEFORE ") : QString::fromLatin1("AFTER "));
    QString types[] = { QString::fromLatin1("INSERT"), QString::fromLatin1("UPDATE"), QString::fromLatin1("DELETE") };
    type++;         // compensate for decrement
    type >>= 1;     // remove bit 0
    for (int i = 0; i < 3; ++i, type >>= 2)
    {
        if (type % 4)
        {
            if (i)
                result += QString::fromLatin1(" OR ");
            result += types[ (type%4) - 1 ];
        }
    }
    return result;
}
//-----------------------------------------------------------------------------
Trigger::fireTimeType Trigger::getFiringTime()
{
    ensurePropertiesLoaded();
    if (isDatabaseTriggerM)
        return databaseTrigger;
    if (triggerTypeM.mid(0, 6) == QString::fromLatin1("BEFORE"))
        return beforeTrigger;
    else
        return afterTrigger;
}
//-----------------------------------------------------------------------------
QString Trigger::getAlterSql()
{
    QString object, type;
    bool active, db;
    int position;

    getTriggerInfo(object, active, position, type, db);
    QString source = getSource();
    QString sqlString;
	{
		QTextStream sql(&sqlString);
		sql << QString::fromLatin1("SET TERM ^ ;\nALTER TRIGGER ") << getQuotedName();
		if (active)
			sql << QString::fromLatin1(" ACTIVE\n");
		else
			sql << QString::fromLatin1(" INACTIVE\n");
		sql << type;
		sql << QString::fromLatin1(" POSITION ");
		sql << position << QString::fromLatin1("\n");
		sql << source;
		sql << QString::fromLatin1("^\nSET TERM ; ^");
	}
    return sqlString;
}
//-----------------------------------------------------------------------------
const QString Trigger::getTypeName() const
{
    return QString::fromLatin1("TRIGGER");
}
//-----------------------------------------------------------------------------
void Trigger::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitTrigger(*this);
}
//-----------------------------------------------------------------------------
// Triggers collection
Triggers::Triggers(DatabasePtr database)
    : MetadataCollection<Trigger>(ntTriggers, database, _("Triggers"))
{
}
//-----------------------------------------------------------------------------
void Triggers::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitTriggers(*this);
}
//-----------------------------------------------------------------------------
void Triggers::load(ProgressIndicator* progressIndicator)
{
    QString stmt = QString::fromLatin1(
			"select rdb$trigger_name from rdb$triggers"
			" where (rdb$system_flag = 0 or rdb$system_flag is null)"
			" order by 1"
		);
    setItems(getDatabase()->loadIdentifiers(stmt, progressIndicator));
}
//-----------------------------------------------------------------------------
void Triggers::loadChildren()
{
    load(0);
}
//-----------------------------------------------------------------------------
const QString Triggers::getTypeName() const
{
    return QString::fromLatin1("TRIGGER_COLLECTION");
}
//-----------------------------------------------------------------------------

} // namespace fr
