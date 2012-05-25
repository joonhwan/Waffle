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


  $Id: DatabaseConfig.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
//-----------------------------------------------------------------------------
#include "config/Config.h"
#include "config/DatabaseConfig.h"
#include "metadata/database.h"

namespace fr {

//-----------------------------------------------------------------------------
DatabaseConfig::DatabaseConfig(const Database *d, Config& referenceConfig)
    :Config(), databaseM(d), referenceConfigM(referenceConfig)
{
    // we need to copy these settings, since they may have been modified
    // by env variables or command line params
    setHomePath(referenceConfigM.getHomePath());
    setUserHomePath(referenceConfigM.getUserHomePath());
}
//-----------------------------------------------------------------------------
QString DatabaseConfig::addPathToKey(const QString& key) const
{
    if (databaseM)
        return QString::fromLatin1("DATABASE_") + databaseM->getId() + Config::pathSeparator + key;
    else
        return QString::fromLatin1("");
}
//-----------------------------------------------------------------------------
bool DatabaseConfig::keyExists(const QString& key) const
{
    return referenceConfigM.keyExists(addPathToKey(key));
}
//-----------------------------------------------------------------------------
bool DatabaseConfig::getValue(const QString& key, QString& value)
{
    return referenceConfigM.getValue(addPathToKey(key), value);
}
//-----------------------------------------------------------------------------
bool DatabaseConfig::setValue(const QString& key, const QString& value)
{
    return referenceConfigM.setValue(addPathToKey(key), value);
}
//-----------------------------------------------------------------------------

} // namespace fr
