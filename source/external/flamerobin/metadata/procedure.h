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


  $Id: procedure.h 2200 2012-01-20 08:31:01Z mghie $

*/

#ifndef FR_PROCEDURE_H
#define FR_PROCEDURE_H

#include <vector>

#include "metadata/collection.h"
#include "metadata/privilege.h"

namespace fr {

class ProgressIndicator;
//-----------------------------------------------------------------------------
class Procedure : public MetadataItem
{
private:
    std::vector<Privilege> privilegesM;
    ParameterPtrs parametersM;
protected:
    virtual void loadChildren();
    virtual void lockChildren();
    virtual void unlockChildren();
public:
    Procedure(DatabasePtr database, const QString& name);

    bool getChildren(std::vector<MetadataItem *>& temp);

    ParameterPtrs::iterator begin();
    ParameterPtrs::iterator end();
    ParameterPtrs::const_iterator begin() const;
    ParameterPtrs::const_iterator end() const;

    size_t getParamCount() const;
    ParameterPtr findParameter(const QString& name) const;

    QString getOwner();
    QString getSource();
    QString getAlterSql(bool full = true);
    QString getDefinition();   // used for calltip in sql editor
    std::vector<Privilege>* getPrivileges();

    void checkDependentProcedures();

    virtual const QString getTypeName() const;
    virtual void acceptVisitor(MetadataItemVisitor* visitor);
};
//-----------------------------------------------------------------------------
class Procedures: public MetadataCollection<Procedure>
{
protected:
    virtual void loadChildren();
public:
    Procedures(DatabasePtr database);

    virtual void acceptVisitor(MetadataItemVisitor* visitor);
    void load(ProgressIndicator* progressIndicator);
    virtual const QString getTypeName() const;
};
//-----------------------------------------------------------------------------

} // namespace fr

#endif
