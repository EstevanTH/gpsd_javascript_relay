/** Class MainWindow
This widget class represents the main window.
It handles most of the startup work as well.
**/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define MAINWINDOW_TRAY_GPSD_CONNECTION_MESSAGE_DURATION 4000

#include <QWidget>
#include <QCloseEvent>
#include <QMenu>
#include <QStringList>
#include <QSessionManager>
#include "ui_MainWindow.h"
#include "HttpServer.h"
#include "GpsdTabSetupWidget.h"
#include "GpsdStatusWidget.h"
#include "TargetTabSetupWidget.h"

namespace Ui{
	class MainWindow;
}

class MainWindow:
	public QWidget,
	private Ui::MainWindow
{
	Q_OBJECT
	
	public:
		explicit MainWindow(QWidget *parent = 0);
		
	public slots:
		void addedGpsdClient(GpsdClient* gpsdClient);
		void removedGpsdClient(GpsdClient* gpsdClient);
		void restoredMinimizedInTray();
		void closeWindow();
		
	protected:
		void minimizeInTray();
		void saveSettings();
		void makeAppIcon(QIcon const* &icon, QColor color);
		void closeEvent(QCloseEvent *event);
		
	private slots:
		void selectCategory();
		void updateHttpRequests(quint64 const& newRequests);
		void updateHttpStatus(HttpServerStatus const& newStatus);
		void flagHttpSetupChanged();
		GpsdClient* makeNewGpsdClient();
		void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
		void gpsdConnectionStateChanged(bool connected);
		void updateTrayIconColor();
		void prepareLogout(QSessionManager& manager);
		
	private:
		bool m_httpSetupChanged;
		QPushButton* m_btnGpsdNewTab;
		QSystemTrayIcon* m_trayIcon;
		bool m_loggingOut;
		static QString s_messageConnected;
		static QString s_messageConnectionLost;
		static QPixmap const* s_appIconDefault; // only used as a base
		static QIcon const* s_appIconStopped;
		static QIcon const* s_appIconConnecting;
		static QIcon const* s_appIconConnected;
		static QIcon const* s_appIconUnconnected;
};

#endif // MAINWINDOW_H
