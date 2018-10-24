#pragma once

#include <QWidget>
#include "ui_PropertyApi.h"

class Options;

class PropertyApi : public QWidget
{
	Q_OBJECT

public:
	PropertyApi(QWidget *parent = Q_NULLPTR);
	~PropertyApi() = default;

	void setOptions(Options *option);

public slots:
	void onApply();

private:
	Ui::PropertyApi ui;
	Options *options = nullptr;
};
