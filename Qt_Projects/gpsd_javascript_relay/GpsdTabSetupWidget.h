/** Class GpsdTabSetupWidget
This widget class sets up the connection to GPSD servers of a GPSD client.
**/

#ifndef GPSDTABSETUPWIDGET_H
#define GPSDTABSETUPWIDGET_H

#include <QWidget>
#include "ui_GpsdTabSetupWidget.h"
#include "Application.h"
#include "GpsdHostWidget.h"
#include "GpsdClient.h"

namespace Ui {
	class GpsdTabSetupWidget;
}

class GpsdTabSetupWidget:
	public QWidget,
	private Ui::GpsdTabSetupWidget
{
	Q_OBJECT
	
	public:
		struct ConfigurationHost{
			QString hostName;
			quint16 port;
			QAbstractSocket::SocketType protocol;
			bool symmetric;
		};
		struct Configuration{
			QString name;
			double refreshRate;
			bool autoConnect;
			bool autoReconnect;
			QList<struct ConfigurationHost> hosts;
		};
		
		explicit GpsdTabSetupWidget(GpsdClient* gpsdClient, QTabWidget* sourcesTabs, QWidget *parent=0);
		GpsdClient* getGpsdClient();
		void resetHasChanged();
		bool getHasChanged() const;
		int getNumHosts() const;
		GpsdHostWidget* addHost();
		GpsdHostWidget* addHost(QString const& hostName, quint16 const& port=2947, QAbstractSocket::SocketType const& protocol=QAbstractSocket::TcpSocket, bool symmetric=false);
		void applyConfiguration();
		void getConfiguration(struct Configuration& c) const;
		void setConfiguration(QJsonObject const& gpsdCSettings);
		
	private slots:
		void flagHasChanged();
		void updateTabText(QString const& text);
		void tabCloseRequested(int index);
		
	private:
		GpsdClient* m_gpsdClient;
		QTabWidget* m_gSourcesTabs; // not contained by this class
		bool m_hasChanged;
		
	private slots:
		void on_m_btnAddHost_clicked();
};

#endif // GPSDTABSETUPWIDGET_H
