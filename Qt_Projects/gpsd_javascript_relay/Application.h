/** Class Application
This class represents the application.
Only 1 instance can be created, so attributes and most methods are static.
**/

#ifndef APPLICATION_H
#define APPLICATION_H

#define APPLICATION_INSTANCE_TIMEOUT 1000
#define APPLICATION_INSTANCE_REFRESH 100

#define App ( Application::instance() )
#define APP_CLO_CONFIGNOLOAD ( Application::getCommandLineOptions().confignoload )
#define APP_CLO_CONFIGNOSAVE ( Application::getCommandLineOptions().confignosave )
#define APP_CLO_MINIMIZED ( Application::getCommandLineOptions().minimized )
#define APP_CLO_NOAUTOSTART ( Application::getCommandLineOptions().noautostart )
#define APP_CLO_TRAYNOMESSAGES ( Application::getCommandLineOptions().traynomessages )

#include <QApplication>
#include <QIcon>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDateTime>
#include <QTimer>
#include <QLocale>
#include "SingleInstanceData.h"
class GpsdClient;

class Application: public QApplication{
	Q_OBJECT
		
	public:
		typedef struct CommandLineOptions{
			bool confignoload;
			bool confignosave;
			bool minimized;
			bool noautostart;
			bool traynomessages;
		} CommandLineOptions;
		
		Application(int &argc, char **argv);
		void addGpsdClient(GpsdClient* gpsdClient);
		void removeGpsdClient(GpsdClient* gpsdClient);
		static QJsonObject getAppSettings(); // QSettings has only 1 array level, so JSON chosen instead
		static bool setAppSettings(QJsonObject const& settings);
		static Application* instance();
		static QString const& getAppTitle();
		static void flagPreventingMultipleInstances();
		static bool isPreventingMultipleInstances();
		static CommandLineOptions const& getCommandLineOptions();
		static QString const& getLanguage();
		static QString const& getVersion();
		
	signals:
		void addedGpsdClient(GpsdClient* gpsdClient);
		void removedGpsdClient(GpsdClient* gpsdClient);
		void seenNewInstance();
		
	public slots:
		
	private slots:
		void singleInstanceCheck();
		
	private:
		static QString const s_version;
		static QString const s_appTitle;
		static QSettings* s_appSettings;
		static Application* s_instance;
		static QTimer* s_singleInstanceChecker;
		static bool s_preventingMultipleInstances;
		static CommandLineOptions s_commandLineOptions;
		static QString s_language;
};

#endif // APPLICATION_H
