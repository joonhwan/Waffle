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


  $Id: exception.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "ibpp/ibpp.h"

#include "core/FRError.h"
#include "core/StringUtils.h"
#include "engine/MetadataLoader.h"
#include "metadata/database.h"
#include "metadata/exception.h"
#include "metadata/MetadataItemVisitor.h"
#include "sql/StatementBuilder.h"

namespace fr {

//-----------------------------------------------------------------------------
Exception::Exception(DatabasePtr database, const QString& name)
    : MetadataItem(ntException, database.get(), name), numberM(0)
{
}
//-----------------------------------------------------------------------------
QString Exception::getMessage()
{
    ensurePropertiesLoaded();
    return messageM;
}
//-----------------------------------------------------------------------------
int Exception::getNumber()
{
    ensurePropertiesLoaded();
    return numberM;
}
//-----------------------------------------------------------------------------
void Exception::loadProperties()
{
    setPropertiesLoaded(false);

    DatabasePtr db = getDatabase();
    MetadataLoader* loader = db->getMetadataLoader();
    MetadataLoaderTransaction tr(loader);

    IBPP::Statement& st1 = loader->getStatement(
        "select RDB$MESSAGE, RDB$EXCEPTION_NUMBER from RDB$EXCEPTIONS"
        " where RDB$EXCEPTION_NAME = ?");
    st1->Set(1, wx2std(getName_(), db->getCharsetConverter()));
    st1->Execute();
    st1->Fetch();
    std::string message;
    st1->Get(1, message);
    messageM = std2wx(message, db->getCharsetConverter());
    st1->Get(2, numberM);

    setPropertiesLoaded(true);
}
//-----------------------------------------------------------------------------
QString Exception::getAlterSql()
{
    QString message = getMessage();
    message.replace(QString::fromLatin1("'"), QString::fromLatin1("''"));

    StatementBuilder sb;
    sb << kwALTER << ' ' << kwEXCEPTION << ' ' << getQuotedName() << QString::fromLatin1(" '")
        << message << QString::fromLatin1("';");
    return sb;
}
//-----------------------------------------------------------------------------
const QString Exception::getTypeName() const
{
    return QString::fromLatin1("EXCEPTION");
}
//-----------------------------------------------------------------------------
void Exception::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitException(*this);
}
//-----------------------------------------------------------------------------
// Exceptions collection
Exceptions::Exceptions(DatabasePtr database)
    : MetadataCollection<Exception>(ntExceptions, database, _("Exceptions"))
{
}
//-----------------------------------------------------------------------------
void Exceptions::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitExceptions(*this);
}
//-----------------------------------------------------------------------------
void Exceptions::load(ProgressIndicator* progressIndicator)
{
    QString stmt = QString::fromLatin1(
			"select rdb$exception_name from rdb$exceptions"
			" order by 1"
		);
    setItems(getDatabase()->loadIdentifiers(stmt, progressIndicator));
}
//-----------------------------------------------------------------------------
void Exceptions::loadChildren()
{
    load(0);
}
//-----------------------------------------------------------------------------
const QString Exceptions::getTypeName() const
{
    return QString::fromLatin1("EXCEPTION_COLLECTION");
}
//-----------------------------------------------------------------------------

} // namespace fr
