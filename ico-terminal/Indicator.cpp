#include "Indicator.h"

#include <QtVariantProperty>
#include <memory>

namespace ChartCore
{
	Indicator::Indicator(QObject * parent): QObject(parent)
	{
		auto manager = new QtVariantPropertyManager(this);
		managerList.push_back(manager);
		indicator = manager->addProperty(QtVariantPropertyManager::groupTypeId());
	}

	Indicator::~Indicator()
	{
		for (auto manager : managerList)
		{
			manager->deleteLater();
		}

		managerList.clear();

		delete indicator;
	}

	void Indicator::setTitle(QString title)
	{
		indicator->setPropertyName(title);
	}

	QtVariantPropertyManager * Indicator::addProperty(QString title, int valueType, QVariant value)
	{
		auto manager = new QtVariantPropertyManager(this);
		managerList.push_back(manager);

		// с данным поинтом не работает
		//std::shared_ptr<QtVariantProperty> property(manager->addProperty(valueType, title));
		//todo устранить утечку
		auto property(manager->addProperty(valueType, title));

		property->setValue(value);

		indicator->addSubProperty(property);

		return manager;
	}

	QtProperty * Indicator::getIndicator() const
	{
		return indicator;
	}

	QList<QtVariantPropertyManager *> Indicator::getManagerList() const
	{
		return managerList;
	}
}