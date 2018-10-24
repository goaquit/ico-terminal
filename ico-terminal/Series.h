#pragma once
#include "TradePrice.h"

#include <QObject>
#include <QtCharts/QChartGlobal>

#include <vector>
#include <QColor>

QT_CHARTS_BEGIN_NAMESPACE
class QAbstractSeries;
class QLineSeries;
class QCandlestickSeries;
QT_CHARTS_END_NAMESPACE

using namespace QtCharts;
using namespace Entity;

namespace ChartCore
{
	enum ChartSeriesType : uint8_t
	{
		Line,
		Candlestick
	};

	class Series : public QObject
	{
		Q_OBJECT

	public:
		Series(QObject *parent = nullptr);

		// Инициирование 
		void init(ChartSeriesType);
		// Смена типа серии
		void change(ChartSeriesType);
		// Добавление данных к серии
		void append(const TradePrice&);
		// Очистка
		void clear();

		// Удаление первого элемента
		void removeFirst();
		// Удаление последнего элемента
		void removeLast();
		// Удалениес первого элемента данных до указанного TradePrice
		void removeTo(const TradePrice&);

		// Обновление последнего элемента в серии
		void updateLastTrade(const TradePrice& trade);

		// Получение серии
		QAbstractSeries* getSeries() const;
		QAbstractSeries* getAmount() const;

		// Расчет верхней/нижней границы цены
		void processRange(time_t start, time_t end);

		// Верхняя граница цены
		double getMax() const;
		// Нижняя граница цены
		double getMin() const;

		double getMaxAmount() const;

		TradePrice getLastTrade() const;

		size_t size() const;

	private:
		ChartSeriesType type = ChartSeriesType::Line;

		// data
		std::vector<TradePrice> data;

		QAbstractSeries* main = nullptr;

		QLineSeries*        line        = nullptr;
		QCandlestickSeries* candlestick = nullptr;

		double max = DBL_MIN;
		double min = DBL_MAX;

		double maxAmount = 0;

		TradePrice lastTradePrice;

		// amount 

		QLineSeries *amount = nullptr;

		std::vector<QAbstractSeries *> seriesVector;
	};
}