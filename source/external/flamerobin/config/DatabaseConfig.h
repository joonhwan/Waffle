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


  $Id: DatabaseConfig.h 2200 2012-01-20 08:31:01Z mghie $

*/

//! This class is interface between Config and Database classes
//  Preferences dialog takes Config class as parameter, so interface like this
//  is needed
//
#ifndef FR_DATABASE_CONFIG_H
#define FR_DATABASE_CONFIG_H

#include "config/Config.h"
#include "metadata/MetadataClasses.h"

namespace fr {

//-----------------------------------------------------------------------------
class DatabaseConfig: public Config
{
private:
    const Database* databaseM;
    Config& referenceConfigM;
    QString addPathToKey(const QString& key) const;

public:
    DatabaseConfig(const Database *d, Config& referenceConfig);

    // unhides methods of base class, for details see:
    // http://www.parashift.com/c++-faq-lite/strange-inheritance.html#faq-23.7
    using Config::getValue;
    using Config::setValue;

    // transform the key based on Database, and call regular config
    virtual bool keyExists(const QString& key) const;
    virtual bool getValue(const QString& key, QString& value);
    virtual bool setValue(const QString& key, const QString& value);
};
//-----------------------------------------------------------------------------

} // namespace fr

#endif
