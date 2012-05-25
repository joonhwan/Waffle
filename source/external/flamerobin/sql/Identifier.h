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


  $Id: Identifier.h 2200 2012-01-20 08:31:01Z mghie $

*/
#ifndef FR_IDENTIFIER_H
#define FR_IDENTIFIER_H

#include <QString>

namespace fr {

//----------------------------------------------------------------------------
//! The purpose of this class is to abstract all the work with identifiers
//! so that we don't have to struggle with quoted identifiers all over the
//! place. If also makes matching easier (upper/lower case problems)
class Identifier
{
private:
    QString textM;
    static bool needsQuoting(const QString& s);
    static bool isQuoted(const QString &s);
    static QString& escape(QString& s);
    static QString& strip(QString& s);
    static QString& quote(QString &s);
public:
    Identifier();
    Identifier(const QString& source);
    void setText(const QString& source);
    void setFromSql(const QString& source);

    bool equals(const Identifier& rhs) const;
    bool equals(const QString& rhs) const;
    QString get() const;
    QString getQuoted() const;
    static QString userString(const QString& s);
};
//----------------------------------------------------------------------------

}; // namespace fr
#endif
