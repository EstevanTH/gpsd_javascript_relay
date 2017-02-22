#include "GpsdStatusWidget.h"

GpsdStatusWidget::GpsdStatusWidget(GpsdClient* gpsdClient, QWidget *parent):
	QFrame( parent ),
	m_gpsdClient( gpsdClient )
{
	// Translation:
	if( s_statusTextStopped.isEmpty() ) s_statusTextStopped = QString( "<span style='color:red;'>" )+tr( "Stopped" )+QString( "</span>" );
	if( s_statusTextHostLookup.isEmpty() ) s_statusTextHostLookup = QString( "<span style='color:orange;'>" )+tr( "Resolving" )+QString( "</span>" );
	if( s_statusTextConnecting.isEmpty() ) s_statusTextConnecting = QString( "<span style='color:orange;'>" )+tr( "Connecting" )+QString( "</span>" );
	if( s_statusTextConnected.isEmpty() ) s_statusTextConnected = QString( "<span style='color:green;'>" )+tr( "Connected" )+QString( "</span>" );
	if( s_statusTextUnconnected.isEmpty() ) s_statusTextUnconnected = QString( "<span style='color:fuchsia;'>" )+tr( "Unconnected" )+QString( "</span>" );
	
	// Initialization:
	setupUi( this );
	connect( gpsdClient, SIGNAL( nameChanged(QString const&) ), m_gName, SLOT( setText(QString const&) ) );
	connect( gpsdClient, SIGNAL( statusChanged(QAbstractSocket::SocketState, QString const&, quint16 const&, QAbstractSocket::SocketType const&) ), this, SLOT( statusChanged(QAbstractSocket::SocketState) ) );
	connect( gpsdClient, SIGNAL( hostChanged(QString const&, quint16 const&, QAbstractSocket::SocketType const&) ), this, SLOT( hostChanged(QString const&, quint16 const&, QAbstractSocket::SocketType const&) ) );
	connect( gpsdClient, SIGNAL( dataTpvChanged(GpsdClient::DataTpv const&) ), this, SLOT( dataTpvChanged(GpsdClient::DataTpv const&) ) );
	connect( m_btnStop, SIGNAL( clicked(bool) ), gpsdClient, SLOT( stopClient() ) );
	connect( m_btnStart, SIGNAL( clicked(bool) ), gpsdClient, SLOT( startClient() ) );
	m_gStatus->setText( s_statusTextStopped );
}

GpsdClient* GpsdStatusWidget::getGpsdClient() const{
	return m_gpsdClient;
}

QString GpsdStatusWidget::s_statusTextStopped;
QString GpsdStatusWidget::s_statusTextHostLookup;
QString GpsdStatusWidget::s_statusTextConnecting;
QString GpsdStatusWidget::s_statusTextConnected;
QString GpsdStatusWidget::s_statusTextUnconnected;
QString const GpsdStatusWidget::s_statusProtocolTcp = " TCP";
QString const GpsdStatusWidget::s_statusProtocolUdp = " UDP";

void GpsdStatusWidget::statusChanged(QAbstractSocket::SocketState status) const{
	QString const* statusText;
	if( m_gpsdClient->isManuallyStopped() ){
		statusText = &s_statusTextStopped;
		m_gHost->setText( s_nullValue );
	}
	else{
		switch( status ){
			case QAbstractSocket::HostLookupState:
				statusText = &s_statusTextHostLookup;
			break;
			case QAbstractSocket::ConnectingState:
				statusText = &s_statusTextConnecting;
			break;
			case QAbstractSocket::ConnectedState:
				statusText = &s_statusTextConnected;
			break;
			default:
				statusText = &s_statusTextUnconnected;
		}
	}
	m_gStatus->setText( *statusText );
}

void GpsdStatusWidget::hostChanged(QString const& hostname, quint16 const& port, QAbstractSocket::SocketType const& protocol){
	QString const* protocolText;
	if( protocol!=QAbstractSocket::UdpSocket ){
		protocolText = &s_statusProtocolTcp;
	}
	else{
		protocolText = &s_statusProtocolUdp;
	}
	m_gHost->setText( hostname+':'+QString::number( port )+( *protocolText ) );
}

QString const GpsdStatusWidget::s_unitDegrees = "°";
QString const GpsdStatusWidget::s_unitMeters = " m";
QString const GpsdStatusWidget::s_unitMetersPerSecond = " m/s";
QString const GpsdStatusWidget::s_unitMilliseconds = " ms";
QString const GpsdStatusWidget::s_unitSeconds = " s";
QString const GpsdStatusWidget::s_tpvModeNoMode = "No mode";
QString const GpsdStatusWidget::s_tpvModeNoFix = "No fix";
QString const GpsdStatusWidget::s_tpvMode2d = "2D";
QString const GpsdStatusWidget::s_tpvMode3d = "3D";
QString const GpsdStatusWidget::s_nullValue = "-";

void GpsdStatusWidget::dataTpvChanged(GpsdClient::DataTpv const& data) const{
	if( GPSD_TPV_FIELD_IS_VALID( data, GPSD_TPV_NULL_DEVICE ) ){
		m_gDevice->setText( data.device );
	}
	else{
		m_gDevice->setText( s_nullValue );
	}
	if( GPSD_TPV_FIELD_IS_VALID( data, GPSD_TPV_NULL_STATUS ) ){
		m_gStatusGps->setText( QString::number( data.status ) );
	}
	else{
		m_gStatusGps->setText( s_nullValue );
	}
	if( GPSD_TPV_FIELD_IS_VALID( data, GPSD_TPV_NULL_MODE ) ){
		QString mode;
		switch( data.mode ){
			case GpsdClient::no_fix:
				mode = s_tpvModeNoFix;
			break;
			case GpsdClient::_2D:
				mode = s_tpvMode2d;
			break;
			case GpsdClient::_3D:
				mode = s_tpvMode3d;
			break;
			default:
				mode = s_tpvModeNoMode;
		}
		m_gMode->setText( mode );
	}
	else{
		m_gMode->setText( s_nullValue );
	}
	if( GPSD_TPV_FIELD_IS_VALID( data, GPSD_TPV_NULL_TIME ) ){
		m_gTime->setText( data.time );
	}
	else{
		m_gTime->setText( s_nullValue );
	}
	if( GPSD_TPV_FIELD_IS_VALID( data, GPSD_TPV_NULL_LAT ) ){
		m_gLat->setText( QString::number( data.lat, 'f', 6 )+s_unitDegrees );
	}
	else{
		m_gLat->setText( s_nullValue );
	}
	if( GPSD_TPV_FIELD_IS_VALID( data, GPSD_TPV_NULL_LON ) ){
		m_gLon->setText( QString::number( data.lon, 'f', 6 )+s_unitDegrees );
	}
	else{
		m_gLon->setText( s_nullValue );
	}
	if( GPSD_TPV_FIELD_IS_VALID( data, GPSD_TPV_NULL_ALT ) ){
		m_gAlt->setText( QString::number( data.alt, 'f', 2 )+s_unitMeters );
	}
	else{
		m_gAlt->setText( s_nullValue );
	}
	if( GPSD_TPV_FIELD_IS_VALID( data, GPSD_TPV_NULL_TRACK ) ){
		m_gTrack->setText( QString::number( data.track, 'f', 2 )+s_unitDegrees );
	}
	else{
		m_gTrack->setText( s_nullValue );
	}
	if( GPSD_TPV_FIELD_IS_VALID( data, GPSD_TPV_NULL_SPEED ) ){
		m_gSpeed->setText( QString::number( data.speed, 'f', 2 )+s_unitMetersPerSecond );
	}
	else{
		m_gSpeed->setText( s_nullValue );
	}
	if( GPSD_TPV_FIELD_IS_VALID( data, GPSD_TPV_NULL_CLIMB ) ){
		m_gClimb->setText( QString::number( data.climb, 'f', 3 )+s_unitMetersPerSecond );
	}
	else{
		m_gClimb->setText( s_nullValue );
	}
	if( GPSD_TPV_FIELD_IS_VALID( data, GPSD_TPV_NULL_EPT ) ){
		m_gEpt->setText( QString::number( data.ept*1000, 'f', 3 )+s_unitMilliseconds );
	}
	else{
		m_gEpt->setText( s_nullValue );
	}
	if( GPSD_TPV_FIELD_IS_VALID( data, GPSD_TPV_NULL_EPX ) ){
		m_gEpx->setText( QString::number( data.epx, 'f', 2 )+s_unitMeters );
	}
	else{
		m_gEpx->setText( s_nullValue );
	}
	if( GPSD_TPV_FIELD_IS_VALID( data, GPSD_TPV_NULL_EPY ) ){
		m_gEpy->setText( QString::number( data.epy, 'f', 2 )+s_unitMeters );
	}
	else{
		m_gEpy->setText( s_nullValue );
	}
	if( GPSD_TPV_FIELD_IS_VALID( data, GPSD_TPV_NULL_EPV ) ){
		m_gEpv->setText( QString::number( data.epv, 'f', 2 )+s_unitMeters );
	}
	else{
		m_gEpv->setText( s_nullValue );
	}
	if( GPSD_TPV_FIELD_IS_VALID( data, GPSD_TPV_NULL_EPD ) ){
		m_gEpd->setText( QString::number( data.epd, 'f', 2 )+s_unitDegrees );
	}
	else{
		m_gEpd->setText( s_nullValue );
	}
	if( GPSD_TPV_FIELD_IS_VALID( data, GPSD_TPV_NULL_EPS ) ){
		m_gEps->setText( QString::number( data.eps, 'f', 3 )+s_unitMetersPerSecond );
	}
	else{
		m_gEps->setText( s_nullValue );
	}
	if( GPSD_TPV_FIELD_IS_VALID( data, GPSD_TPV_NULL_EPC ) ){
		m_gEpc->setText( QString::number( data.epc, 'f', 3 )+s_unitMetersPerSecond );
	}
	else{
		m_gEpc->setText( s_nullValue );
	}
}

void GpsdStatusWidget::socketException(const char* description) const{
	m_gStatus->setText( QString( "<span style='color:red;'>" )+QString::fromLatin1( description )+QString( "</span>" ) );
}
