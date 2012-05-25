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


  $Id: frutils.h 2200 2012-01-20 08:31:01Z mghie $

*/

#ifndef FRUTILS_H
#define FRUTILS_H
//-----------------------------------------------------------------------------
#include <list>

#include "flamerobin/ibpp/ibpp.h"
#include "metadata/MetadataClasses.h"

namespace fr {

class ProgressDialog;
class ProgressIndicator;
// //-----------------------------------------------------------------------------
// //! sets all controls to width of widest control
// void adjustControlsMinWidth(std::list<wxWindow*> controls);
//-----------------------------------------------------------------------------
//! reads blob from statement into QString
void readBlob(IBPP::Statement& st, int column, QString& result,
			  QTextCodec* conv);
//-----------------------------------------------------------------------------
// //! displays a list of table columns and lets user select some
// bool selectRelationColumnsIntoVector(Relation* t, wxWindow* parent,
//     std::vector<QString>& list);
// //-----------------------------------------------------------------------------
// //! prompts for password if needed and connects to database
// bool connectDatabase(Database *db, wxWindow* parent,
//     ProgressDialog* progressdialog = 0);
//-----------------------------------------------------------------------------
bool getService(Server* s, IBPP::Service& svc, ProgressIndicator* p,
    bool sysdba);
//-----------------------------------------------------------------------------

} // namespace fr

#endif // FRUTILS_H
