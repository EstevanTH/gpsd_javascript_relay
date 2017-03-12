#include "GpsdHost.h"

GpsdHost::GpsdHost(QObject *parent):
	QObject( parent ),
	m_connectionMethod( 0 ),
	m_port( 0 ),
	m_protocol( QAbstractSocket::TcpSocket ),
	m_symmetric( false ),
	m_serialBaudRate( -1 ),
	m_serialDataBits( -1 ),
	m_serialFlowControl( -1 ),
	m_serialParity( -1 ),
	m_serialStopBits( -1 ),
	m_serialLowLatency( true )
{
	// nothing
}

void GpsdHost::modify(
	QString const& hostname,
	quint16 const& port,
	QAbstractSocket::SocketType const& protocol,
	bool symmetric,
	int connectionMethod,
	qint32 serialBaudRate,
	int serialDataBits,
	int serialFlowControl,
	int serialParity,
	int serialStopBits,
	bool serialLowLatency
){
	m_connectionMethod = connectionMethod;
	m_hostname = hostname;
	m_port = port;
	m_protocol = protocol;
	m_symmetric = symmetric;
	m_serialBaudRate = serialBaudRate;
	m_serialDataBits = serialDataBits;
	m_serialFlowControl = serialFlowControl;
	m_serialParity = serialParity;
	m_serialStopBits = serialStopBits;
	m_serialLowLatency = serialLowLatency;
}

bool operator==(GpsdHost const& a, GpsdHost const& b){
	if( a.m_connectionMethod!=b.m_connectionMethod || a.m_hostname!=b.m_hostname ) return false;
	if( a.m_connectionMethod==1 ){ // serial port
		return(
			a.m_serialBaudRate==b.m_serialBaudRate &&
			a.m_serialDataBits==b.m_serialDataBits &&
			a.m_serialFlowControl==b.m_serialFlowControl &&
			a.m_serialParity==b.m_serialParity &&
			a.m_serialStopBits==b.m_serialStopBits &&
			a.m_serialLowLatency==b.m_serialLowLatency
		);
	}
	else{ // IP link
		return(
			a.m_port==b.m_port &&
			a.m_protocol==b.m_protocol &&
			a.m_symmetric==b.m_symmetric
		);
	}
}
