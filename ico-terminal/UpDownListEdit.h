#pragma once

#include <QComboBox>

class UpDownListEdit : public QComboBox
{
	Q_OBJECT
	Q_PROPERTY(QString string READ string WRITE setString USER true)
public:
	UpDownListEdit(QWidget *parent = nullptr);

	QString string() const;
	void setString(QString c);

private:
	void populateList();
};
