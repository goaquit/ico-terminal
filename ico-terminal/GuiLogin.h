#pragma once

#include <QMainWindow>
#include "ui_GuiLogin.h"
#include "UserRegistrationClient.h"

class GuiLogin : public QMainWindow
{
	Q_OBJECT

public:
	GuiLogin(QWidget *parent = Q_NULLPTR);

	void setHost(const QString&);

public slots:
	void signIn();
	void registration();
	void passwordChange();
	void showSignUp();
	void showPassChange();
	void showDefault();
	void hasRegistered();
	void hasChangedPassword();
	void onReceivedMessage(const QString&);

private:
	Ui::GuiLogin ui;
	UserRegistrationClient socket;

	void hideAdvanced();
	void showAdvanced();
	void defaultButton();
	void hideAllButtons();

	QString host = "127.0.0.1";
};
