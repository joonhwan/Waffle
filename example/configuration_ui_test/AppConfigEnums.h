#pragma once

#include "common/config/WSettingExchangeTraits.h"
#include <QObject>

class AppConfigEnums : public QObject
{
    Q_OBJECT
	Q_ENUMS(Priority)
public:
    enum Priority
	{
		High = 0,
		Middle,
		Low,
		VeryLow
	};
};

SETTINGS_ENUM_TRAIT(AppConfigEnums, Priority)

