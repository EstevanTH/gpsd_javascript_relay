/** Class MainWindow
This widget class represents the main window.
It handles most of the startup work as well.
**/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QCloseEvent>
#include <QMenu>
#include <QStringList>
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
		void closeEvent(QCloseEvent *event);
		
	private slots:
		void selectCategory();
		void updateHttpRequests(quint64 const& newRequests);
		void updateHttpStatus(HttpServerStatus const& newStatus);
		void flagHttpSetupChanged();
		GpsdClient* makeNewGpsdClient();
		void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
		
	private:
		bool m_httpSetupChanged;
		QPushButton* m_btnGpsdNewTab;
		QSystemTrayIcon* m_trayIcon;
		// bool m_quitRequested;
};

#endif // MAINWINDOW_H
