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


  $Id: SelectStatement.h 2200 2012-01-20 08:31:01Z mghie $

*/
//-----------------------------------------------------------------------------
#ifndef FR_SELECT_STATEMENT_H
#define FR_SELECT_STATEMENT_H

#include <vector>

#include "sql/SqlTokenizer.h"

namespace fr {

//-----------------------------------------------------------------------------
//! Should provide a way to:
//! - parse the user supplied SELECT statement into components
//! - add/remove tables and columns to it
//! - build a statement from scratch (by ADDing to blank statement)
class SelectStatement
{
private:
    QString sqlM;
    SqlTokenizer tokenizerM;
    int posSelectM, posFromM, posFromEndM;
    void add(const QString& toAdd, int position);

public:
    SelectStatement(const QString& sql);

    bool isValidSelectStatement();  // needs to have SELECT and FROM at least
    void setStatement(const QString& sql);
    QString getStatement();

    void getTables(std::vector<QString>& tables);
    void getColumns(std::vector<QString>& columns);

    void addTable(const QString& name, const QString& joinType,
        const QString& joinList);
    void addColumn(const QString& columnList); // adds as-is currently
    
    void orderBy(int column);
};
//-----------------------------------------------------------------------------

}

#endif
