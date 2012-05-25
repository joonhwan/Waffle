#pragma once

#include "common/config/WSettingExchanger.h"
#include "common/config/WSettingExchangeable.h"
#include "common/config/WSettingExchangeTraits.h"
#include "common/config/WSettingsHelper.h"
#include <QList>
#include <QSettings>

class WSettingExchanger;

template<typename T>
class WSettingConfigArray : public QList<T>
						  , public WSettingExchangeable
{
public:
	typedef WArraySettingTag ExchangeCategory;

	WSettingConfigArray()
	{
	}
	WSettingConfigArray(QSettings& _s)
	{
		WSettingExchanger e(_s, true);
		exchange(e);
	}
	void exchange(WSettingExchanger& e, const QString& key, const WSettingConfigArray<T>& defaultValue)
	{
		QString keyTrailed = key + QLatin1Char('/');
		if (e.isLoading() && !e.accessor().hasKeyStartsWith(keyTrailed)) {
			*this = defaultValue;
		} else {
			e.accessor().beginGroup(key);
			exchange(e);
			e.accessor().endGroup();
		}
	}
	virtual void exchange(WSettingExchanger& e)
	{
		if (e.isLoading()) {
			int count = e.accessor().beginArray("items");
			this->clear();
			for (int i = 0; i < count; ++i) {
				e.accessor().beginArrayIndex(i);
				T v;
				v.exchange(e);
				(*this) << v;
				e.accessor().endArrayIndex();
			}
		} else {
			e.accessor().beginArray("items");
			for (int i = 0; i < this->size(); ++i) {
				e.accessor().beginArrayIndex(i);
				(*this)[i].exchange(e);
				e.accessor().endArrayIndex();
			}
		}
		e.accessor().endArray();
	}
};
