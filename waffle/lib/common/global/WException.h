#pragma once

class WException
{
public:
	WException(const QString& error, int code=0)
		: m_error(error)
		, m_code(code)
	{
	}
	const QString& error() const {
		return m_error;
	}
	int code() const {
		return m_code;
	}
protected:
	QString m_error;
	int m_code;
};

