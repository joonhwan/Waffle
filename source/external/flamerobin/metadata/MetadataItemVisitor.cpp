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


  $Id: MetadataItemVisitor.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
#include "metadata/MetadataItemVisitor.h"

namespace fr {

//-----------------------------------------------------------------------------
MetadataItemVisitor::MetadataItemVisitor()
{
};
//-----------------------------------------------------------------------------
MetadataItemVisitor::~MetadataItemVisitor()
{
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitColumn(Column&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitDatabase(Database&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitDomain(Domain&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitDomains(Domains&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitException(Exception&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitExceptions(Exceptions&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitForeignKey(ForeignKey&)
{
    defaultAction();
}
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitFunction(Function&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitFunctions(Functions&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitGenerator(Generator&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitGenerators(Generators&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitIndex(Index&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitParameter(Parameter&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitPrimaryKeyConstraint(PrimaryKeyConstraint&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitProcedure(Procedure&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitProcedures(Procedures&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitRelation(Relation&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitRole(Role&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitRoles(Roles&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitRoot(Root&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitServer(Server&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitSysDomains(SysDomains&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitSysRoles(SysRoles&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitSysTables(SysTables&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitTable(Table& table)
{
    visitRelation(*(Relation*)&table);
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitTables(Tables&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitTrigger(Trigger&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitTriggers(Triggers&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitUniqueConstraint(UniqueConstraint&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitView(View& view)
{
    visitRelation(*(Relation*)&view);
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitViews(Views&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------
void MetadataItemVisitor::visitMetadataItem(MetadataItem&)
{
    defaultAction();
};
//-----------------------------------------------------------------------------

} // namespace fr
