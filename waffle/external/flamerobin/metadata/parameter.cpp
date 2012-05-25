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


  $Id: parameter.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
#include "config/Config.h"
#include "metadata/database.h"
#include "metadata/domain.h"
#include "metadata/MetadataItemVisitor.h"
#include "metadata/parameter.h"
#include "metadata/procedure.h"

namespace fr {

//-----------------------------------------------------------------------------
// TODO: pass ProcedurePtr instead of Procedure*
Parameter::Parameter(Procedure* procedure, const QString& name)
    : ColumnBase(ntParameter, procedure, name), outputParameterM(false),
        parameterMechanismM(-1)
{
}
//-----------------------------------------------------------------------------
void Parameter::initialize(const QString& source, int parameterType,
    int mechanism, const QString& defaultValue, bool hasDefault)
{
    SubjectLocker lock(this);

    ColumnBase::initialize(source, defaultValue, hasDefault);

    bool changed = false;
    if (parameterMechanismM != mechanism)
    {
        parameterMechanismM = mechanism;
        changed= true;
    }
    bool outputParam = parameterType != 0;
    if (outputParameterM != outputParam)
    {
        outputParameterM = outputParam;
        changed= true;
    }
    if (changed)
        notifyObservers();
}
//-----------------------------------------------------------------------------
bool Parameter::isOutputParameter() const
{
    return outputParameterM;
}
//-----------------------------------------------------------------------------
int Parameter::getMechanism() const
{
    return parameterMechanismM;
}
//-----------------------------------------------------------------------------
const QString Parameter::getTypeName() const
{
    return QString::fromLatin1("PARAMETER");
}
//-----------------------------------------------------------------------------
void Parameter::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitParameter(*this);
}
//-----------------------------------------------------------------------------

} // namespace fr
