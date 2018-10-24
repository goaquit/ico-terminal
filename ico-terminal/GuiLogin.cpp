#include "GuiLogin.h"
#include "MDITerminal.h"
#include "WrapperDb.h"
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

using namespace TerminalCore;

static const quint16 serverPort = 54770; //User registration server
static const QString databse = "crypto";



GuiLogin::GuiLogin(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	showDefault();

	connect(ui.signIn,         &QPushButton::clicked, this, &GuiLogin::signIn);
	connect(ui.passChange,     &QPushButton::clicked, this, &GuiLogin::passwordChange);
	connect(ui.registration,   &QPushButton::clicked, this, &GuiLogin::registration);
	connect(ui.signUp,         &QPushButton::clicked, this, &GuiLogin::showSignUp);
	connect(ui.showPassChange, &QPushButton::clicked, this, &GuiLogin::showPassChange);
	connect(ui.showDafult,     &QPushButton::clicked, this, &GuiLogin::showDefault);

	connect(&socket, &UserRegistrationClient::registerSuccess,       this, &GuiLogin::hasRegistered);
	connect(&socket, &UserRegistrationClient::changePasswordSuccess, this, &GuiLogin::hasChangedPassword);
	connect(&socket, &UserRegistrationClient::message,               this, &GuiLogin::onReceivedMessage);
}

void GuiLogin::signIn()
{
	if (!ui.login->text().length() || !ui.password->text().length()) return;
	// Init database connect (singelton)

	auto db = WrapperDb::instance();

	QString profile = ui.login->text();
	QString password = ui.password->text();


	db->setHost(host);
	db->setDataBaseName(databse);
	db->setUserName(profile);
	db->setPassword(password);

	if (!db->init())
	{
		ui.textEdit->show();
		ui.textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		/*ui.textEdit->setText("Incorrect login or password");*/
		ui.textEdit->setText(db->lastError().databaseText());
		qDebug() << "DataBase is not open: " << db->lastError().databaseText();
		return;
	}
	

	// Run main program
	auto terminal = new MDITerminal;
	terminal->setProfileName(profile);
	terminal->init();
	terminal->show();

	close();
}

void GuiLogin::setHost(const QString &h)
{
	host = h;

	socket.setPort(serverPort);
	socket.connect(host);
}

void GuiLogin::registration()
{
	const QString login = ui.login->text().trimmed();
	const QString password = ui.password->text().trimmed();
	const QString confirmPassword = ui.confirmPassword->text().trimmed();
	const QString checkword = ui.checkword->text().trimmed();

	if (login.isEmpty() || password.isEmpty() || confirmPassword.isEmpty() || checkword.isEmpty())
	{
		ui.textEdit->setText("Empty fields.");

		return;
	}

	QRegExp re("^[a-zA-Z0-9]+$"); 
	if (!re.exactMatch(login))
	{
		ui.textEdit->setText("Wrong symbols in login.");

		return;
	}

	if (password != confirmPassword)
	{
		ui.textEdit->setText("You entered two different passwords. Please try again.");

		return;
	}

	socket.registration(login, password, checkword);
	ui.registration->setEnabled(false);
}

void GuiLogin::passwordChange()
{
	const QString login = ui.login->text().trimmed();
	const QString password = ui.password->text().trimmed();
	const QString confirmPassword = ui.confirmPassword->text().trimmed();
	const QString checkword = ui.checkword->text().trimmed();

	if (login.isEmpty() || password.isEmpty() || confirmPassword.isEmpty() || checkword.isEmpty())
	{
		ui.textEdit->setText("Empty fields.");

		return;
	}

	QRegExp re("^[a-zA-Z0-9]+$");
	if (!re.exactMatch(login))
	{
		ui.textEdit->setText("Wrong symbols in login.");

		return;
	}

	if (password != confirmPassword)
	{
		ui.textEdit->setText("You entered two different passwords. Please try again.");

		return;
	}

	socket.changePassword(login, password, checkword);
	ui.passChange->setEnabled(false);
}


void GuiLogin::showPassChange()
{
	showAdvanced();
	hideAllButtons();
	ui.passChange->show();
	
}

void GuiLogin::showDefault()
{
	hideAdvanced();
	defaultButton();
}

void GuiLogin::hasRegistered()
{
	showDefault();
}

void GuiLogin::hasChangedPassword()
{
	showDefault();
	ui.passChange->setEnabled(false);
}

void GuiLogin::onReceivedMessage(const QString &msg)
{
	ui.textEdit->setText(msg);

	ui.registration->setEnabled(true);
	ui.passChange->setEnabled(true);
}

void GuiLogin::hideAdvanced()
{
	ui.labelConfirmPassword->hide();
	ui.labelCheckword->hide();
	ui.confirmPassword->hide();
	ui.checkword->hide();
}

void GuiLogin::showAdvanced()
{
	ui.labelConfirmPassword->show();
	ui.labelCheckword->show();
	ui.confirmPassword->show();
	ui.checkword->show();

	ui.confirmPassword->setText("");
	ui.checkword->setText("");
}

void GuiLogin::defaultButton()
{
	ui.registration->hide();
	ui.passChange->hide();
	ui.signIn->show();
	ui.signUp->show();
	ui.showPassChange->show();
	ui.showDafult->hide();

	ui.registration->setEnabled(true);
	ui.passChange->setEnabled(true);
}

void GuiLogin::hideAllButtons()
{
	ui.registration->hide();
	ui.passChange->hide();
	ui.signIn->hide();
	ui.signUp->hide();
	ui.showPassChange->hide();
	ui.showDafult->show();
}

void GuiLogin::showSignUp()
{
	showAdvanced();
	hideAllButtons();
	ui.registration->show();
}