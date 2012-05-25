#pragma once

#include <QString>

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

template<>
struct WExchangeTrait<double>
{
	typedef WSingleValueSettingTag ExchangeCategory;
};

template<>
struct WExchangeTrait<int>
{
	typedef WSingleValueSettingTag ExchangeCategory;
};

template<>
struct WExchangeTrait<bool>
{
	typedef WSingleValueSettingTag ExchangeCategory;
};

template<>
struct WExchangeTrait<const char*>
{
	typedef WSingleValueSettingTag ExchangeCategory;
};

template<>
struct WExchangeTrait<QString>
{
	typedef WSingleValueSettingTag ExchangeCategory;
};
