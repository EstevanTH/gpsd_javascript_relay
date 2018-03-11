#include "HttpServer.h"
#include <QNetworkProxy>

/// Static stuff:
HttpServer* HttpServer::s_httpServer = 0;
HttpServer* HttpServer::getHttpServer(){
	return s_httpServer;
}

/// Object stuff:
void HttpServer::initialize(){
	if( !s_httpServer ) s_httpServer = this;
}
HttpServer::HttpServer(QObject *parent):
	QObject( parent ),
	m_port( 0 ),
	m_requests( 0 ),
	m_tcpServer( 0 )
{
	initialize();
}
HttpServer::HttpServer(HttpServerSetup const& newSetup, QObject *parent):
	QObject( parent ),
	m_port( 0 ),
	m_requests( 0 ),
	m_tcpServer( 0 )
{
	initialize();
	changeSetup( newSetup );
}

HttpServerStatus HttpServer::getStatus() const{
	HttpServerStatus status;
	if( m_tcpServer && m_tcpServer->isListening() ){
		status.active = true;
		status.address = m_tcpServer->serverAddress().toString();
		status.port = m_tcpServer->serverPort();
		status.requests = m_requests;
	}
	else{
		status.active = false;
		status.address = m_address;
		status.port = m_port;
		status.requests = m_requests;
	}
	return status;
}

quint64 HttpServer::getRequests() const{
	return m_requests;
}

void HttpServer::signalStatusUpdated() const{
	emit statusUpdated( getStatus() );
}

void HttpServer::signalRequestsUpdated() const{
	emit requestsUpdated( getRequests() );
}

void HttpServer::changeSetup(HttpServerSetup const& newSetup){
	m_address = newSetup.address;
	m_port = newSetup.port;
	
	signalStatusUpdated();
}

void HttpServer::startServer(){
	if( !m_tcpServer ){
		m_tcpServer = new QTcpServer( this );
		m_tcpServer->setProxy( QNetworkProxy::NoProxy );
		connect( m_tcpServer, SIGNAL( newConnection() ), this, SLOT( welcomeClient() ) );
		m_requests = 0;
	}
	
	if( !m_tcpServer->isListening() ){
		QString addressStr = m_address.toLower();
		bool validAddress = true;
		QHostAddress address;
			if( addressStr.isEmpty() ) address = QHostAddress::Any;
			else if( addressStr=="any" ) address = QHostAddress::Any;
			else if( addressStr=="local" ) address = QHostAddress::LocalHost;
			else if( addressStr=="local6" ) address = QHostAddress::LocalHostIPv6;
			else validAddress = address.setAddress( addressStr );
		
		if( !validAddress ){
			QMessageBox::critical(
				qobject_cast<QWidget*>( this->parent() ),
				tr( "HTTP server" ),
				tr( "The specified IP address is not valid." ),
				QMessageBox::Close,
				QMessageBox::Close
			);
			stopServer();
		}
		else if( !m_tcpServer->listen( address, m_port ) ){
			QMessageBox::critical(
				qobject_cast<QWidget*>( this->parent() ),
				tr( "HTTP server" ),
				tr( "The HTTP server could not start:<br/>" )+m_tcpServer->errorString(),
				QMessageBox::Close,
				QMessageBox::Close
			);
			stopServer();
		}
		
		signalStatusUpdated();
	}
}

void HttpServer::stopServer(){
	if( m_tcpServer ){
		m_tcpServer->deleteLater();
		m_tcpServer = 0;
	}
	
	signalStatusUpdated();
}

void HttpServer::welcomeClient(){
	QTcpSocket* clientSocket = m_tcpServer->nextPendingConnection();
	if( clientSocket ){
		new HttpServerClient( clientSocket );
		clientSocket->setReadBufferSize( HTTP_MAX_BUFFER ); // should be super large for request headers!
		clientSocket->setSocketOption( QAbstractSocket::LowDelayOption, 1 );
		clientSocket->setSocketOption( QAbstractSocket::ReceiveBufferSizeSocketOption, HTTP_MAX_BUFFER );
		++m_requests;
		signalRequestsUpdated();
	}
}
