#include "WLuaSettings.h"
#include "WLuaHelpers.h"
#include "qglobal.h"
#include <QMutex>
#include <QDebug>
#include <QRegExp>
#include <QStringList>
#include <QDataStream>
#include <QByteArray>
#include <QImage>
#include <QBuffer>

namespace {

const char* INTERNAL_OBJECT_PTR = "__internal_object_pointer";

const int INDEX_BASE_NUMBER = 1;

}

void mapLuaVariables(void* luaState, const QString& parentName,
					 QSettings::SettingsMap& map,
					 int depth);

// static
QSettings::Format WLuaSettings::s_format =
	QSettings::registerFormat("lua", WLuaSettings::readLuaFileCB, WLuaSettings::writeLuaFileCB);

//static
QMutex WLuaSettings::s_lock;

WLuaSettings::WLuaSettings(const QString& fileName, QObject* parent)
	: QSettings(fileName,
				// QSettings::IniFormat,
				// // QSettings::NativeFormat,
				s_format,
				parent)
{
}

//virtual
WLuaSettings::~WLuaSettings()
{
}

//static
bool WLuaSettings::readLuaFileCB(QIODevice &device, QSettings::SettingsMap &map)
{
	QMutexLocker locker(&WLuaSettings::s_lock);

	QByteArray script = device.readAll();

	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	if (luaL_loadbuffer(L, script.data(), script.size(), "WriteFunc") ||
		lua_pcall(L, 0, 0, 0))
	{
		qDebug() << QLatin1String(lua_tostring(L, -1));
	}

	lua_getglobal(L, "config");
	mapLuaVariables(L, QLatin1String("config"), map, 0);

	return true;

	// if (!map.contains(INTERNAL_OBJECT_PTR)) {
	// 	return true;
	// }
	// WLuaSettings* me = map[INTERNAL_OBJECT_PTR].value<WLuaSettings*>();
	// return me->readLuaFile(device, map);
}

//static
bool WLuaSettings::writeLuaFileCB(QIODevice &device, const QSettings::SettingsMap &map)
{
	QMutexLocker locker(&WLuaSettings::s_lock);

	// if (!map.contains(INTERNAL_OBJECT_PTR)) {
	// 	return true;
	// }
	// WLuaSettings* me = map[INTERNAL_OBJECT_PTR].value<WLuaSettings*>();
	// return me->writeLuaFile(device, map);

	QSet<QString> tablizedPath;

	QRegExp numeric("[0-9]+");
	QString script;
	QLatin1String slash("/");
	QLatin1String dot(".");
	QLatin1String quote("\"");
	QLatin1String imageHeader("IMAGE");
	QLatin1String root("config");

	for (QSettings::SettingsMap::iterator it = map.begin();
		 it != map.end(); ++it) {

		QString namePath = it.key();

		QVariant value = it.value();
		QString valueString;
		switch (value.type()) {
		case QVariant::Image: {
			QByteArray buffer;
			{
				QBuffer io(&buffer);
				io.open(QIODevice::WriteOnly);
				value.value<QImage>().save(&io, "PNG");
			}
			// binary data is encoded into base64 which is also string(quoted!)
			valueString = quote + imageHeader + buffer.toBase64() + quote;
			// Q_ASSERT(valueString.size() == base64.size());
			break;
		}
		case QVariant::String: {
			// string should be quoted.
			valueString = quote + value.toString() + quote;
			break;
		}
		default: {
			valueString = value.toString();
			break;
		}
		}
		// path normalization
		//  - any number only path should be prefixed with "_" character.
		QStringList names = namePath.split(slash);
		names.insert(0, root);
		namePath.clear();
		for (int i = 0; i < names.size(); ++i) {
			QString& name = names[i];
			if (numeric.exactMatch(name)) {
				name = QLatin1String("_") + QString("%1").arg(name.toInt(),3,10,QLatin1Char('0'));
			}
		}
		namePath = names.join(dot);

		// create table considering path's directory structure.
		QString nameDir;
		QString parentDir;
		for (int i = 0; i < names.size()-1; ++i) {
			const QString& name = names[i];
			if (name == "size") {
				if (tablizedPath.contains(parentDir)) {
					continue;
				}
			}
			if (!nameDir.isEmpty()) {
				nameDir += dot;
			}
			nameDir += name;
			if (!tablizedPath.contains(nameDir)) {
				// `dir.to.this.variable = {}' in lua script will create empty table.
				script += QString("%1 = {}\n").arg(nameDir);
				tablizedPath.insert(nameDir);
			}

			parentDir = nameDir;
		}

		// final lua script to set variable's value
		script += QString("%1 = %2\n").arg(namePath).arg(valueString);
	}
	// qDebug() << script;
	// device.write(script.toLocal8Bit());

	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	#include "DataDumper.lua.c"
	QByteArray ansiScript = script.toLocal8Bit();
	if (luaL_loadbuffer(L, ansiScript.data(), ansiScript.size(), "WriteFunc") ||
		lua_pcall(L, 0, 0, 0))
	{
		qDebug() << QLatin1String(lua_tostring(L, -1));
	}

	QString saveScript = QString("dumped = DataDumper(config, \"config\")");
	QByteArray ansiSaveScript = saveScript.toLocal8Bit();
	if (luaL_loadbuffer(L, ansiSaveScript.data(), ansiSaveScript.size(), "SaveVariableToFile") ||
		lua_pcall(L, 0, 0, 0))
	{
		qDebug() << QLatin1String(lua_tostring(L, -1));
	}
	lua_getglobal(L, "dumped");
	QByteArray dumped = lua_tostring(L, -1);
	lua_pop(L, -1);

	device.write(dumped);
	lua_close(L);

	return true;
}

void mapLuaVariables(void* luaState, const QString& parentName,
					 QSettings::SettingsMap& map,
					 int depth)
{
	lua_State* L = (lua_State*)luaState;

    /* get the table */
    int t = lua_gettop( L );
    /* get the first key */
    lua_pushnil( L );

	QRegExp prefixedNumeric("_[0-9]+");
	QLatin1String slash("/");

    while( lua_next(L, t) != 0 )
    {
		QString varName;
		bool isNumericKey = false;
        if( lua_isstring( L, -2 ) ) {
            // save the name: lua_tostring( L, -2 )
			varName = QLatin1String(lua_tostring(L, -2));
			if (prefixedNumeric.exactMatch(varName)) {
				// remove underscore "_"
				varName = varName.remove(0,1);
				varName = QString("%1").arg(varName.toInt());
				isNumericKey = true;
			}
        } else if( lua_isnumber( L, -2 ) ) {
            // save the index: static_cast<UINT>( lua_tonumber( L, -2 ) )
			varName = QString("%1").arg(lua_tonumber(L, -2)).toAscii();
			isNumericKey = true;
        } else {
            // error, unknown type: lua_typename( L, lua_type( L, -2 ) )
			Q_ASSERT(FALSE);
			continue;
        }

		// ignore internal top level
		QString key = depth ? parentName + slash + varName : varName;

		// guess correct 'possible' number from index number
		if (isNumericKey && parentName.size()) {
			int index = varName.toInt();
			int expectedCount = index - INDEX_BASE_NUMBER + 1;
			QString sizeVarName = parentName + slash + QLatin1String("size");
			if (map.contains(sizeVarName)) {
				int count = map[sizeVarName].value<int>();
				if (expectedCount >= count) {
					map[sizeVarName] = expectedCount;
				}
			} else {
				map[sizeVarName] = expectedCount;
			}
		}

		bool pop = true;
        if( lua_isstring( L, -1 ) ) {
            // save the value: lua_tostring( L, -1 )
			QLatin1String stringValue(lua_tostring(L, -1));
			map[key] = stringValue;
			qDebug() << key << " = " << stringValue;
        } else if( lua_isnumber( L, -1 ) ) {
            // save the value: lua_tonumber( L, -1 )
			double doubleValue = lua_tonumber(L, -1);
			map[key] = doubleValue;
			qDebug() << key << " = " << doubleValue;
        } else if( lua_isboolean( L, -1 ) ) {
            // save the value: lua_toboolean( L, -1 )
			bool boolValue = lua_toboolean(L, -1);
			map[key] = boolValue;
			qDebug() << key << " = " << lua_toboolean(L, -1);
        } else if( lua_istable( L, -1 ) ) {
            mapLuaVariables(L, key, map, depth+1);
			pop = false;
        } else {
            // error, unknown type: lua_typename( L, lua_type( L, -1 ) )
        }

        /* removes 'value'; keeps 'key' for next iteration */
		if (pop) {
			lua_pop( L, 1 );
		}
    }
    /* removes 'key' */
    lua_pop( L, 1 );
}
