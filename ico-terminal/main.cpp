#include "GuiLogin.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QCoreApplication::addLibraryPath("./");
	QCoreApplication::setOrganizationName("No name");
	QCoreApplication::setApplicationName("Terminal");

	QApplication a(argc, argv);

	GuiLogin login;
	login.setHost("144.76.132.240");
	login.show();

	return a.exec();
}
