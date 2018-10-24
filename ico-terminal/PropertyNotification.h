#pragma once

#include <QWidget>
#include "ui_PropertyNotification.h"

class Options;

class PropertyNotification : public QWidget
{
	Q_OBJECT

public:
	PropertyNotification(QWidget *parent = Q_NULLPTR);
	~PropertyNotification() = default;

	void onApply();
	void setOptions(Options *);

private:
	Ui::PropertyNotification ui;

	Options *options = nullptr;
};
