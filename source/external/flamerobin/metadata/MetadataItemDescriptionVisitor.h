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


  $Id: MetadataItemDescriptionVisitor.h 2200 2012-01-20 08:31:01Z mghie $

*/

#ifndef FR_DESCRIPTIONVISITOR_H
#define FR_DESCRIPTIONVISITOR_H

#include "metadata/MetadataClasses.h"
#include "metadata/MetadataItemVisitor.h"

namespace fr {

//-----------------------------------------------------------------------------
class LoadDescriptionVisitor : public MetadataItemVisitor
{
private:
    QString descriptionM;
    bool availableM;
    void loadDescription(MetadataItem* object, const std::string& statement);
    void loadDescription(MetadataItem* object, MetadataItem* parent,
        const std::string& statement);
public:
    LoadDescriptionVisitor();

    virtual void visitColumn(Column& column);
    virtual void visitDomain(Domain& domain);
    virtual void visitException(Exception& exception);
    virtual void visitFunction(Function& function);
    virtual void visitGenerator(Generator& generator);
    virtual void visitIndex(Index& index);
    virtual void visitParameter(Parameter& parameter);
    virtual void visitProcedure(Procedure& procedure);
    virtual void visitRelation(Relation& relation);
    virtual void visitRole(Role& role);
    virtual void visitTrigger(Trigger& trigger);

    bool descriptionAvailable() const;
    QString getDescription() const;
};
//-----------------------------------------------------------------------------
class SaveDescriptionVisitor : public MetadataItemVisitor
{
private:
    QString descriptionM;
    void saveDescription(MetadataItem* object, const std::string& statement);
    void saveDescription(MetadataItem* object, MetadataItem* parent,
        const std::string& statement);
public:
    SaveDescriptionVisitor(QString description);

    virtual void visitColumn(Column& column);
    virtual void visitDomain(Domain& domain);
    virtual void visitException(Exception& exception);
    virtual void visitFunction(Function& function);
    virtual void visitGenerator(Generator& generator);
    virtual void visitIndex(Index& index);
    virtual void visitParameter(Parameter& parameter);
    virtual void visitProcedure(Procedure& procedure);
    virtual void visitRelation(Relation& relation);
    virtual void visitRole(Role& role);
    virtual void visitTrigger(Trigger& trigger);
};
//-----------------------------------------------------------------------------

} // namespace fr

#endif // FR_DESCRIPTIONVISITOR_H
