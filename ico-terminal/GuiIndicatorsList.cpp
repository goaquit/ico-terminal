#include <QAbstractButton>
#include "AbstractIndicator.h"
#include "GuiIndicatorsList.h"
#include <QList>
#include <QtProperty>
#include <QtVariantPropertyManager>
#include <QtVariantEditorFactory>

#include "IndicatorBollinger.h"
#include "IndicatorDx.h"
#include "IndicatorParabollicSar.h"
#include "IndicatorSma.h"
#include "IndicatorStochastic.h"

using namespace ChartCore;
using namespace Indicators;

namespace Gui
{


GuiIndicatorsList::GuiIndicatorsList(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowTitle("Indicators list");
	setAttribute(Qt::WA_DeleteOnClose);

	ui.treeWidget->setColumnCount(1);

	connect(ui.buttonBox, &QDialogButtonBox::clicked, this, &GuiIndicatorsList::onButtonBoxClicked);
	connect(ui.removeButton, &QPushButton::released, this, &Gui::GuiIndicatorsList::onRemove);
}

GuiIndicatorsList::~GuiIndicatorsList()
{
	for (auto i : temporary)
	{
		delete i;
	}
}

void GuiIndicatorsList::setIndicators(const std::vector<Indicators::AbstractIndicator*> &arg)
{
	indicators = arg;
	uint counter = 0;
	for (const auto indicator : indicators)
	{
		auto item = new QTreeWidgetItem(ui.treeWidget);
		item->setText(0, indicator->getAlias());
		item->setData(0, Qt::UserRole + QVariant::UInt, counter);

		ui.treeWidget->addTopLevelItem(item);		

		counter++;
	}

	connect(ui.treeWidget, &QTreeWidget::itemClicked, this, &GuiIndicatorsList::onSelectedIndicator);

}

void GuiIndicatorsList::onSelectedIndicator(QTreeWidgetItem * item, int column)
{
	if (column != 0) return;
	
	if(browser)	browser->deleteLater();

	const auto index = item->data(0, Qt::UserRole + QVariant::UInt).toUInt();
	auto indicator = indicators.at(index);

	browser = new QtTreePropertyBrowser(this);
	ui.layoutProp->addWidget(browser);

	auto factory = new QtVariantEditorFactory(browser);
	auto manager = new QtVariantPropertyManager(browser);

	auto propGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId());
	propGroup->setPropertyName("General");
	auto prop = manager->addProperty(QVariant::String, "alias");
	propGroup->addSubProperty(prop);
	prop->setValue(indicator->getAlias());

	connect(manager, &QtVariantPropertyManager::valueChanged, 
		[indicator](QtProperty *p, QVariant  value)
		{
			Q_UNUSED(p);

			indicator->setAlias(value.toString());
		});

	browser->addProperty(propGroup);
	browser->setFactoryForManager(manager, factory);

	switch (indicator->getType())
	{
	case AbstractIndicator::Adx: setAdxProperty(static_cast<IndicatorDx*>(indicator)); break;
	case AbstractIndicator::Bollinger: setBollingerProperty(static_cast<IndicatorBollinger*>(indicator)); break;
	case AbstractIndicator::Sar: setSarProperty(static_cast<IndicatorParabollicSar*>(indicator)); break;
	case AbstractIndicator::Sma: setSmaProperty(static_cast<IndicatorSma*>(indicator)); break;
	case AbstractIndicator::Stochastic: setStochasticProperty(static_cast<IndicatorStochastic*>(indicator)); break;
	}

	browser->show();
}

void GuiIndicatorsList::onButtonBoxClicked(QAbstractButton * button)
{
	switch (ui.buttonBox->standardButton(button))
	{
		case QDialogButtonBox::Close: close(); break;
	}
}

void GuiIndicatorsList::setAdxProperty(Indicators::IndicatorDx *indicator)
{
	auto factory = new QtVariantEditorFactory(browser);
	auto manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);

	auto group = manager->addProperty(QtVariantPropertyManager::groupTypeId());
	group->setPropertyName("Average Directional Index");
	auto prop = manager->addProperty(QVariant::Int, "period");
	prop->setValue(indicator->getPeriod());
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator, this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changePeriod((value.toInt() > 0) ? value.toInt() : 0);
		emit writeIndicator(indicator);
	});

	browser->addProperty(group);
	
	///////////////////////////////////////////////////
	// ADX
	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);

	group = manager->addProperty(QtVariantPropertyManager::groupTypeId());
	group->setPropertyName("ADX");

	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	prop = manager->addProperty(QVariant::Color, "color");
	prop->setValue(indicator->getAdxColor());
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator,this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changeAdxColor(value.value<QColor>());
		emit writeIndicator(indicator);
	});

	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	prop = manager->addProperty(QVariant::Int, "size");
	prop->setValue(indicator->getAdxSize());
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator,this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changeAdxSize((value.toInt() > 0) ? value.toInt() : 0);
		emit writeIndicator(indicator);
	});

	browser->addProperty(group);

	/////////////////////////////////////////////////////
	// +DM
	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);

	group = manager->addProperty(QtVariantPropertyManager::groupTypeId());
	group->setPropertyName("+DM");

	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	prop = manager->addProperty(QVariant::Color, "color");
	prop->setValue(indicator->getPdiColor());
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator, this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changePdiColor(value.value<QColor>());
		emit writeIndicator(indicator);
	});

	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	prop = manager->addProperty(QVariant::Int, "size");
	prop->setValue(indicator->getPdiSize());
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator, this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changePdiSize((value.toInt() > 0) ? value.toInt() : 0);
		emit writeIndicator(indicator);
	});

	browser->addProperty(group);

	//////////////////////////////////////////////////////////
	// -DM
	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);

	group = manager->addProperty(QtVariantPropertyManager::groupTypeId());
	group->setPropertyName("-DM");

	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	prop = manager->addProperty(QVariant::Color, "color");
	prop->setValue(indicator->getNdiColor());
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator,this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changeNdiColor(value.value<QColor>());
		emit writeIndicator(indicator);
	});

	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	prop = manager->addProperty(QVariant::Int, "size");
	prop->setValue(indicator->getNdiSize());
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator, this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changeNdiSize((value.toInt() > 0) ? value.toInt() : 0);
		emit writeIndicator(indicator);
	});

	browser->addProperty(group);
}

void GuiIndicatorsList::setBollingerProperty(Indicators::IndicatorBollinger *indicator)
{
	auto factory = new QtVariantEditorFactory(browser);
	auto manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);

	auto group = manager->addProperty(QtVariantPropertyManager::groupTypeId());
	group->setPropertyName("Bollinger bands");

	// period
	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	auto prop = manager->addProperty(QVariant::Int, "period");
	prop->setValue(indicator->getPeriod());
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator, this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changePeriod((value.toInt() > 0) ? value.toInt() : 0);
		emit writeIndicator(indicator);
	});

	// multiplier
	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	prop = manager->addProperty(QVariant::Int, "multiplier");
	prop->setValue(indicator->getMultiplier());
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator, this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changeMultiplier((value.toInt() > 0) ? value.toInt() : 0);
		emit writeIndicator(indicator);
	});

	browser->addProperty(group);

	///////////////////////////////////////////////////
	// Series
	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);

	group = manager->addProperty(QtVariantPropertyManager::groupTypeId());
	group->setPropertyName("Series");

	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	prop = manager->addProperty(QVariant::Color, "color");
	prop->setValue(indicator->getColor());
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator, this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changeColor(value.value<QColor>());
		emit writeIndicator(indicator);
	});

	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	prop = manager->addProperty(QVariant::Int, "size");
	prop->setValue(indicator->getSize());
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator, this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changeSize((value.toInt() > 0) ? value.toInt() : 0);
		emit writeIndicator(indicator);
	});

	browser->addProperty(group);
}

void GuiIndicatorsList::setSarProperty(Indicators::IndicatorParabollicSar *indicator)
{
	auto factory = new QtVariantEditorFactory(browser);
	auto manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);

	auto group = manager->addProperty(QtVariantPropertyManager::groupTypeId());
	group->setPropertyName("Parabolic SAR");

	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	auto prop = manager->addProperty(QVariant::Double, "max");
	prop->setValue(indicator->getMax());
	prop->setAttribute("singleStep", 0.01);
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator, this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changeMax((value.toDouble() > 0) ? value.toDouble() : 0);
		emit writeIndicator(indicator);
	});

	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	prop = manager->addProperty(QVariant::Double, "step");
	prop->setValue(indicator->getStep());
	prop->setAttribute("singleStep", 0.01);
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator, this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changeStep((value.toDouble() > 0) ? value.toDouble() : 0);
		emit writeIndicator(indicator);
	});

	browser->addProperty(group);

	///////////////////////////////////////////////////
	// Series
	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);

	group = manager->addProperty(QtVariantPropertyManager::groupTypeId());
	group->setPropertyName("Series");

	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	prop = manager->addProperty(QVariant::Color, "color");
	prop->setValue(indicator->getColor());
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator, this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changeColor(value.value<QColor>());
		emit writeIndicator(indicator);
	});

	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	prop = manager->addProperty(QVariant::Double, "size");
	prop->setAttribute("singleStep", 0.1);
	prop->setValue(indicator->getSize());
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator, this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changeSize((value.toDouble() > 0) ? value.toDouble() : 0);
		emit writeIndicator(indicator);
	});

	browser->addProperty(group);
}

void GuiIndicatorsList::setSmaProperty(Indicators::IndicatorSma *indicator)
{
	auto factory = new QtVariantEditorFactory(browser);
	auto manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);

	auto group = manager->addProperty(QtVariantPropertyManager::groupTypeId());
	group->setPropertyName("Simple Moving Average");

	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	auto prop = manager->addProperty(QVariant::Int, "period");
	prop->setValue(indicator->getPeriod());
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator, this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changePeriod((value.toInt() > 0) ? value.toInt() : 0);
		emit writeIndicator(indicator);
	});

	browser->addProperty(group);

	///////////////////////////////////////////////////
	// Series
	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);

	group = manager->addProperty(QtVariantPropertyManager::groupTypeId());
	group->setPropertyName("Series");

	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	prop = manager->addProperty(QVariant::Color, "color");
	prop->setValue(indicator->getColor());
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator, this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changeColor(value.value<QColor>());
		emit writeIndicator(indicator);
	});

	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	prop = manager->addProperty(QVariant::Int, "size");
	prop->setValue(indicator->getSize());
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator, this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changeSize(value.toInt());
		emit writeIndicator(indicator);
	});

	browser->addProperty(group);
}

void GuiIndicatorsList::setStochasticProperty(Indicators::IndicatorStochastic *indicator)
{
	auto factory = new QtVariantEditorFactory(browser);
	auto manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);

	auto group = manager->addProperty(QtVariantPropertyManager::groupTypeId());
	group->setPropertyName("Stochastic Oscillator");

	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	auto prop = manager->addProperty(QVariant::Int, "%K period");
	prop->setValue(indicator->getFastPeriod());
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator, this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changeFast((value.toInt() > 0) ? value.toInt() : 0);
		emit writeIndicator(indicator);
	});

	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	prop = manager->addProperty(QVariant::Int, "%D period");
	prop->setValue(indicator->getSlowPeriod());
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator, this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changeSlow((value.toInt() > 0) ? value.toInt() : 0);
		emit writeIndicator(indicator);
	});

	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	prop = manager->addProperty(QVariant::Int, "smooth");
	prop->setValue(indicator->getSmooth());
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator, this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changeSmooth((value.toInt() > 0) ? value.toInt() : 0);
		emit writeIndicator(indicator);
	});

	browser->addProperty(group);

	///////////////////////////////////////////////////
	// %K Series
	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);

	group = manager->addProperty(QtVariantPropertyManager::groupTypeId());
	group->setPropertyName("%K series");

	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	prop = manager->addProperty(QVariant::Color, "color");
	prop->setValue(indicator->getFastColor());
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator, this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changeFastColor(value.value<QColor>());
		emit writeIndicator(indicator);
	});

	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	prop = manager->addProperty(QVariant::Int, "size");
	prop->setValue(indicator->getFastSize());
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator, this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changeFastSize((value.toInt() > 0) ? value.toInt() : 0);
		emit writeIndicator(indicator);
	});

	browser->addProperty(group);


	///////////////////////////////////////////////////
	// %D Series
	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);

	group = manager->addProperty(QtVariantPropertyManager::groupTypeId());
	group->setPropertyName("%D series");

	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	prop = manager->addProperty(QVariant::Color, "color");
	prop->setValue(indicator->getSlowColor());
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator, this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changeSlowColor(value.value<QColor>());
		emit writeIndicator(indicator);
	});

	manager = new QtVariantPropertyManager(browser);
	browser->setFactoryForManager(manager, factory);
	prop = manager->addProperty(QVariant::Int, "size");
	prop->setValue(indicator->getSlowSize());
	group->addSubProperty(prop);
	connect(manager, &QtVariantPropertyManager::valueChanged,
		[indicator, this](QtProperty *p, QVariant  value)
	{
		Q_UNUSED(p);

		indicator->changeSlowSize((value.toInt() > 0) ? value.toInt() : 0);
		emit writeIndicator(indicator);
	});

	browser->addProperty(group);
}

void GuiIndicatorsList::onRemove()
{
	for (auto item : ui.treeWidget->selectedItems())
	{
		const auto index = item->data(0, Qt::UserRole + QVariant::UInt).toUInt();
		auto indicator = indicators.at(index);

		emit removeIndicator(indicator);

		ui.treeWidget->removeItemWidget(item, 0);
		delete item;
		if (browser)	browser->hide();
	}
}


}