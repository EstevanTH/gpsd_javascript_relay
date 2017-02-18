#include "GpsdHost.h"

GpsdHost::GpsdHost(QObject *parent):
	QObject( parent ),
	m_port( 0 ),
	m_protocol( QAbstractSocket::TcpSocket ),
	m_symmetric( false )
{
	// nothing
}

QString GpsdHost::getHostname() const{ return m_hostname; }
quint16 GpsdHost::getPort() const{ return m_port; }
QAbstractSocket::SocketType GpsdHost::getProtocol() const{ return m_protocol; }
bool GpsdHost::isSymmetric() const{ return m_symmetric; }

void GpsdHost::modify(QString const& hostname, quint16 const& port, QAbstractSocket::SocketType const& protocol, bool symmetric){
	m_hostname = hostname;
	m_port = port;
	m_protocol = protocol;
	m_symmetric = symmetric;
}

bool operator==(GpsdHost const& a, GpsdHost const& b){
	return( a.m_hostname==b.m_hostname && a.m_port==b.m_port && a.m_protocol==b.m_protocol && a.m_symmetric==b.m_symmetric );
}
