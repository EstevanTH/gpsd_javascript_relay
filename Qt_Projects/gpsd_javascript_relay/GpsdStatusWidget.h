/** Class GpsdStatusWidget
This widget class starts and stops a GPSD client and it displays its information.
**/

#ifndef GPSDSTATUSWIDGET_H
#define GPSDSTATUSWIDGET_H

#include <QFrame>
#include "ui_GpsdStatusWidget.h"
#include "GpsdClient.h"

namespace Ui {
	class GpsdStatusWidget;
}

class GpsdStatusWidget:
	public QFrame,
	private Ui::GpsdStatusWidget
{
	Q_OBJECT
	
	public:
		explicit GpsdStatusWidget(GpsdClient* gpsdClient, QWidget *parent=0);
		GpsdClient* getGpsdClient() const;
		
	private slots:
		void statusChanged(QAbstractSocket::SocketState status, QString const& hostname, quint16 const& port, QAbstractSocket::SocketType const& protocol) const;
		void dataTpvChanged(GpsdClient::DataTpv const& data) const;
		
	private:
		GpsdClient* m_gpsdClient;
		static QString const s_statusTextStopped;
		static QString const s_statusTextHostLookup;
		static QString const s_statusTextConnecting;
		static QString const s_statusTextConnected;
		static QString const s_statusTextUnconnected;
		static QString const s_statusProtocolTcp;
		static QString const s_statusProtocolUdp;
		static QString const s_unitDegrees;
		static QString const s_unitMeters;
		static QString const s_unitMetersPerSecond;
		static QString const s_unitMilliseconds;
		static QString const s_unitSeconds;
		static QString const s_tpvModeNoMode;
		static QString const s_tpvModeNoFix;
		static QString const s_tpvMode2d;
		static QString const s_tpvMode3d;
		static QString const s_nullValue;
};

#endif // GPSDSTATUSWIDGET_H
