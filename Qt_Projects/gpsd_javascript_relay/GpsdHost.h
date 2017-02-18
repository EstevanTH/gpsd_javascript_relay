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
		QString getHostname() const;
		quint16 getPort() const;
		QAbstractSocket::SocketType getProtocol() const;
		bool isSymmetric() const;
		void modify(QString const& hostname, quint16 const& port, QAbstractSocket::SocketType const& protocol, bool symmetric=false);
		
	private:
		QString m_hostname;
		quint16 m_port;
		QAbstractSocket::SocketType m_protocol;
		bool m_symmetric;
};

bool operator==(GpsdHost const& a, GpsdHost const& b);

#endif // GPSDHOST_H
