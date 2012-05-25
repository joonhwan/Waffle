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


  $Id: frutils.cpp 2200 2012-01-20 08:31:01Z mghie $

*/

// For compilers that support precompilation, includes "wx/wx.h".
#include "core/StringUtils.h"
#include "frutils.h"
#include "core/ProgressIndicator.h"
// #include "gui/ProgressDialog.h"
// #include "gui/UsernamePasswordDialog.h"
#include "metadata/column.h"
#include "metadata/relation.h"
#include "metadata/server.h"

namespace fr {

// //-----------------------------------------------------------------------------
// void adjustControlsMinWidth(std::list<wxWindow*> controls)
// {
//     int w = 0;
//     wxSize sz;
//     // find widest control
//     for (std::list<wxWindow*>::iterator it = controls.begin();
//         it != controls.end(); ++it)
//     {
//         wxASSERT(*it != 0);
//         sz = (*it)->GetSize();
//         w = std::max(w, sz.GetWidth());
//     }
//     // set minimum width of all controls
//     for (std::list<wxWindow*>::iterator it = controls.begin();
//         it != controls.end(); ++it)
//     {
//         sz = (*it)->GetSize();
//         (*it)->SetSize(w, sz.GetHeight());
//         (*it)->SetSizeHints(w, sz.GetHeight());
//     }
// }
//-----------------------------------------------------------------------------
void readBlob(IBPP::Statement& st, int column, QString& result,
    QTextCodec* conv)
{
    result = QString::fromLatin1("");
    if (st->IsNull(column))
        return;

    IBPP::Blob b = IBPP::BlobFactory(st->DatabasePtr(), st->TransactionPtr());
    st->Get(column, b);

    try              // if blob is empty the exception is thrown
    {                // I tried to check st1->IsNull(1) but it doesn't work
        b->Open();   // to this hack is the only way (for the time being)
    }
    catch (...)
    {
        return;
    }

    std::string resultBuffer;
    char readBuffer[8192];        // 8K block
    while (true)
    {
        int size = b->Read(readBuffer, 8192);
        if (size <= 0)
            break;
        readBuffer[size] = 0;
        resultBuffer += readBuffer;
    }
    result = std2wx(resultBuffer, conv);
    b->Close();
}
//-----------------------------------------------------------------------------
// QString selectRelationColumns(Relation* t, wxWindow* parent)
// {
//     std::vector<QString> list;
//     if (!selectRelationColumnsIntoVector(t, parent, list))
//         return wxEmptyString;

//     std::vector<QString>::iterator it = list.begin();
//     QString retval(*it);
//     while ((++it) != list.end())
//         retval += QString::fromLatin1String(", ") + (*it);
//     return retval;
// }
// //-----------------------------------------------------------------------------
// bool selectRelationColumnsIntoVector(Relation* t, wxWindow* parent,
//     std::vector<QString>& list)
// {
//     t->ensureChildrenLoaded();

//     wxArrayInt selected_columns;
//     wxArrayString colNames;
//     colNames.Alloc(t->getColumnCount());
//     for (ColumnPtrs::const_iterator it = t->begin(); it != t->end(); ++it)
//         colNames.Add((*it)->getName_());

//     // set default selection.
//     for (std::vector<QString>::const_iterator it = list.begin();
//         it != list.end(); ++it)
//     {
//         QString::size_type i = colNames.Index((*it));
//         if (i != wxNOT_FOUND)
//             selected_columns.Add(i);
//     }    
    
//     bool ok = 
// #if wxCHECK_VERSION(2, 9, 0)
//     ::wxGetSelectedChoices(selected_columns,
// #else
//     ::wxGetMultipleChoices(selected_columns,
// #endif
//         _("Select one or more fields... (use ctrl key)"),  _("Table Fields"),
//         colNames, parent) > 0;
//     list.clear();
//     if (!ok)
//         return false;

//     for (size_t i = 0; i < selected_columns.GetCount(); ++i)
//     {
//         Identifier temp(colNames[selected_columns[i]]);
//         list.push_back(temp.getQuoted());
//     }
//     return true;
// }
// //-----------------------------------------------------------------------------
// bool connectDatabase(Database* db, wxWindow* parent,
//     ProgressDialog* progressdialog)
// {
//     QString pass(db->getDecryptedPassword());
//     if (db->getAuthenticationMode().getAlwaysAskForPassword())
//     {
//         // don't allow different username here
//         UsernamePasswordDialog upd(wxGetActiveWindow(),
//             _("Database Credentials"), db->getUsername(), true,
//             _("Please enter the the database user's password:"));
//         if (upd.ShowModal() != wxID_OK)
//             return false;
//         pass = upd.getPassword();
//     }

//     QString caption(QString::Format(_("Connecting to Database \"%s\""),
//         db->getName_().c_str()));
//     if (progressdialog)
//     {
//         progressdialog->setProgressMessage(caption);
//         db->connect(pass, progressdialog);
//     }
//     else
//     {
//         ProgressDialog pd(parent, caption, 1);
//         db->connect(pass, &pd);;
//     }
//     return true;
// }
// //-----------------------------------------------------------------------------
bool getService(Server* s, IBPP::Service& svc, ProgressIndicator* p,
    bool sysdba)
{
    if (!s->getService(svc, p, sysdba))
    {
		QString username = "SYSDBA";
		QString password = "masterky";
#ifdef TODO_IMPLEMENT_ME
        QString msg;
		QTextStream s(&msg);
        if (sysdba) {
            s << QObject::tr("Please enter the database administrator password:");
		} else {
            if (p && p->isCanceled()) {
                s << QObject::tr("You have canceled the search for a usable username and password.");
			} else {
				s << QObject::tr("None of the known database credentials could be used.");
			}
            s << QString::fromLatin1String("\n")
			  << QObject::tr("Please enter a valid username and password:");
        }
		s.flush();

        UsernamePasswordDialog upd(wxGetActiveWindow(),
            _("Database Credentials"), QString::fromLatin1("SYSDBA"), sysdba, msg);
        if (upd.ShowModal() != wxID_OK)
            return false;
        username = upd.getUsername();
        password = upd.getPassword();
#endif
        try
        {
            svc = IBPP::ServiceFactory(wx2std(s->getConnectionString()),
                wx2std(username), wx2std(password));
            svc->Connect();
            // exception might be thrown. If not, we store the credentials:
            if (sysdba || username.toUpper() == QString::fromLatin1("SYSDBA"))
                s->setServiceSysdbaPassword(password);
            else
                s->setServiceCredentials(username, password);
        }
        catch(IBPP::Exception& e)
        {
			Q_UNUSED(e);
            // wxMessageBox(std2wx(e.ErrorMessage()), _("Error"),
            //     wxICON_ERROR | wxOK);
            return false;
        }
    }
    return true;
}
// //-----------------------------------------------------------------------------

} // namespace fr
