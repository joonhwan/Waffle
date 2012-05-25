#pragma once

class WSettingExchangerValidator
{
public:
    WSettingExchangerValidator();
    virtual ~WSettingExchangerValidator();
	void beginValidate();
	bool isValid() const;
	QString validityCheckString() const;
	WSettingExchangerValidator& operator << (const QString& log);
protected:
	bool m_valid;
	QString m_validityCheckString;
};
