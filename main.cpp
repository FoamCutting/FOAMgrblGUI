#include <QtGui/QApplication>
#include "grblcage.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	GrblCage MainWindow;
	MainWindow.showMaximized();
	return a.exec();
}
