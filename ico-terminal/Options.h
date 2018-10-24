#pragma once

#include <QObject>

class Options : public QObject
{
	Q_OBJECT

public:
	Options(QObject *parent = nullptr);

	void setChatId(qint32);
	qint32 getChatId() const;

	void enableBackgroundNotice(bool enabled = true);

	bool isBackgroundNotice() const;

	inline void setWexKey(const QString &key) {	wexKey = key;}
	inline void setWexSecret(const QString &secret) {wexSecret = secret;}

	inline QString getWexKey()    const {return wexKey;}
	inline QString getWexSecret() const {return wexSecret;}

	void save();

private:
	// notification
	qint32 chatId = 0;
	bool backgroundNotice = false;

	// wex api
	QString wexKey    = "";
	QString wexSecret = "";
};
