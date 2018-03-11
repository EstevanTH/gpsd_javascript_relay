#include "Application.h"

QString const Application::s_version = "0.9.3a";
QString const Application::s_appTitle = "GPSD to JavaScript relay";
QSettings* Application::s_appSettings = 0;
Application* Application::s_instance = 0;
QTimer* Application::s_singleInstanceChecker = 0;
bool Application::s_preventingMultipleInstances = false;
Application::CommandLineOptions Application::s_commandLineOptions = {
	false,
	false,
	false,
	false,
	false,
};
QString Application::s_language = QLocale::system().name().section( '_', 0,0 );

Application::Application(int &argc, char **argv):
	QApplication( argc, argv )
{
	s_instance = this;
	setWindowIcon( QIcon( ":/gpsd_javascript_relay_32.png" ) );
	
	// Load command-line arguments:
	QStringList args = QCoreApplication::arguments();
	for( int i=0; i<args.size(); ++i ){
		QString const str = args[i];
		QString str_lower = str.toLower();
		if( str_lower=="--confignoload" ){
			s_commandLineOptions.confignoload = true;
		}
		else if( str_lower=="--confignosave" ){
			s_commandLineOptions.confignosave = true;
		}
		else if( str_lower=="--language" ){
			if( ( i+1 )<args.size() && args[i+1].size()==2 ){
				s_language = args[++i].toLower();
			}
		}
		else if( str_lower=="--minimized" ){
			s_commandLineOptions.minimized = true;
		}
		else if( str_lower=="--noautostart" ){
			s_commandLineOptions.noautostart = true;
		}
		else if( str_lower=="--traynomessages" ){
			s_commandLineOptions.traynomessages = true;
		}
	}
	
	// Prepare for application settings:
	if( !s_appSettings ){
		s_appSettings = new QSettings(
			QSettings::IniFormat,
			QSettings::UserScope,
			s_appTitle,
			QString(),
			this
		);
	}
	
	// Prepare for single instance:
	SingleInstanceData::prepare();
	if( !s_singleInstanceChecker ){
		s_singleInstanceChecker = new QTimer();
		s_singleInstanceChecker->setInterval( 100 );
		s_singleInstanceChecker->setTimerType( Qt::CoarseTimer );
		connect( s_singleInstanceChecker, SIGNAL( timeout() ), this, SLOT( singleInstanceCheck() ) );
		s_singleInstanceChecker->start();
	}
}

void Application::addGpsdClient(GpsdClient* gpsdClient){
	emit addedGpsdClient( gpsdClient );
}
void Application::removeGpsdClient(GpsdClient* gpsdClient){
	emit removedGpsdClient( gpsdClient );
}

QJsonObject Application::getAppSettings(){
	if( APP_CLO_CONFIGNOLOAD ){
		return QJsonObject();
	}
	else{
		QFile file( s_appSettings->fileName() ); // not pointer, good idea?
		if( file.open( QIODevice::ReadOnly ) ){
			QJsonDocument doc = QJsonDocument::fromJson( file.readAll() );
			file.close();
			if( doc.isNull() ){
				return QJsonObject();
			}
			else {
				return doc.object();
			}
		}
		else{
			return QJsonObject();
		}
	}
}

bool Application::setAppSettings(QJsonObject const& settings){
	QFile file( s_appSettings->fileName() ); // not pointer, good idea?
	if( file.open( QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text ) ){
		QJsonDocument doc = QJsonDocument( settings );
		qint64 written = file.write( doc.toJson( QJsonDocument::Indented ) );
		file.close();
		return( written!=-1 );
	}
	else{
		return false;
	}
}

Application* Application::instance(){
	return s_instance;
}

QString const& Application::getAppTitle(){
	return s_appTitle;
}

void Application::flagPreventingMultipleInstances(){
	s_preventingMultipleInstances = true;
}

bool Application::isPreventingMultipleInstances(){
	return s_preventingMultipleInstances;
}

Application::CommandLineOptions const& Application::getCommandLineOptions(){
	return s_commandLineOptions;
}

QString const& Application::getLanguage(){
	return s_language;
}

QString const& Application::getVersion(){
	return s_version;
}

void Application::singleInstanceCheck(){
	SingleInstanceData* data = SingleInstanceData::getData();
	if( data ){
		bool newInstanceSignaled = data->hasNewInstanceSignaled();
		data->refresh();
		data->releaseMutex();
		if( newInstanceSignaled ) emit seenNewInstance();
	}
}
