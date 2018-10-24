#pragma once

#include <QWidget>
#include "ui_GuiModifyPosition.h"

class Position;

class GuiModifyPosition : public QWidget
{
	Q_OBJECT

public:
	GuiModifyPosition(QWidget *parent = Q_NULLPTR);
	~GuiModifyPosition() = default;

	inline void setPosition(Position *p) {		position = p; readParameters();		}

public slots:
	void modify();
	void onNewBid(double,const QString &);
	void onNewAsk(double,const QString &);

signals:
	void modifyed();

private:
	Ui::GuiModifyPosition ui;

	Position *position = nullptr;

	void readParameters();
};
