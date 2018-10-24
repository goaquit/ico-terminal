#pragma once

#include <QComboBox>

class ColorListEditor : public QComboBox
{
	Q_OBJECT
	Q_PROPERTY(QColor color READ color WRITE setColor USER true)
public:
	ColorListEditor(QWidget *parent = nullptr);

	QColor color() const;
	void setColor(QColor c);

private:
	void populateList();
};
