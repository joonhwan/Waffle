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


  $Id: MetadataItemCreateStatementVisitor.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
//-----------------------------------------------------------------------------
#include "metadata/MetadataItemCreateStatementVisitor.h"
#include "sql/StatementBuilder.h"

namespace fr {

//-----------------------------------------------------------------------------
/*static*/
QString MetadataItemCreateStatementVisitor::getCreateDomainStatement()
{
    return  QString::fromLatin1(
			"CREATE DOMAIN domain_name\n"
            "AS datatype [CHARACTER SET charset]\n"
            "DEFAULT {literal | NULL | USER}\n"
            "[NOT NULL]\n"
            "[CHECK (dom_search_condition)]\n"
            "COLLATE collation;\n"
		);
}
//-----------------------------------------------------------------------------
/*static*/
QString MetadataItemCreateStatementVisitor::getCreateExceptionStatement()
{
    StatementBuilder sb;
    sb << kwCREATE << ' ' << kwEXCEPTION << QString::fromLatin1(" name 'exception message';")
        << StatementBuilder::NewLine;
    return sb;
}
//-----------------------------------------------------------------------------
/*static*/
QString MetadataItemCreateStatementVisitor::getCreateFunctionStatement()
{
    return QString::fromLatin1(
			"DECLARE EXTERNAL FUNCTION name [datatype | CSTRING (int) "
           "[, datatype | CSTRING (int) ...]]\n"
           "RETURNS {datatype [BY VALUE] | CSTRING (int)} [FREE_IT]\n"
           "ENTRY_POINT 'entryname'\n"
           "MODULE_NAME 'modulename';\n"
		);
}
//-----------------------------------------------------------------------------
/*static*/
QString MetadataItemCreateStatementVisitor::getCreateGeneratorStatement()
{
    StatementBuilder sb;
    sb << kwCREATE << ' ' << kwGENERATOR << QString::fromLatin1(" name;")
        << StatementBuilder::NewLine
        << kwSET << ' ' << kwGENERATOR << QString::fromLatin1(" name ") << kwTO
        << QString::fromLatin1(" value;") << StatementBuilder::NewLine;
    return sb;
}
//-----------------------------------------------------------------------------
/*static*/
QString MetadataItemCreateStatementVisitor::getCreateProcedureStatement()
{
    return QString::fromLatin1(
			"SET TERM ^ ;\n\n"
            "CREATE PROCEDURE name \n"
            " ( input_parameter_name < datatype>, ... ) \n"
            "RETURNS \n"
            " ( output_parameter_name < datatype>, ... )\n"
            "AS \n"
            "DECLARE VARIABLE variable_name < datatype>; \n"
            "BEGIN\n"
            "  /* write your code here */ \n"
            "END^\n\n"
            "SET TERM ; ^\n"
		);
}
//-----------------------------------------------------------------------------
/*static*/
QString MetadataItemCreateStatementVisitor::getCreateRoleStatement()
{
    return  QString::fromLatin1("CREATE ROLE role_name;\n");
}
//-----------------------------------------------------------------------------
/*static*/
QString MetadataItemCreateStatementVisitor::getCreateTableStatement()
{
    return QString::fromLatin1(
			"CREATE TABLE table_name\n"
			"(\n"
			"    column_name {< datatype> | COMPUTED BY (< expr>) | domain}\n"
			"        [DEFAULT { literal | NULL | USER}] [NOT NULL]\n"
			"    ...\n"
			"    CONSTRAINT constraint_name\n"
			"        PRIMARY KEY (column_list),\n"
			"        UNIQUE      (column_list),\n"
			"        FOREIGN KEY (column_list) REFERENCES other_table (column_list),\n"
			"        CHECK       (condition),\n"
			"    ...\n"
			");\n"
		);
}
//-----------------------------------------------------------------------------
/*static*/
QString MetadataItemCreateStatementVisitor::getCreateTriggerStatement()
{
    return QString::fromLatin1(
			"SET TERM ^ ;\n\n"
			"CREATE TRIGGER name [FOR table/view] \n"
			" [IN]ACTIVE \n"
			" [ON {[DIS]CONNECT | TRANSACTION {START | COMMIT | ROLLBACK}} ] \n"
			" [{BEFORE | AFTER} INSERT OR UPDATE OR DELETE] \n"
			" POSITION number \n"
			"AS \n"
			"BEGIN \n"
			"    /* enter trigger code here */ \n"
			"END^\n\n"
			"SET TERM ; ^\n"
		);
}
//-----------------------------------------------------------------------------
/*static*/
QString MetadataItemCreateStatementVisitor::getCreateViewStatement()
{
    StatementBuilder sb;
    sb << kwCREATE << ' ' << kwVIEW << QString::fromLatin1(" name ( view_column, ...)")
        << StatementBuilder::NewLine << kwAS << StatementBuilder::NewLine
        << QString::fromLatin1("/* write select statement here */")
        << StatementBuilder::NewLine
        << kwWITH << ' ' << kwCHECK << ' ' << kwOPTION << ';'
        << StatementBuilder::NewLine;
    return sb;
}
//-----------------------------------------------------------------------------
void MetadataItemCreateStatementVisitor::visitDomains(Domains& /*domains*/)
{
    statementM = getCreateDomainStatement();
}
//-----------------------------------------------------------------------------
void MetadataItemCreateStatementVisitor::visitExceptions(
    Exceptions& /*exceptions*/)
{
    statementM = getCreateExceptionStatement();
}
//-----------------------------------------------------------------------------
void MetadataItemCreateStatementVisitor::visitFunctions(
    Functions& /*functions*/)
{
    statementM = getCreateFunctionStatement();
}
//-----------------------------------------------------------------------------
void MetadataItemCreateStatementVisitor::visitGenerators(
    Generators& /*generators*/)
{
    statementM = getCreateGeneratorStatement();
}
//-----------------------------------------------------------------------------
void MetadataItemCreateStatementVisitor::visitProcedures(
    Procedures& /*procedures*/)
{
    statementM = getCreateProcedureStatement();
}
//-----------------------------------------------------------------------------
void MetadataItemCreateStatementVisitor::visitRoles(Roles& /*roles*/)
{
    statementM = getCreateRoleStatement();
}
//-----------------------------------------------------------------------------
void MetadataItemCreateStatementVisitor::visitTables(Tables& /*tables*/)
{
    statementM = getCreateTableStatement();
}
//-----------------------------------------------------------------------------
void MetadataItemCreateStatementVisitor::visitTriggers(Triggers& /*triggers*/)
{
    statementM = getCreateTriggerStatement();
}
//-----------------------------------------------------------------------------
void MetadataItemCreateStatementVisitor::visitViews(Views& /*views*/)
{
    statementM = getCreateViewStatement();
}
//-----------------------------------------------------------------------------
QString MetadataItemCreateStatementVisitor::getStatement() const
{
    return statementM;
}
//-----------------------------------------------------------------------------

} // namespace fr
