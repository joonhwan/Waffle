#pragma once

#include <QString>
#include <QDate>
#include <QDateTime>

// tags list for dispatching.
struct WSingleValueSettingTag { };
struct WArraySettingTag { };
struct WEnumSettingTag { };
struct WGroupSettingTag { };

template<typename T>
struct WEnumMetaObjectOf;

#define SETTINGS_ENUM_TRAIT(CLASSNAME, ENUMNAME)	\
	template<>										\
	struct WExchangeTrait<CLASSNAME::ENUMNAME>		\
	{												\
		typedef WEnumSettingTag ExchangeCategory;	\
	};												\
	template<>										\
	struct WEnumMetaObjectOf<CLASSNAME::ENUMNAME>	\
	{												\
		typedef CLASSNAME Type;						\
		static const char* enumName()				\
		{											\
			return #ENUMNAME;						\
		}											\
	};												\

template<typename T>
struct WExchangeTrait
{
	typedef typename T::ExchangeCategory ExchangeCategory;
};

#define W_SINGLE_VALUE_SETTING(TYPE)						\
	template<>												\
	struct WExchangeTrait<TYPE>								\
	{														\
		typedef WSingleValueSettingTag ExchangeCategory;	\
	};														\

W_SINGLE_VALUE_SETTING(double);
W_SINGLE_VALUE_SETTING(int);
W_SINGLE_VALUE_SETTING(bool);
W_SINGLE_VALUE_SETTING(char*);
W_SINGLE_VALUE_SETTING(QString);
W_SINGLE_VALUE_SETTING(QDate);
W_SINGLE_VALUE_SETTING(QDateTime);
