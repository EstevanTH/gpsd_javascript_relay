#include "GpsdClient.h"
#include "compiler_options.h"

QString GpsdClient::s_titleGpsdClient;
QLinkedList<GpsdClient*> GpsdClient::s_gpsdClients;
QByteArray const GpsdClient::s_requestContentTpv = "?TPV;\x0D\x0A";
QByteArray const GpsdClient::s_requestContentVersion = "?VERSION;\x0D\x0A";

GpsdClient::GpsdClient(QObject *parent):
	QObject( parent ),
	m_refreshRate( 2 ),
	m_autoReconnect( false ),
	m_httpEnabled( true ),
	m_httpFilename( "default" ),
	m_httpFunction( "" ),
	m_httpExpire( 5.0 ),
	m_javascriptEnabled( false ),
	m_javascriptFunction( "" ),
	m_jsonEnabled( false ),
	m_manuallyStopped( true ),
	m_indexConnectingHost( -1 ),
	m_socket( 0 ),
	m_socketValidated( false ),
	m_requestTimer( 0 ),
	m_expectedAnswers( 0 ),
	m_beginConnection( -1 ),
	m_lastAnswer( -1 ),
	m_lastRequest( -1 ),
	m_outputJavascript( 0 ),
	m_outputJson( 0 )
{
	// Translation:
	if( s_titleGpsdClient.isEmpty() ) s_titleGpsdClient = tr( "GPSD client" );
	
	// Initialization:
	s_gpsdClients.append( this );
	App->addGpsdClient( this );
	m_dataTpv.NotNullFields = 0;
	m_dataTpv.ClientTime = 0;
}

GpsdClient::~GpsdClient(){
	s_gpsdClients.removeOne( this );
	App->removeGpsdClient( this );
}

QString const& GpsdClient::getName() const{ return m_name; }
double GpsdClient::getRefreshRate() const{ return m_refreshRate; }
bool GpsdClient::getAutoReconnect() const{ return m_autoReconnect; }
QList<GpsdHost*> const& GpsdClient::getHosts() const{ return m_hosts; }
bool GpsdClient::getHttpEnabled() const{ return m_httpEnabled; }
	QByteArray const& GpsdClient::getHttpFilename() const{ return m_httpFilename; }
	QByteArray const& GpsdClient::getHttpFunction() const{ return m_httpFunction; }
	double GpsdClient::getHttpExpire() const{ return m_httpExpire; }
bool GpsdClient::getJavascriptEnabled() const{ return m_javascriptEnabled; }
	QString const& GpsdClient::getJavascriptFilename() const{ return m_javascriptFilename; }
	QByteArray const& GpsdClient::getJavascriptFunction() const{ return m_javascriptFunction; }
bool GpsdClient::getJsonEnabled() const{ return m_jsonEnabled; }
	QString const& GpsdClient::getJsonFilename() const{ return m_jsonFilename; }

void GpsdClient::setName(QString const& name){
	m_name = name;
	emit nameChanged( m_name );
}
void GpsdClient::setRefreshRate(double refreshRate){ m_refreshRate = refreshRate; }
void GpsdClient::setAutoReconnect(bool autoReconnect){ m_autoReconnect = autoReconnect; }
GpsdHost* GpsdClient::addHost(QString const& hostname, quint16 const& port, QAbstractSocket::SocketType const& protocol, bool symmetric){
	GpsdHost* host = new GpsdHost( this );
		host->modify( hostname, port, protocol, symmetric );
		m_hosts.append( host );
	return host;
}
void GpsdClient::removeHost(GpsdHost* host){
	host->deleteLater();
	m_hosts.removeOne( host );
}
void GpsdClient::clearHosts(){
	for( QList<GpsdHost*>::const_iterator host=m_hosts.begin(); host!=m_hosts.end(); ++host ){
		( *host )->deleteLater();
	}
	m_hosts.clear();
}
void GpsdClient::setHttpEnabled(bool httpEnabled){ m_httpEnabled = httpEnabled; }
	void GpsdClient::setHttpFilename(QByteArray const& httpFilename){ m_httpFilename = httpFilename; }
	void GpsdClient::setHttpFunction(QByteArray const& httpFunction){ m_httpFunction = httpFunction; }
	void GpsdClient::setHttpExpire(double httpExpire){ m_httpExpire = httpExpire; }
void GpsdClient::setJavascriptEnabled(bool javascriptEnabled){ m_javascriptEnabled = javascriptEnabled; }
	void GpsdClient::setJavascriptFilename(QString const& javascriptFilename){ m_javascriptFilename = javascriptFilename; }
	void GpsdClient::setJavascriptFunction(QByteArray const& javascriptFunction){ m_javascriptFunction = javascriptFunction; }
void GpsdClient::setJsonEnabled(bool jsonEnabled){ m_jsonEnabled = jsonEnabled; }
	void GpsdClient::setJsonFilename(QString const& jsonFilename){ m_jsonFilename = jsonFilename; }


GpsdClient::DataTpv const& GpsdClient::getDataTpv() const{ return m_dataTpv; }
QByteArray const& GpsdClient::getDataTpvJson() const{ return m_dataTpvJson; }
QByteArray const& GpsdClient::getDataTpvJavascriptHttp() const{ return m_dataTpvJavascriptHttp; }

bool GpsdClient::isDataTpvExpired() const{
	if( m_dataTpv.ClientTime==0 ){
		return true; // no data available
	}
	else if( m_httpExpire<=0 ){
		return false; // never expire
	}
	else{
		qint64 httpExpire = ( m_httpExpire*1000 );
		qint64 refreshPeriod = ( 1000/m_refreshRate );
		return( QDateTime::currentMSecsSinceEpoch()>( m_dataTpv.ClientTime+httpExpire+refreshPeriod ) );
	}
}

QAbstractSocket::SocketState GpsdClient::getConnectionStatus() const{
	if( m_socket ){
		QAbstractSocket::SocketState state = m_socket->state();
		if( state==QAbstractSocket::ConnectedState ){
			if( m_socketValidated ){
				return QAbstractSocket::ConnectedState;
			}
			else{
				QUdpSocket* socketUdp = qobject_cast<QUdpSocket*>( m_socket );
				if( socketUdp ){ // UDP socket
					return QAbstractSocket::ConnectingState; // liar!
				}
				else{ // TCP socket
					return QAbstractSocket::ConnectedState;
				}
			}
		}
		else{
			return state;
		}
	}
	else{
		return QAbstractSocket::UnconnectedState;
	}
}

bool GpsdClient::isManuallyStopped() const{
	return m_manuallyStopped;
}

GpsdClient* GpsdClient::findByHttpFilename(QString const& filename){
	for( QLinkedList<GpsdClient*>::const_iterator it=s_gpsdClients.begin(); it!=s_gpsdClients.end(); ++it ){
		if( ( *it )->m_httpFilename==filename ){
			return *it;
		}
	}
	return 0;
}

QLinkedList<GpsdClient*> const& GpsdClient::getGpsdClients(){
	return s_gpsdClients;
}

void GpsdClient::stopClient(){
	if( m_outputJavascript ){
		delete m_outputJavascript;
		m_outputJavascript = 0;
	}
	if( m_outputJson ){
		delete m_outputJson;
		m_outputJson = 0;
	}
	m_manuallyStopped = true;
	m_indexConnectingHost = -1;
	clearSocket();
	signalSocketStateChanged();
}

void GpsdClient::startClient(){
	stopClient();
	if( m_hosts.size()==0 ){
		QMessageBox::critical(
			qobject_cast<QWidget*>( this->parent() ),
			s_titleGpsdClient,
			tr( "The GPSD client <b>" )+m_name+tr( "</b> has an empty hosts list!" ),
			QMessageBox::Close,
			QMessageBox::Close
		);
	}
	else{
		if( m_javascriptEnabled ){
			m_outputJavascript = new QFile( m_javascriptFilename, this );
			if( !m_outputJavascript->open( QIODevice::Append|QIODevice::Text ) ){
				delete m_outputJavascript;
				m_outputJavascript = 0;
				QMessageBox::warning(
					qobject_cast<QWidget*>( this->parent() ),
					s_titleGpsdClient,
					tr( "Unable to open JavaScript output file for <b>" )+m_name+tr( "</b>:<br/>" )+m_javascriptFilename,
					QMessageBox::Ignore,
					QMessageBox::Ignore
				);
			}
		}
		if( m_jsonEnabled ){
			m_outputJson = new QFile( m_jsonFilename, this );
			if( !m_outputJson->open( QIODevice::Append|QIODevice::Text ) ){
				delete m_outputJson;
				m_outputJson = 0;
				QMessageBox::warning(
					qobject_cast<QWidget*>( this->parent() ),
					s_titleGpsdClient,
					tr( "Unable to open JSON output file for <b>" )+m_name+tr( "</b>:<br/>" )+m_javascriptFilename,
					QMessageBox::Ignore,
					QMessageBox::Ignore
				);
			}
		}
		m_manuallyStopped = false;
		m_indexConnectingHost = 0;
		connectToHost( *( m_hosts.at( 0 ) ) );
	}
}

void GpsdClient::sendRequest(QByteArray const& content){
	if( m_socket && m_socket->isValid() ){
		bool expired = false;
		if( qobject_cast<QUdpSocket*>( m_socket ) ){
			qint64 timeReference = m_lastAnswer;
			qint64 refreshPeriod = ( 1000/m_refreshRate );
			qint64 expireDelay = GPSD_EXPIRE_UDP_DATA;
			if( !m_socketValidated ){
				timeReference = m_beginConnection;
				expireDelay = GPSD_EXPIRE_UDP_CONNECT;
			}
			expired = ( QDateTime::currentMSecsSinceEpoch()>( timeReference+expireDelay+refreshPeriod ) ); // flag expired UDP connection
		}
		if( !expired ){
			m_socket->write( content );
			++m_expectedAnswers;
			m_lastRequest = QDateTime::currentMSecsSinceEpoch();
		}
		else{
			m_socket->abort(); // will trigger the socketFailure() signal on the right time (no crash)
		}
	}
}

void GpsdClient::requestTpv(){
	sendRequest( s_requestContentTpv );
}

void GpsdClient::clearRequestTimer(){
	if( m_requestTimer ){
		m_requestTimer->stop();
		m_requestTimer->deleteLater();
		m_requestTimer = 0;
	}
	m_expectedAnswers = 0;
	m_beginConnection = -1;
	m_lastAnswer = -1;
	m_lastRequest = -1;
}

void GpsdClient::clearSocket(){
	// WARNING: Recreating a UDP socket immediatly will crash the program while binding. This method should only be called when the socket is already effectively disconnected.
	if( m_socket ){
		m_socket->abort();
		m_socket->deleteLater();
		m_socket = 0;
	}
	m_socketValidated = false;
	m_bufferIn.clear();
	clearRequestTimer();
}

void GpsdClient::connectToHost(GpsdHost const& host){
	// qDebug() << "GpsdClient::connectToHost" << host.m_hostname << ":" << host.m_port << host.m_protocol; // debug
	emit hostChanged( host.m_hostname, host.m_port, host.m_protocol );
	clearSocket();
	if( host.m_protocol!=QAbstractSocket::UdpSocket ){ // TCP
		m_socket = new QTcpSocket( this );
		connect( m_socket, SIGNAL( connected() ), this, SLOT( linkValidated() ) );
	}
	else{ // UDP
		m_socket = new QUdpSocket( this );
		connect( m_socket, SIGNAL( connected() ), this, SLOT( validateUdpLink() ) );
	}
	m_socket->setReadBufferSize( GPSD_RECEIVE_BUFFER );
	m_socket->setSocketOption( QAbstractSocket::ReceiveBufferSizeSocketOption, GPSD_RECEIVE_BUFFER );
	m_socket->setSocketOption( QAbstractSocket::KeepAliveOption, 1 );
	m_socket->setSocketOption( QAbstractSocket::LowDelayOption, 1 );
	m_bufferIn.clear();
	// m_socket->bind() and m_socket->connectToHost() can both produce errors
	connect( m_socket, SIGNAL( error(QAbstractSocket::SocketError) ), this, SLOT( socketError(QAbstractSocket::SocketError) ) );
	if( host.m_symmetric ){ // symmetric UDP
		m_socket->bind( host.m_port, QAbstractSocket::ShareAddress|QAbstractSocket::ReuseAddressHint );
	}
	else{ // TCP or UDP
		m_socket->bind( 0, QAbstractSocket::DontShareAddress );
	}
	if( m_socket->error()==QAbstractSocket::UnknownSocketError ){ // no problem
		// The condition is mandatory in order to avoid double error trigger in case of bind error.
		connect( m_socket, SIGNAL( stateChanged(QAbstractSocket::SocketState) ), this, SLOT( signalSocketStateChanged() ) );
		connect( m_socket, SIGNAL( disconnected() ), this, SLOT( socketFailure() ) );
		connect( m_socket, SIGNAL( readyRead() ), this, SLOT( incomingData() ) );
		m_beginConnection = QDateTime::currentMSecsSinceEpoch();
		m_socket->connectToHost( host.m_hostname, host.m_port, QIODevice::ReadWrite, QAbstractSocket::AnyIPProtocol );
	}
}

void GpsdClient::signalSocketStateChanged(){
	if( m_socket ){
		emit statusChanged( getConnectionStatus(), m_socket->peerName(), m_socket->peerPort(), m_socket->socketType() );
	}
	else{
		emit statusChanged( getConnectionStatus() );
	}
}

void GpsdClient::validateUdpLink(){
	if( !m_socketValidated ){
		if( !m_requestTimer ){
			m_requestTimer = new QTimer( this );
				m_requestTimer->setInterval( GPSD_RETRY_UDP_DELAY );
				m_requestTimer->setSingleShot( false );
				m_requestTimer->setTimerType( Qt::PreciseTimer );
				m_requestTimer->start();
				connect( m_requestTimer, SIGNAL( timeout() ), this, SLOT( validateUdpLink() ) );
		}
		sendRequest( s_requestContentVersion );
	}
}

void GpsdClient::linkValidated(){
	m_socketValidated = true;
	m_indexConnectingHost = -1;
	clearRequestTimer();
	m_requestTimer = new QTimer( this );
		m_requestTimer->setInterval( 1000/m_refreshRate );
		m_requestTimer->setSingleShot( false );
		m_requestTimer->setTimerType( Qt::PreciseTimer );
		m_requestTimer->start();
		connect( m_requestTimer, SIGNAL( timeout() ), this, SLOT( requestTpv() ) );
	emit connectionStateChanged( true );
}

void GpsdClient::socketRetry(){
	if( !m_manuallyStopped ){
		bool runConnection = false;
		if( m_indexConnectingHost!=-1 ){
			++m_indexConnectingHost;
			if( m_indexConnectingHost>=m_hosts.count() ) m_indexConnectingHost = 0;
			runConnection = true;
		}
		else if( m_autoReconnect ){
			m_indexConnectingHost = 0;
			runConnection = true;
		}
		if( runConnection && m_hosts.count()>0 ){
			connectToHost( *( m_hosts.at( m_indexConnectingHost ) ) );
		}
		else{
			m_indexConnectingHost = -1;
		}
	}
}

void GpsdClient::socketFailure(){
	// socketFailure() does not reconnect immediatly because it would stack connection retries when failure after failure happen. A timer is used to avoid huge stack and bind crash.
	if( m_socketValidated ){
		emit connectionStateChanged( false );
	}
	if( !m_manuallyStopped ){
		QTimer::singleShot( GPSD_SOCKET_FAILURE_WAIT, this, SLOT( socketRetry() ) );
	}
	clearSocket();
}

void GpsdClient::socketError(QAbstractSocket::SocketError error) const{
	// qDebug() << "GpsdClient::socketError" << error; // debug
	QTimer::singleShot( 1, this, SLOT( socketFailure() ) );
	( void )error;
}

QString const GpsdClient::s_jsonKeyClass = "class";
QString const GpsdClient::s_jsonKeyDevice = "device";
QString const GpsdClient::s_jsonKeyStatus = "status";
QString const GpsdClient::s_jsonKeyMode = "mode";
QString const GpsdClient::s_jsonKeyTime = "time";
QString const GpsdClient::s_jsonKeyEpt = "ept";
QString const GpsdClient::s_jsonKeyLat = "lat";
QString const GpsdClient::s_jsonKeyLon = "lon";
QString const GpsdClient::s_jsonKeyAlt = "alt";
QString const GpsdClient::s_jsonKeyEpx = "epx";
QString const GpsdClient::s_jsonKeyEpy = "epy";
QString const GpsdClient::s_jsonKeyEpv = "epv";
QString const GpsdClient::s_jsonKeyTrack = "track";
QString const GpsdClient::s_jsonKeySpeed = "speed";
QString const GpsdClient::s_jsonKeyClimb = "climb";
QString const GpsdClient::s_jsonKeyEpd = "epd";
QString const GpsdClient::s_jsonKeyEps = "eps";
QString const GpsdClient::s_jsonKeyEpc = "epc";
QString const GpsdClient::s_jsonKeyRelayTime = "relaytime";
QString const GpsdClient::s_jsonValueClassTpv = "TPV";
char const GpsdClient::s_javascriptTpvObjectFormat[] = "{class:\"TPV\",device:%s,status:%s,mode:%s,time:%s,ept:%s,lat:%s,lon:%s,alt:%s,epx:%s,epy:%s,epv:%s,track:%s,speed:%s,climb:%s,epd:%s,eps:%s,epc:%s,relaytime:%s}";

void GpsdClient::incomingData(){
	if( !m_socketValidated ){ // for UDP
		linkValidated();
		signalSocketStateChanged();
	}
	m_expectedAnswers = 0;
	m_lastAnswer = QDateTime::currentMSecsSinceEpoch();
	
	m_bufferIn.append( m_socket->readAll() );
	while( m_bufferIn.size()>0 ){
		// WARNING: This code has not been tested under unexpected incoming traffic type!
		char chr, chr2;
		// Bypass bad characters before JSON object:
		int offset = 0;
		for( ; offset<m_bufferIn.size(); ++offset ){
			chr = m_bufferIn[offset];
			if( chr=='{' ) break;
		}
		// Finished loop: offset equals m_bufferIn.size()
		//   Broken loop: offset is lower than m_bufferIn.size()
		if( offset>GPSD_RECEIVE_BUFFER_HALF && offset==m_bufferIn.size() ){ // cursor over buffer
			m_bufferIn.clear(); // bad data overflow
			return;
		}
		// Measure length of JSON object:
		int length = 0;
		for( ; length+offset<m_bufferIn.size(); ++length ){
			chr = m_bufferIn[length+offset];
			if( chr=='}' ){ // end of JSON object
				if( length+offset+1==m_bufferIn.size() ){ // no next character
					break;
				}
				else{
					chr2 = m_bufferIn[length+offset+1];
					if( chr2==0x0D || chr2==0x0A ){ // EOL
						break;
					}
				}
			}
		}
		// Finished loop: length+offset equals m_bufferIn.size()
		//   Broken loop: length+offset is lower than m_bufferIn.size()
		if( length+offset==m_bufferIn.size() ){ // cursor over buffer
			if( length+offset>GPSD_RECEIVE_BUFFER_HALF ){
				m_bufferIn.clear(); // bad data overflow
			}
			return; // incomplete message or bad data overflow
		}
		++length; // count last brace (include last tested character)
		// Remove EOL characters:
		int eolLen = 0;
		for( ; length+offset+eolLen<m_bufferIn.size(); ++eolLen ){
			chr = m_bufferIn[length+offset+eolLen];
			if( chr!=0x0D && chr!=0x0A ){
				break; // pending message in buffer
			}
		}
		// Finished loop: length+offset+eolLen equals m_bufferIn.size()
		//   Broken loop: length+offset+eolLen is lower than m_bufferIn.size()
		// Extract JSON object:
		QJsonDocument jsonDocument = QJsonDocument::fromJson( m_bufferIn.mid( offset, length ) );
		// Remove message from incoming buffer:
		if( length+offset+eolLen==m_bufferIn.size() ){
			m_bufferIn.clear();
		}
		else{
			m_bufferIn.remove( 0, length+offset+eolLen ); // inefficient
		}
		// Treat received JSON object:
		if( jsonDocument.isObject() ){
			QJsonObject jsonObject = jsonDocument.object();
			QJsonValue jsonClass = jsonObject[s_jsonKeyClass];
			if( !jsonClass.isUndefined() && jsonClass.isString() ){
				if( jsonClass.toString()==s_jsonValueClassTpv ){
					m_dataTpv.ClientTime = m_lastAnswer;
					m_dataTpv.NotNullFields = 0;
					QJsonValue jsonValue;
					// Parse every field:
					// Presence is checked to avoid creating a null field.
					if( jsonObject.contains( s_jsonKeyDevice ) ){
						jsonValue = jsonObject[s_jsonKeyDevice];
						if( jsonValue.isString() ){
							m_dataTpv.device = jsonValue.toString();
							GPSD_TPV_UNSET_NULL( m_dataTpv, GPSD_TPV_NULL_DEVICE );
						}
					}
					if( jsonObject.contains( s_jsonKeyStatus ) ){
						jsonValue = jsonObject[s_jsonKeyStatus];
						if( jsonValue.isDouble() ){
							m_dataTpv.status = jsonValue.toInt();
							GPSD_TPV_UNSET_NULL( m_dataTpv, GPSD_TPV_NULL_STATUS );
						}
					}
					if( jsonObject.contains( s_jsonKeyMode ) ){
						jsonValue = jsonObject[s_jsonKeyMode];
						if( jsonValue.isDouble() ){
							m_dataTpv.mode = ( GpsdClient::TpvMode )jsonValue.toInt();
							GPSD_TPV_UNSET_NULL( m_dataTpv, GPSD_TPV_NULL_MODE );
						}
					}
					if( jsonObject.contains( s_jsonKeyTime ) ){
						jsonValue = jsonObject[s_jsonKeyTime];
						if( jsonValue.isString() ){
							m_dataTpv.time = jsonValue.toString();
							GPSD_TPV_UNSET_NULL( m_dataTpv, GPSD_TPV_NULL_TIME );
						}
					}
					if( jsonObject.contains( s_jsonKeyLat ) ){
						jsonValue = jsonObject[s_jsonKeyLat];
						if( jsonValue.isDouble() ){
							m_dataTpv.lat = jsonValue.toDouble();
							GPSD_TPV_UNSET_NULL( m_dataTpv, GPSD_TPV_NULL_LAT );
						}
					}
					if( jsonObject.contains( s_jsonKeyLon ) ){
						jsonValue = jsonObject[s_jsonKeyLon];
						if( jsonValue.isDouble() ){
							m_dataTpv.lon = jsonValue.toDouble();
							GPSD_TPV_UNSET_NULL( m_dataTpv, GPSD_TPV_NULL_LON );
						}
					}
					if( jsonObject.contains( s_jsonKeyAlt ) ){
						jsonValue = jsonObject[s_jsonKeyAlt];
						if( jsonValue.isDouble() ){
							m_dataTpv.alt = jsonValue.toDouble();
							GPSD_TPV_UNSET_NULL( m_dataTpv, GPSD_TPV_NULL_ALT );
						}
					}
					if( jsonObject.contains( s_jsonKeyTrack ) ){
						jsonValue = jsonObject[s_jsonKeyTrack];
						if( jsonValue.isDouble() ){
							m_dataTpv.track = jsonValue.toDouble();
							GPSD_TPV_UNSET_NULL( m_dataTpv, GPSD_TPV_NULL_TRACK );
						}
					}
					if( jsonObject.contains( s_jsonKeySpeed ) ){
						jsonValue = jsonObject[s_jsonKeySpeed];
						if( jsonValue.isDouble() ){
							m_dataTpv.speed = jsonValue.toDouble();
							GPSD_TPV_UNSET_NULL( m_dataTpv, GPSD_TPV_NULL_SPEED );
						}
					}
					if( jsonObject.contains( s_jsonKeyClimb ) ){
						jsonValue = jsonObject[s_jsonKeyClimb];
						if( jsonValue.isDouble() ){
							m_dataTpv.climb = jsonValue.toDouble();
							GPSD_TPV_UNSET_NULL( m_dataTpv, GPSD_TPV_NULL_CLIMB );
						}
					}
					if( jsonObject.contains( s_jsonKeyEpt ) ){
						jsonValue = jsonObject[s_jsonKeyEpt];
						if( jsonValue.isDouble() ){
							m_dataTpv.ept = jsonValue.toDouble();
							GPSD_TPV_UNSET_NULL( m_dataTpv, GPSD_TPV_NULL_EPT );
						}
					}
					if( jsonObject.contains( s_jsonKeyEpx ) ){
						jsonValue = jsonObject[s_jsonKeyEpx];
						if( jsonValue.isDouble() ){
							m_dataTpv.epx = jsonValue.toDouble();
							GPSD_TPV_UNSET_NULL( m_dataTpv, GPSD_TPV_NULL_EPX );
						}
					}
					if( jsonObject.contains( s_jsonKeyEpy ) ){
						jsonValue = jsonObject[s_jsonKeyEpy];
						if( jsonValue.isDouble() ){
							m_dataTpv.epy = jsonValue.toDouble();
							GPSD_TPV_UNSET_NULL( m_dataTpv, GPSD_TPV_NULL_EPY );
						}
					}
					if( jsonObject.contains( s_jsonKeyEpv ) ){
						jsonValue = jsonObject[s_jsonKeyEpv];
						if( jsonValue.isDouble() ){
							m_dataTpv.epv = jsonValue.toDouble();
							GPSD_TPV_UNSET_NULL( m_dataTpv, GPSD_TPV_NULL_EPV );
						}
					}
					if( jsonObject.contains( s_jsonKeyEpd ) ){
						jsonValue = jsonObject[s_jsonKeyEpd];
						if( jsonValue.isDouble() ){
							m_dataTpv.epd = jsonValue.toDouble();
							GPSD_TPV_UNSET_NULL( m_dataTpv, GPSD_TPV_NULL_EPD );
						}
					}
					if( jsonObject.contains( s_jsonKeyEps ) ){
						jsonValue = jsonObject[s_jsonKeyEps];
						if( jsonValue.isDouble() ){
							m_dataTpv.eps = jsonValue.toDouble();
							GPSD_TPV_UNSET_NULL( m_dataTpv, GPSD_TPV_NULL_EPS );
						}
					}
					if( jsonObject.contains( s_jsonKeyEpc ) ){
						jsonValue = jsonObject[s_jsonKeyEpc];
						if( jsonValue.isDouble() ){
							m_dataTpv.epc = jsonValue.toDouble();
							GPSD_TPV_UNSET_NULL( m_dataTpv, GPSD_TPV_NULL_EPC );
						}
					}
					// Prepare JSON if necessary:
					if( m_httpEnabled || m_jsonEnabled ){ // can be improved: on-demand when HTTP only
						// Add extra fields to the JSON object:
						jsonObject.insert( s_jsonKeyRelayTime, QJsonValue( m_dataTpv.ClientTime ) );
						// Apply modifications:
						jsonDocument.setObject( jsonObject );
						// Cache the JSON object:
						m_dataTpvJson = jsonDocument.toJson( QJsonDocument::Indented );
						// Save JSON file if requested:
						if( m_outputJson ){
							m_outputJson->seek( 0 );
							m_outputJson->resize( 0 );
							m_outputJson->write( m_dataTpvJson );
							m_outputJson->flush();
						}
					}
					// Prepare JavaScript if necessary:
					if( m_httpEnabled || m_javascriptEnabled ){ // can be improved: on-demand when HTTP only
						// Format the JS object:
						char javascriptObject[GPSD_JS_MAX_LENGTH];
						QByteArray serializeDevice, serializeStatus, serializeMode, serializeTime, serializeEpt, serializeLat, serializeLon, serializeAlt, serializeEpx, serializeEpy, serializeEpv, serializeTrack, serializeSpeed, serializeClimb, serializeEpd, serializeEps, serializeEpc, serializeClientTime;
						COMPAT_SNPRINTF( javascriptObject, GPSD_JS_MAX_LENGTH, s_javascriptTpvObjectFormat )
							serializeJavascriptField( serializeDevice, m_dataTpv.device, GPSD_TPV_NULL_DEVICE ),
							serializeJavascriptField( serializeStatus, m_dataTpv.status, GPSD_TPV_NULL_STATUS ),
							serializeJavascriptField( serializeMode, m_dataTpv.mode, GPSD_TPV_NULL_MODE ),
							serializeJavascriptField( serializeTime, m_dataTpv.time, GPSD_TPV_NULL_TIME ),
							serializeJavascriptField( serializeEpt, m_dataTpv.ept, GPSD_TPV_NULL_EPT ),
							serializeJavascriptField( serializeLat, m_dataTpv.lat, GPSD_TPV_NULL_LAT ),
							serializeJavascriptField( serializeLon, m_dataTpv.lon, GPSD_TPV_NULL_LON ),
							serializeJavascriptField( serializeAlt, m_dataTpv.alt, GPSD_TPV_NULL_ALT ),
							serializeJavascriptField( serializeEpx, m_dataTpv.epx, GPSD_TPV_NULL_EPX ),
							serializeJavascriptField( serializeEpy, m_dataTpv.epy, GPSD_TPV_NULL_EPY ),
							serializeJavascriptField( serializeEpv, m_dataTpv.epv, GPSD_TPV_NULL_EPV ),
							serializeJavascriptField( serializeTrack, m_dataTpv.track, GPSD_TPV_NULL_TRACK ),
							serializeJavascriptField( serializeSpeed, m_dataTpv.speed, GPSD_TPV_NULL_SPEED ),
							serializeJavascriptField( serializeClimb, m_dataTpv.climb, GPSD_TPV_NULL_CLIMB ),
							serializeJavascriptField( serializeEpd, m_dataTpv.epd, GPSD_TPV_NULL_EPD ),
							serializeJavascriptField( serializeEps, m_dataTpv.eps, GPSD_TPV_NULL_EPS ),
							serializeJavascriptField( serializeEpc, m_dataTpv.epc, GPSD_TPV_NULL_EPC ),
							serializeJavascriptField( serializeClientTime, m_dataTpv.ClientTime )
						);
						if( m_httpEnabled ){
							m_dataTpvJavascriptHttp.clear();
							if( m_httpFunction.isEmpty() ){
								m_dataTpvJavascriptHttp.append( javascriptObject );
							}
							else{
								m_dataTpvJavascriptHttp.append( m_httpFunction );
								m_dataTpvJavascriptHttp.append( '(' );
								m_dataTpvJavascriptHttp.append( javascriptObject );
								m_dataTpvJavascriptHttp.append( ')' );
								m_dataTpvJavascriptHttp.append( ';' );
							}
						}
						if( m_javascriptEnabled && m_outputJavascript ){
							QByteArray dataTpvJavascriptFile;
							if( m_javascriptFunction.isEmpty() ){
								dataTpvJavascriptFile.append( javascriptObject );
							}
							else{
								dataTpvJavascriptFile.append( m_javascriptFunction );
								dataTpvJavascriptFile.append( '(' );
								dataTpvJavascriptFile.append( javascriptObject );
								dataTpvJavascriptFile.append( ')' );
								dataTpvJavascriptFile.append( ';' );
							}
							// Save JavaScript file if requested:
							m_outputJavascript->seek( 0 );
							m_outputJavascript->resize( 0 );
							m_outputJavascript->write( dataTpvJavascriptFile );
							m_outputJavascript->flush();
						}
					}
					// Signal the update:
					emit dataTpvChanged( m_dataTpv );
				}
			}
		}
	}
}

char const GpsdClient::s_javascriptNull[] = "null";
QString const GpsdClient::s_javascriptUnsafeCRLF = "\x0D\x0A";
QChar const GpsdClient::s_javascriptUnsafeDoubleQuote = '"';
QChar const GpsdClient::s_javascriptUnsafeAntiSlash = '\\';
QChar const GpsdClient::s_javascriptUnsafeCR = '\x0D';
QChar const GpsdClient::s_javascriptUnsafeLF = '\x0A';
QString const GpsdClient::s_javascriptEscapedCRLF = "\\n";
QString const GpsdClient::s_javascriptEscapedDoubleQuote = "\\\"";
QString const GpsdClient::s_javascriptEscapedAntiSlash = "\\\\";
QString const GpsdClient::s_javascriptEscapedCR = "\\r";
QString const GpsdClient::s_javascriptEscapedLF = "\\n";

const char* GpsdClient::serializeJavascriptField(QByteArray& out, QString fieldValue, quint32 field) const{
	if( field && GPSD_TPV_FIELD_IS_NULL( m_dataTpv, field ) ) return s_javascriptNull;
	else{ // This escape process should be improved.
		fieldValue.replace( s_javascriptUnsafeCRLF, s_javascriptEscapedCRLF );
		fieldValue.replace( s_javascriptUnsafeDoubleQuote, s_javascriptEscapedDoubleQuote );
		fieldValue.replace( s_javascriptUnsafeAntiSlash, s_javascriptEscapedAntiSlash );
		fieldValue.replace( s_javascriptUnsafeCR, s_javascriptEscapedCR );
		fieldValue.replace( s_javascriptUnsafeLF, s_javascriptEscapedLF );
		out.clear();
		out.append( '"' );
		out.append( fieldValue.toUtf8() );
		out.append( '"' );
		return out.constData();
	}
}
const char* GpsdClient::serializeJavascriptField(QByteArray& out, double fieldValue, quint32 field) const{
	if( field && GPSD_TPV_FIELD_IS_NULL( m_dataTpv, field ) ) return s_javascriptNull;
	else{
		out.clear();
		out.append( QString::number( fieldValue, 'f', GPSD_JS_PRECISION ).toLatin1() );
		return out.constData();
	}
	
}
const char* GpsdClient::serializeJavascriptField(QByteArray& out, int fieldValue, quint32 field) const{
	if( field && GPSD_TPV_FIELD_IS_NULL( m_dataTpv, field ) ) return s_javascriptNull;
	else{
		out.clear();
		out.append( QString::number( fieldValue ).toLatin1() );
		return out.constData();
	}
}
const char* GpsdClient::serializeJavascriptField(QByteArray& out, GpsdClient::TpvMode fieldValue, quint32 field) const{
	if( field && GPSD_TPV_FIELD_IS_NULL( m_dataTpv, field ) ) return s_javascriptNull;
	else{
		out.clear();
		out.append( QString::number( fieldValue ).toLatin1() );
		return out.constData();
	}
}
const char* GpsdClient::serializeJavascriptField(QByteArray& out, quint32 fieldValue, quint32 field) const{
	if( field && GPSD_TPV_FIELD_IS_NULL( m_dataTpv, field ) ) return s_javascriptNull;
	else{
		out.clear();
		out.append( QString::number( fieldValue ).toLatin1() );
		return out.constData();
	}
}
const char* GpsdClient::serializeJavascriptField(QByteArray& out, qint64 fieldValue, quint32 field) const{
	if( field && GPSD_TPV_FIELD_IS_NULL( m_dataTpv, field ) ) return s_javascriptNull;
	else{
		out.clear();
		out.append( QString::number( fieldValue ).toLatin1() );
		return out.constData();
	}
}
