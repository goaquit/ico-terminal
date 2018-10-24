#pragma once
#include <QtTreePropertyBrowser>
#include <QWidget>
#include "ui_GuiIndicatorsList.h"
#include <vector>

namespace Indicators
{
class AbstractIndicator;
class IndicatorBollinger;
class IndicatorDx;
class IndicatorStochastic;
class IndicatorSma;
class IndicatorParabollicSar;
}


namespace Gui
{


class GuiIndicatorsList : public QWidget
{
	Q_OBJECT

public:
	GuiIndicatorsList(QWidget *parent = Q_NULLPTR);
	~GuiIndicatorsList();

	void setIndicators(const std::vector<Indicators::AbstractIndicator *> &);

public slots:
	void onSelectedIndicator(QTreeWidgetItem *item, int column);
	void onButtonBoxClicked(QAbstractButton *button);
	void onRemove();

signals:
	void writeIndicator(const Indicators::AbstractIndicator *);
	void removeIndicator(Indicators::AbstractIndicator *);

private:
	Ui::GuiIndicatorsList ui;
	QtTreePropertyBrowser *browser = nullptr;

	// indicators
	std::vector<Indicators::AbstractIndicator *> indicators;
	std::vector<Indicators::AbstractIndicator *> temporary;

	void setAdxProperty(Indicators::IndicatorDx *);
	void setBollingerProperty(Indicators::IndicatorBollinger *);
	void setSarProperty(Indicators::IndicatorParabollicSar *);
	void setSmaProperty(Indicators::IndicatorSma *);
	void setStochasticProperty(Indicators::IndicatorStochastic *);
	
};


}


