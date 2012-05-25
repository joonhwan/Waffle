#pragma once

#include <QString>
#include <QMessageBox>

namespace fr {

class Config;

// int showWarningDialog(wxWindow* parent, const wxString& primaryText,
//     const wxString& secondaryText, AdvancedMessageDialogButtons buttons);
int showWarningDialog(const QString& primaryText,
					  const QString& secondaryText,
					  QMessageBox::StandardButtons buttons = QMessageBox::Ok,
					  QMessageBox::StandardButtons defaultButton = QMessageBox::NoButton);


// int showWarningDialog(wxWindow* parent, const wxString& primaryText,
//     const wxString& secondaryText, AdvancedMessageDialogButtons buttons,
//     Config& config, const wxString& configKey,
//     const wxString& dontShowAgainText);
int showWarningDialog(const QString& primaryText,
					  const QString& secondaryText,
					  Config& config, const QString& configKey,
					  const QString& dontShowAgainText,
					  QMessageBox::StandardButtons buttons = QMessageBox::Ok,
					  QMessageBox::StandardButtons defaultButton = QMessageBox::NoButton);

} // namespace fr
