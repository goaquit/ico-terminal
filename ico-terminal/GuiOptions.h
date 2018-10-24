#pragma once

#include <QWidget>
#include "ui_GuiOptions.h"
#include "Options.h"

class GuiOptions : public QWidget
{
	Q_OBJECT

public:
	GuiOptions(QWidget *parent = Q_NULLPTR);
	~GuiOptions();

	void initOptions();

	void selectNotification();
	void selectApi();

	void setOption(Options *);

signals:
	void applyParameters();

private:
	Ui::GuiOptions ui;
	Options *option = nullptr;
	QWidget *currentProperty = nullptr;

private slots:
	void selectItem(const QListWidgetItem *);
	void onDialogButtonClicked(QAbstractButton * button);

};
