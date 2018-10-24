#include "AbstractIndicator.h"
#include "TradePrice.h"

#include <QtCharts/QChartGlobal>
#include <QDateTime>
#include <QGraphicsView>
#include <QList>

#include <vector>

QT_BEGIN_NAMESPACE
class QAction;
class QGraphicsLineItem;
class QGraphicsScene;
class QMenu;
class QMouseEvent;
class QResizeEvent;
class QShowEvent;
class QTimer;
QT_END_NAMESPACE

QT_CHARTS_BEGIN_NAMESPACE
class QAbstractSeries;
class QDateTimeAxis;
QT_CHARTS_END_NAMESPACE

namespace ChartCore
{
class ChartTracker;
class Chart;
}


QT_CHARTS_USE_NAMESPACE;

namespace Terminal
{


class ChartView : public QGraphicsView
{
	Q_OBJECT

public:
	ChartView(QWidget *parent = nullptr);
	~ChartView();

	QDateTime maxDate() const;
	QDateTime minDate() const;

	void setMaxDate(const QDateTime &);
	void setMinDate(const QDateTime &);

	void setLastTradePrice(const Entity::TradePrice &);

	void setMainSeries(QAbstractSeries *);
	void setAmountSeries(QAbstractSeries *);

	void addToMain(QAbstractSeries *);

	void addAdditionalChart(ChartCore::Chart *);

	uint getAdditionalChartCount() const;

public slots:
	//void updateChart(QList<Entity::TradePrice>);
	void scrollHorizontal(const QDateTime &);
	void scrollToLastDate();
	void changeSegment(int);
	void changeRangeMain(qreal max, qreal min);
	void changeRangeAmount(qreal max);
	void onMainChartInfo(const QString &);
	void drawCharts();
	void onRemoveChart(const ChartCore::Chart *);

signals:
	void requestDateInfo(uint);

protected:
	void resizeEvent(QResizeEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;

private:

	ChartCore::Chart *mainChart   = nullptr;
	ChartCore::Chart *amountChart = nullptr;
	std::vector<ChartCore::Chart *> additionalCharts;

	QDateTimeAxis *axisDate;


	// trackers
	ChartCore::ChartTracker *trackerPrice = nullptr;
	std::vector<ChartCore::ChartTracker *> trackersDate;

	Entity::TradePrice lastTrade;
};



}