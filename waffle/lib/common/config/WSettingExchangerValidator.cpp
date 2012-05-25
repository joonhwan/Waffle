#include "WSettingExchangerValidator.h"

WSettingExchangerValidator::WSettingExchangerValidator()
{
	beginValidate();
}

//virtual
WSettingExchangerValidator::~WSettingExchangerValidator()
{
}

void WSettingExchangerValidator::beginValidate()
{
	m_valid = true;
	m_validityCheckString.clear();
	m_stream.setString(&m_validityCheckString, QIODevice::WriteOnly);
}

bool WSettingExchangerValidator::isValid() const
{
	return m_valid;
}

QString WSettingExchangerValidator::validityCheckString() const
{
	m_stream.flush();
	return m_validityCheckString;
}

WSettingExchangerValidator&
WSettingExchangerValidator::operator << (const QString& log)
{
	m_valid = false;

	m_stream << log << QLatin1Char('\n');
	return *this;
}

