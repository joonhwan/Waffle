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


  $Id: Index.h 2200 2012-01-20 08:31:01Z mghie $

*/

//-----------------------------------------------------------------------------
#ifndef FR_INDEX_H
#define FR_INDEX_H

#include <vector>

#include "metadata/metadataitem.h"

namespace fr {

//-----------------------------------------------------------------------------
class Index: public MetadataItem
{
public:
    // needs to be declared here as type is used in private section
    enum IndexType { itAscending, itDescending };

private:
    bool isSystemM;
    bool uniqueFlagM;
    bool activeM;
    IndexType indexTypeM;
    double statisticsM;
    std::vector<QString> segmentsM;
    QString expressionM;
public:
    Index(bool unique, bool active, bool ascending, double statistics,
        bool system, QString expression);

    virtual bool isSystem() const;
    bool isActive() const;
    bool isUnique() const;
    double getStatistics();
    QString getExpression() const;
    IndexType getIndexType();
    // Returns a list of index fields, or the expression source if
    // the index is an expression-based index.
    QString getFieldsAsString();
    std::vector<QString> *getSegments();

    virtual void acceptVisitor(MetadataItemVisitor* visitor);
};
//-----------------------------------------------------------------------------

}

#endif
