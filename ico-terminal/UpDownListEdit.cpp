#include "UpDownListEdit.h"

UpDownListEdit::UpDownListEdit(QWidget *parent)
	: QComboBox(parent)
{
	populateList();
}


QString UpDownListEdit::string() const
{
	return itemData(currentIndex(), Qt::DecorationRole).toString();
}

void UpDownListEdit::setString(QString c)
{
	setCurrentIndex(findData(c, int(Qt::DecorationRole)));
}

void UpDownListEdit::populateList()
{
	QStringList list;
	list << "up" << "down";

	for (int i = 0; i < list.size(); ++i) {
		insertItem(i,list[i]);
		setItemData(i, list[i], Qt::DecorationRole);
	}
}
