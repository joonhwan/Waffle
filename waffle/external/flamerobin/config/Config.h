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


  $Id: Config.h 2200 2012-01-20 08:31:01Z mghie $

*/

#ifndef FR_CONFIG_H
#define FR_CONFIG_H

#include <QString>
#include <QFileInfo>
#include <QStringList>

#include "core/Observer.h"
#include "core/Subject.h"

// forward declarations
class QSettings;

namespace fr {

//-----------------------------------------------------------------------------
enum StorageGranularity
{
    sgFrame,        // Store settings per frame type.
    sgObjectType,   // Store settings per object type.
    sgObject,       // Store settings per object.
};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//! Do not instantiate objects of this class. Use config() function (see below).

// #if defined(__UNIX__) && !defined(__WXMAC_OSX__)
//     #define FR_CONFIG_USE_PRIVATE_STDPATHS
// #else
//     #undef FR_CONFIG_USE_PRIVATE_STDPATHS
// #endif
#undef FR_CONFIG_USE_PRIVATE_STDPATHS

//! Base class to be used as generic container of configuration info.
class Config: public Subject
{
private:
    mutable QSettings* configM;
    bool needsFlushM;
    // performs lazy initialization of configM.
    QSettings* getConfig() const;
#ifdef FR_CONFIG_USE_PRIVATE_STDPATHS
    wxStandardPaths standardPathsM;
#endif
    QString homePathM;
    QString userHomePathM;
    QFileInfo configFileNameM;
protected:
    virtual void lockedChanged(bool locked);
public:
    Config();
    virtual ~Config();

    virtual QFileInfo getConfigFileName() const;
    void setConfigFileName(const QFileInfo& fileName);

    static const QString pathSeparator;
    
    // We must use an instance of wxStandardPaths for UNIX, but must not
    // use such an instance for things to work on Mac OS X.  Great...
    // These methods are to work around that, use them instead of
    // wxStandardPaths methods.
    QString getDataDir() const;
    QString getLocalDataDir() const;
    QString getUserLocalDataDir() const;

    // returns the home path to use as the basis for the following calls.
    QString getHomePath() const;
    // returns the home path to use as the basis for the following call.
    QString getUserHomePath() const;

    // these should be called before calling the get* functions below,
    // otherwise defaults apply.
    void setHomePath(const QString& homePath);
    void setUserHomePath(const QString& userHomePath);

    // return true if value exists, false if not
    virtual bool keyExists(const QString& key) const;
    virtual bool getValue(const QString& key, QString& value);
	bool getValueV(const QString& key, QVariant& value);
    bool getValue(const QString& key, int& value);
    bool getValue(const QString& key, double& value);
    bool getValue(const QString& key, bool& value);
    bool getValue(const QString& key, StorageGranularity& value);
    bool getValue(const QString& key, QStringList& value);

    // returns the value for key if it exists, or default value if it doesn't.
    template <typename T>
    T get(const QString& key, const T& defaultValue)
    {
        T temp;
        if (getValue(key, temp))
            return temp;
        else
            return defaultValue;
    }

    // return true if value existed, false if not.
    virtual bool setValue(const QString& key, const QString& value);
	bool setValueV(const QString& key, QVariant& value);
    bool setValue(const QString& key, int value);
    bool setValue(const QString& key, double value);
    bool setValue(const QString& key, bool value);
    bool setValue(const QString& key, StorageGranularity value);
    bool setValue(const QString& key, const QStringList& value);
};
//-----------------------------------------------------------------------------
//! Class used to contain all FlameRobin and database configuration info sets.
class FRConfig: public Config
{
public:
    // this class has a fixed file name - setting it through
    // setConfigFileName() is ineffective.
    virtual QFileInfo getConfigFileName() const;
    // returns the path from which to load HTML templates.
    QString getHtmlTemplatesPath() const;
    // returns the path from which to load code templates.
    QString getCodeTemplatesPath() const;
    // returns the path from which to load user code templates and overrides.
    QString getUserCodeTemplatesPath() const;
    // returns the path from which to load system templates.
    QString getSysTemplatesPath() const;
    // returns the path from which to load user overrides of system templates.
    QString getUserSysTemplatesPath() const;
    // returns the path containing the docs.
    QString getDocsPath() const;
    // returns the path containing the confdefs.
    QString getConfDefsPath() const;
    // returns the path containing the images.
    QString getImagesPath() const;
    // returns the file name (with full path) of the file containing
    // registered databases.
    QString getDBHFileName() const;
    // Returns the full pathname of the specified system template, giving
    // precedence to any existing user override.
    const QString getSysTemplateFileName(const QString& templateName);
};
//-----------------------------------------------------------------------------
FRConfig& config();
//-----------------------------------------------------------------------------
// class ConfigCache
// used to cache settings in a Config instance, observes the instance to
// reload the information when necessary (reloads on-demand)
class ConfigCache: public Observer
{
private:
    bool cacheValidM;
protected:
    void ensureCacheValid();
    virtual void loadFromConfig();
    virtual void update();
public:
    ConfigCache(Config& config);
};
//-----------------------------------------------------------------------------

} // namespace fr

#endif
