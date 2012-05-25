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


  $Id: function.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
#include "ibpp/ibpp.h"

#include "core/StringUtils.h"
#include "engine/MetadataLoader.h"
#include "metadata/database.h"
#include "metadata/domain.h"
#include "metadata/function.h"
#include "metadata/MetadataItemVisitor.h"
#include "sql/StatementBuilder.h"
#include "flamerobin-port-helper.h"

namespace fr {

//-----------------------------------------------------------------------------
Function::Function(DatabasePtr database, const QString& name)
    : MetadataItem(ntFunction, database.get(), name)
{
}
//-----------------------------------------------------------------------------
QString Function::getCreateSql()
{
    ensurePropertiesLoaded();
    StatementBuilder sb;
    sb << kwDECLARE << ' ' << kwEXTERNAL << ' ' << kwFUNCTION << ' '
        << getQuotedName() << StatementBuilder::NewLine
        << paramListM  << StatementBuilder::NewLine
        << kwRETURNS << ' ' << retstrM << StatementBuilder::NewLine
        << kwENTRY_POINT << QString::fromLatin1(" '") << entryPointM << '\''
        << StatementBuilder::NewLine
        << kwMODULE_NAME << QString::fromLatin1(" '") << libraryNameM << QString::fromLatin1("\';")
        << StatementBuilder::NewLine;
    return sb;
}
//-----------------------------------------------------------------------------
const QString Function::getTypeName() const
{
    return QString::fromLatin1("FUNCTION");
}
//-----------------------------------------------------------------------------
QString Function::getDropSqlStatement() const
{
    StatementBuilder sb;
    sb << kwDROP << ' ' << kwEXTERNAL << ' ' << kwFUNCTION << ' '
        << getQuotedName() << ';';
    return sb;
}
//-----------------------------------------------------------------------------
QString Function::getDefinition()
{
    ensurePropertiesLoaded();
    return definitionM;
}
//-----------------------------------------------------------------------------
QString Function::getLibraryName()
{
    ensurePropertiesLoaded();
    return libraryNameM;
}
//-----------------------------------------------------------------------------
QString Function::getEntryPoint()
{
    ensurePropertiesLoaded();
    return entryPointM;
}
//-----------------------------------------------------------------------------
void Function::loadProperties()
{
    setPropertiesLoaded(false);

    QString mechanismNames[] = { QString::fromLatin1("value"), QString::fromLatin1("reference"),
        QString::fromLatin1("descriptor"), QString::fromLatin1("blob descriptor"), QString::fromLatin1("scalar array"),
        QString::fromLatin1("null"), QString() };
    QString mechanismDDL[] = { QString::fromLatin1(" BY VALUE "), QString(),
        QString::fromLatin1(" BY DESCRIPTOR "), QString(), QString::fromLatin1(" BY SCALAR ARRAY "),
        QString::fromLatin1(" NULL "), QString() };

    bool first = true;
    QString retstr;
    definitionM = getName_() + QString::fromLatin1("(") + QLatin1Char('\n');
    paramListM = QString();

    DatabasePtr db = getDatabase();
    MetadataLoader* loader = db->getMetadataLoader();
    QTextCodec* converter = db->getCharsetConverter();
    MetadataLoaderTransaction tr(loader);

    IBPP::Statement& st1 = loader->getStatement(
        "SELECT f.RDB$RETURN_ARGUMENT, a.RDB$MECHANISM,"
        " a.RDB$ARGUMENT_POSITION, a.RDB$FIELD_TYPE, a.RDB$FIELD_SCALE,"
        " a.RDB$FIELD_LENGTH, a.RDB$FIELD_SUB_TYPE, a.RDB$FIELD_PRECISION,"
        " f.RDB$MODULE_NAME, f.RDB$ENTRYPOINT, c.RDB$CHARACTER_SET_NAME "
        " FROM RDB$FUNCTIONS f"
        " LEFT OUTER JOIN RDB$FUNCTION_ARGUMENTS a"
        " ON f.RDB$FUNCTION_NAME = a.RDB$FUNCTION_NAME"
        " LEFT OUTER JOIN RDB$CHARACTER_SETS c"
        " ON a.RDB$CHARACTER_SET_ID = c.RDB$CHARACTER_SET_ID"
        " WHERE f.RDB$FUNCTION_NAME = ? "
        " ORDER BY a.RDB$ARGUMENT_POSITION"
    );
    st1->Set(1, wx2std(getName_(), converter));
    st1->Execute();
    while (st1->Fetch())
    {
        short returnarg, mechanism, type, scale, length, subtype, precision,
            retpos;
        std::string libraryName, entryPoint, charset;
        st1->Get(1, returnarg);
        st1->Get(2, mechanism);
        st1->Get(3, retpos);
        st1->Get(4, type);
        st1->Get(5, scale);
        st1->Get(6, length);
        st1->Get(7, subtype);
        st1->Get(8, precision);
        st1->Get(9, libraryName);
        libraryNameM = helper::stripString(std2wx(libraryName, converter));
        st1->Get(10, entryPoint);
        entryPointM = helper::stripString(std2wx(entryPoint, converter));
        QString datatype = Domain::dataTypeToString(type, scale,
            precision, subtype, length);
        if (!st1->IsNull(11))
        {
            st1->Get(11, charset);
            QString chset(helper::stripString(std2wx(charset, converter)));
            if (db->getDatabaseCharset() != chset)
            {
                datatype += QString::fromLatin1(" ") + SqlTokenizer::getKeyword(kwCHARACTER)
                    + QString::fromLatin1(" ") + SqlTokenizer::getKeyword(kwSET)
                    + QString::fromLatin1(" ") + chset;
            }
        }
        if (type == 261)    // avoid subtype information for BLOB
            datatype = SqlTokenizer::getKeyword(kwBLOB);

        int mechIndex = (mechanism < 0 ? -mechanism : mechanism);
        if (mechIndex >= (sizeof(mechanismNames)/sizeof(QString)))
            mechIndex = (sizeof(mechanismNames)/sizeof(QString)) - 1;
        QString param = QString::fromLatin1("    ") + datatype + QString::fromLatin1(" ")
            + SqlTokenizer::getKeyword(kwBY) + QString::fromLatin1(" ")
            + mechanismNames[mechIndex];
        if (mechanism < 0)
            param += QString(QString::fromLatin1(" ")) + SqlTokenizer::getKeyword(kwFREE_IT);
        if (returnarg == retpos)    // output
        {
            retstr = param;
            retstrM = datatype + mechanismDDL[mechIndex];
            if (retpos != 0)
            {
                retstrM = SqlTokenizer::getKeyword(kwPARAMETER) + QString::fromLatin1(" ");
                retstrM += retpos;
                if (!paramListM.isEmpty())
                    paramListM += QString::fromLatin1(", ");
                paramListM += datatype + mechanismDDL[mechIndex];
            }
        }
        else
        {
            if (first)
                first = false;
            else
                definitionM += QString(QString::fromLatin1(",")) + QLatin1Char('\n');
            definitionM += param;
            if (!paramListM.isEmpty())
                paramListM += QString::fromLatin1(", ");
            paramListM += datatype + mechanismDDL[mechIndex];
        }
    }
    definitionM += QString(QLatin1Char('\n')) + QString::fromLatin1(")")
        + QLatin1Char('\n') + SqlTokenizer::getKeyword(kwRETURNS)
        + QString::fromLatin1(":") + QLatin1Char('\n') + retstr;

    setPropertiesLoaded(true);
}
//-----------------------------------------------------------------------------
void Function::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitFunction(*this);
}
//-----------------------------------------------------------------------------
// Functions collection
Functions::Functions(DatabasePtr database)
    : MetadataCollection<Function>(ntFunctions, database, _("Functions"))
{
}
//-----------------------------------------------------------------------------
void Functions::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitFunctions(*this);
}
//-----------------------------------------------------------------------------
void Functions::load(ProgressIndicator* progressIndicator)
{
    QString stmt = QString::fromLatin1(
			"select rdb$function_name from rdb$functions"
			" where (rdb$system_flag = 0 or rdb$system_flag is null)"
			" order by 1"
		);
    setItems(getDatabase()->loadIdentifiers(stmt, progressIndicator));
}
//-----------------------------------------------------------------------------
void Functions::loadChildren()
{
    load(0);
}
//-----------------------------------------------------------------------------
const QString Functions::getTypeName() const
{
    return QString::fromLatin1("FUNCTION_COLLECTION");
}
//-----------------------------------------------------------------------------

} // namespace fr
