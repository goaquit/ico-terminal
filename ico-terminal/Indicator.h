#pragma once
#include <QObject>
#include <QList>
#include <QtProperty>
#include <QtVariantPropertyManager>

namespace ChartCore
{
	class Indicator : public QObject
	{
		Q_OBJECT
	public:
		Indicator(QObject * parent = nullptr);
		~Indicator();

		void setTitle(QString title);

		// Add property to indicator and return property manager for connect signal/slot
		QtVariantPropertyManager *addProperty(QString title, int valueType, QVariant value);

		QtProperty * getIndicator() const;
		QList<QtVariantPropertyManager *> getManagerList() const;

	private:
		QtProperty * indicator;
		QList<QtVariantPropertyManager *> managerList;
	};
}