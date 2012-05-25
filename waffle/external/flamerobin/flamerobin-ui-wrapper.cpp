#include "flamerobin-ui-wrapper.h"

namespace fr {

int showWarningDialog(const QString& primaryText,
					  const QString& secondaryText,
					  QMessageBox::StandardButtons buttons,
					  QMessageBox::StandardButtons defaultButton)
{
	return 0;
}

int showWarningDialog(const QString& primaryText,
					  const QString& secondaryText,
					  Config& config, const QString& configKey,
					  const QString& dontShowAgainText,
					  QMessageBox::StandardButtons buttons,
					  QMessageBox::StandardButtons defaultButton)
{
	return 0;
}

} // namespace fr
