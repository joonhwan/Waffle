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


  $Id: Config.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
#include "config/Config.h"
#ifdef HAVE_FRCONFIG_H
    #include "frconfig.h"
#endif
#include "core/FRError.h"

#include <QtGlobal>
#include <QApplication>
#include <QSettings>
#include <QFileInfo>
#include <QDir>

namespace fr {

//-----------------------------------------------------------------------------
const QString Config::pathSeparator = QString::fromLatin1("/");
//-----------------------------------------------------------------------------
FRConfig& config()
{
    static FRConfig c;
    return c;
}
//-----------------------------------------------------------------------------
Config::Config()
    : configM(0), needsFlushM(false)
{
#ifdef FR_CONFIG_USE_PRIVATE_STDPATHS
    standardPathsM.SetInstallPrefix(QString::fromLatin1(FR_INSTALL_PREFIX));
#endif
}
//-----------------------------------------------------------------------------
Config::~Config()
{
    delete configM;
}
//-----------------------------------------------------------------------------
QSettings* Config::getConfig() const
{
    if (!configM)
    {
        QFileInfo configFileName = getConfigFileName();
		QDir configFileDir = configFileName.absoluteDir();
        if (!configFileDir.exists())
            configFileDir.mkdir(configFileDir.path());
        configM = new QSettings(configFileName.absoluteFilePath(),
								QSettings::NativeFormat);
        // configM->SetExpandEnvVars(false);
    }
    return configM;
}
//-----------------------------------------------------------------------------
void Config::lockedChanged(bool locked)
{
    // delay getConfig()->sync() until object is completely unlocked again
    if (!locked && needsFlushM)
    {
        needsFlushM = false;
        getConfig()->sync();
    }
}
//-----------------------------------------------------------------------------
//! return true if value exists, false if not
bool Config::keyExists(const QString& key) const
{
    return getConfig()->value(key) != QVariant();
}
//-----------------------------------------------------------------------------
//! return true if value exists, false if not
bool Config::getValue(const QString& key, QString& value)
{
	QVariant s;
    if (!getValueV(key, s))
        return false;

    value = s.toString();
    return true;
}

bool Config::getValueV(const QString& key, QVariant& value)
{
    // if complete key is found, then return (recursion exit condition).
    QString configValue;
	if (keyExists(key)) {
		value = getConfig()->value(key);
		return true;
	}

    // does key contain a separator? If not, then the key is not found and
    // we're done.
    int separatorPos = key.lastIndexOf(pathSeparator);
    if (separatorPos == -1)
        return false;
    else
    {
        // split key into keyPart and pathPart; remove last component of
        // pathPart and recurse.
        QString keyPart = key.mid(separatorPos + 1, key.length());
        QString pathPart = key.mid(0, separatorPos);
        int separatorPosInPath = pathPart.lastIndexOf(pathSeparator);
        if (separatorPosInPath == -1)
            return getValueV(keyPart, value);
        else
        {
            return getValueV(pathPart.mid(0, separatorPosInPath) +
                pathSeparator + keyPart, value);
        }
    }
}


//-----------------------------------------------------------------------------
bool Config::getValue(const QString& key, int& value)
{
    QVariant s;
    if (!getValueV(key, s))
        return false;

    // This variable is only needed because the compiler considers
    // int* and long* incompatible. It may be ditched if a better solution
    // is found.
    if (!s.canConvert<long>())
        return false;

    value = s.toInt();
    return true;
}
//-----------------------------------------------------------------------------
bool Config::getValue(const QString& key, double& value)
{
    QVariant s;
    if (!getValueV(key, s))
        return false;

    if (!s.canConvert<double>())
        return false;

    return true;
}
//-----------------------------------------------------------------------------
bool Config::getValue(const QString& key, bool& value)
{
    QVariant s;
    if (!getValueV(key, s))
        return false;

    value = (s.toString() == QString::fromLatin1("1"));
    return true;
}
//-----------------------------------------------------------------------------
bool Config::getValue(const QString& key, StorageGranularity& value)
{
    QVariant intValue = 0;
    bool ret = getValueV(key, intValue);
    if (ret)
        value = StorageGranularity(intValue.toInt());
    return ret;
}
//-----------------------------------------------------------------------------
bool Config::getValue(const QString& key, QStringList& value)
{
    QVariant s;
    if (!getValueV(key, s))
        return false;

	if (!s.canConvert<QStringList>())
		return false;

	value = s.toStringList();
	return true;
    // value.clear();
    // QString item;
    // size_t pos = 0, sep = s.find(',');
    // while (sep != QString::npos)
    // {
    //     item = s.substr(pos, sep - pos);
    //     if (!item.isEmpty())
    //         value.push_back(item);
    //     sep = s.find(',', pos = sep + 1);
    // }
    // if (!(item = s.substr(pos)).isEmpty())
    //     value.push_back(item);
    // return true;
}
//-----------------------------------------------------------------------------
//! return true if value existed, false if not
bool Config::setValue(const QString& key, const QString& value)
{
    return setValueV(key, QVariant(value));
}

bool Config::setValueV(const QString& key, QVariant& value)
{
	bool result = true;
	getConfig()->setValue(key, value);
    if (!isLocked())
        getConfig()->sync();
    else
        needsFlushM = true;
    notifyObservers();
    return result;
}

//-----------------------------------------------------------------------------
bool Config::setValue(const QString& key, int value)
{
    return setValueV(key, QVariant(value));
}
//-----------------------------------------------------------------------------
bool Config::setValue(const QString& key, double value)
{
    return setValueV(key, QVariant(value));
}

//-----------------------------------------------------------------------------
bool Config::setValue(const QString& key, bool value)
{
    // if (value)
    //     return setValueV(key, QString(QString::fromLatin1("1")));
    // else
    //     return setValueV(key, QString(QString::fromLatin1("0")));
	return setValueV(key, QVariant(value));
}
//-----------------------------------------------------------------------------
bool Config::setValue(const QString& key, StorageGranularity value)
{
    return setValueV(key, QVariant(int(value)));
}
//-----------------------------------------------------------------------------
bool Config::setValue(const QString& key, const QStringList& value)
{
    // QString s;
    // for (QStringList::const_iterator it = value.begin(); it != value.end();
    //     it++)
    // {
    //     if (it != value.begin())
    //         s += QString::fromLatin1(",");
    //     // this is just a parachute, if this should ever be triggered we
    //     // will need to quote and unquote this QString or all in value
    //     wxASSERT((*it).find(',') == QString::npos);
    //     s += *it;
    // }
    // return setValue(key, s);
	return setValueV(key, QVariant(value));
}
//-----------------------------------------------------------------------------
QString Config::getHomePath() const
{
    if (!homePathM.isEmpty())
        return homePathM + QDir::separator();
    else
        return getDataDir() + QDir::separator();
}
//-----------------------------------------------------------------------------
QString Config::getUserHomePath() const
{
    if (!userHomePathM.isEmpty())
        return userHomePathM + QDir::separator();
    else
        return getUserLocalDataDir() + QDir::separator();
}
//-----------------------------------------------------------------------------
QFileInfo Config::getConfigFileName() const
{
    return configFileNameM;
}
//-----------------------------------------------------------------------------
void Config::setConfigFileName(const QFileInfo& fileName)
{
    configFileNameM = fileName;
}
//-----------------------------------------------------------------------------
QString Config::getDataDir() const
{
// #ifdef FR_CONFIG_USE_PRIVATE_STDPATHS
//     return standardPathsM.GetDataDir();
// #else
//     return wxStandardPaths::Get().GetDataDir();
// #endif
	return QCoreApplication::instance()->applicationDirPath();
}
//-----------------------------------------------------------------------------
QString Config::getLocalDataDir() const
{
// #ifdef FR_CONFIG_USE_PRIVATE_STDPATHS
//     return standardPathsM.GetLocalDataDir();
// #else
//     return wxStandardPaths::Get().GetLocalDataDir();
// #endif
	return QCoreApplication::instance()->applicationDirPath();
}
//-----------------------------------------------------------------------------
QString Config::getUserLocalDataDir() const
{
// #ifdef FR_CONFIG_USE_PRIVATE_STDPATHS
//     return standardPathsM.GetUserLocalDataDir();
// #else
//     return wxStandardPaths::Get().GetUserLocalDataDir();
// #endif
	return qgetenv("APPDATA");
}
//-----------------------------------------------------------------------------
void Config::setHomePath(const QString& homePath)
{
    homePathM = homePath;
}
//-----------------------------------------------------------------------------
void Config::setUserHomePath(const QString& userHomePath)
{
    userHomePathM = userHomePath;
}
//-----------------------------------------------------------------------------
// class ConfigCache
ConfigCache::ConfigCache(Config& config)
    : Observer(), cacheValidM(false)
{
    config.attachObserver(this, false);
}
//-----------------------------------------------------------------------------
void ConfigCache::ensureCacheValid()
{
    if (!cacheValidM)
    {
        loadFromConfig();
        cacheValidM = true;
    }
}
//-----------------------------------------------------------------------------
void ConfigCache::loadFromConfig()
{
}
//-----------------------------------------------------------------------------
void ConfigCache::update()
{
    // next call to ensureCacheValid() will reload the cached information
    cacheValidM = false;
}
//-----------------------------------------------------------------------------
QString FRConfig::getHtmlTemplatesPath() const
{
    return getHomePath() + QString::fromLatin1("html-templates")
        + QDir::separator();
}
//-----------------------------------------------------------------------------
QString FRConfig::getCodeTemplatesPath() const
{
    return getHomePath() + QString::fromLatin1("code-templates")
        + QDir::separator();
}
//-----------------------------------------------------------------------------
QString FRConfig::getUserCodeTemplatesPath() const
{
    return getUserHomePath() + QString::fromLatin1("code-templates")
        + QDir::separator();
}
//-----------------------------------------------------------------------------
QString FRConfig::getSysTemplatesPath() const
{
    return getHomePath() + QString::fromLatin1("sys-templates")
        + QDir::separator();
}
//-----------------------------------------------------------------------------
QString FRConfig::getUserSysTemplatesPath() const
{
    return getUserHomePath() + QString::fromLatin1("sys-templates")
        + QDir::separator();
}
//-----------------------------------------------------------------------------
QString FRConfig::getDocsPath() const
{
    return getHomePath() + QString::fromLatin1("docs") + QDir::separator();
}
//-----------------------------------------------------------------------------
QString FRConfig::getConfDefsPath() const
{
    return getHomePath() + QString::fromLatin1("conf-defs") + QDir::separator();
}
//-----------------------------------------------------------------------------
QString FRConfig::getImagesPath() const
{
    return getHomePath() + QString::fromLatin1("images") + QDir::separator();
}
//-----------------------------------------------------------------------------
QString FRConfig::getDBHFileName() const
{
    return getUserHomePath() + QString::fromLatin1("fr_databases.conf");
}
//-----------------------------------------------------------------------------
QFileInfo FRConfig::getConfigFileName() const
{
    return QFileInfo(getUserHomePath(), QString::fromLatin1("fr_settings.conf"));
}
//-----------------------------------------------------------------------------
const QString FRConfig::getSysTemplateFileName(const QString& templateName)
{
    QFileInfo fileName = getUserSysTemplatesPath() + templateName
        + QString::fromLatin1(".template");
    if (!fileName.exists())
        fileName = getSysTemplatesPath() + templateName + QString::fromLatin1(".template");
    if (!fileName.exists())
        fileName = getUserCodeTemplatesPath() + templateName + QString::fromLatin1(".template");
    if (!fileName.exists())
        fileName = getCodeTemplatesPath() + templateName + QString::fromLatin1(".template");
    if (!fileName.exists())
    {
        fileName = getSysTemplatesPath() + templateName + QString::fromLatin1(".template");
        if (!fileName.exists())
        {
            throw FRError(QString::fromLatin1("Template \"%!\" not found.")
						  .arg(fileName.absoluteFilePath()));
        }
    }
    return fileName.absoluteFilePath();
}
//-----------------------------------------------------------------------------

} // namespace fr
