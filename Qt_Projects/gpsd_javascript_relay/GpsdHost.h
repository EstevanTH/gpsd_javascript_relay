/** Class GpsdHost
This class represents information of a host of a GPSD client.
It only contains information and it basically does nothing.
**/

#ifndef GPSDHOST_H
#define GPSDHOST_H

#include <QObject>
#include <QString>
#include <QAbstractSocket>

class GpsdHost: public QObject{
	Q_OBJECT
		friend class GpsdClient;
		friend bool operator==(GpsdHost const& a, GpsdHost const& b);
	
	public:
		explicit GpsdHost(QObject *parent = 0);
		void modify(
			QString const& hostname,
			quint16 const& port=2947,
			QAbstractSocket::SocketType const& protocol=QAbstractSocket::TcpSocket,
			bool symmetric=false,
			int connectionMethod=0,
			qint32 serialBaudRate=-1,
			int serialDataBits=-1,
			int serialFlowControl=-1,
			int serialParity=-1,
			int serialStopBits=-1,
			bool serialLowLatency=true
		);
		
	private:
		int m_connectionMethod;
		QString m_hostname;
		quint16 m_port;
		QAbstractSocket::SocketType m_protocol;
		bool m_symmetric;
		qint32 m_serialBaudRate;
		int m_serialDataBits; // cast
		int m_serialFlowControl; // cast
		int m_serialParity; // cast
		int m_serialStopBits; // cast
		bool m_serialLowLatency;
};

bool operator==(GpsdHost const& a, GpsdHost const& b);

#endif // GPSDHOST_H
