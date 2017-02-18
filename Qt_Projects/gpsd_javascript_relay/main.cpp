#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include "Application.h"
#include "MainWindow.h"

int main(int argc, char *argv[]){
	Application a( argc, argv );
	
	if( !Application::isPreventingMultipleInstances() ){
		// Translation:
		QString locale = QLocale::system().name().section( '_', 0,0 );
		QTranslator translator1;
			translator1.load( QString( ":/gpsd_javascript_relay_" )+locale );
		QTranslator translator2;
			translator2.load( QString( "qt_" )+locale, QLibraryInfo::location( QLibraryInfo::TranslationsPath ) );
		a.installTranslator( &translator1 );
		a.installTranslator( &translator2 );
		
		// Main window:
		MainWindow w;
		if( !APP_CLO_MINIMIZED ) w.show();
		return a.exec();
	}
	return 0;
}
