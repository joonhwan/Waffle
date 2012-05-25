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


  $Id: Index.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
#include <vector>

#include "metadata/Index.h"
#include "metadata/MetadataItemVisitor.h"

namespace fr {

//-----------------------------------------------------------------------------
Index::Index(bool unique, bool active, bool ascending, double statistics,
        bool system, QString expression)
    : MetadataItem(ntIndex), isSystemM(system), uniqueFlagM(unique),
        activeM(active), indexTypeM(ascending ? itAscending : itDescending),
        statisticsM(statistics), segmentsM(), expressionM(expression)
{
}
//-----------------------------------------------------------------------------
bool Index::isSystem() const
{
    return isSystemM;
}
//-----------------------------------------------------------------------------
bool Index::isActive() const
{
    return activeM;
}
//-----------------------------------------------------------------------------
bool Index::isUnique() const
{
    return uniqueFlagM;
}
//-----------------------------------------------------------------------------
double Index::getStatistics()
{
    return statisticsM;
}
//-----------------------------------------------------------------------------
std::vector<QString> *Index::getSegments()
{
    return &segmentsM;
}
//-----------------------------------------------------------------------------
QString Index::getFieldsAsString()
{
    if (!expressionM.isEmpty())
        return expressionM;
    else
    {
        QString retval;
        for (std::vector<QString>::iterator it = segmentsM.begin(); 
            it != segmentsM.end(); ++it)
        {
            if (!retval.isEmpty())
                retval += QString::fromLatin1(", ");
            retval += (*it);
        }
        return retval;
    }
}
//-----------------------------------------------------------------------------
Index::IndexType Index::getIndexType()
{
    return indexTypeM;
}
//-----------------------------------------------------------------------------
QString Index::getExpression() const
{
    return expressionM;
}
//-----------------------------------------------------------------------------
void Index::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitIndex(*this);
}
//-----------------------------------------------------------------------------

} // namespace fr
