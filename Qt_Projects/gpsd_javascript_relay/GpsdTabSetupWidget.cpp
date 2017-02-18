#include "GpsdTabSetupWidget.h"

#define PROTO_FROM_PROTO_INDEX( protocol, symmetric, index ) \
	switch( index ){ \
		case 2: \
			protocol = QAbstractSocket::UdpSocket; \
			symmetric = true; \
		break; \
		case 1: \
			protocol = QAbstractSocket::UdpSocket; \
		break; \
		default: \
			protocol = QAbstractSocket::TcpSocket; \
	}

GpsdTabSetupWidget::GpsdTabSetupWidget(GpsdClient* gpsdClient, QTabWidget* sourcesTabs, QWidget *parent):
	QWidget( parent ),
	m_gpsdClient( gpsdClient ),
	m_gSourcesTabs( sourcesTabs ),
	m_hasChanged( false )
{
	setupUi( this );
	connect( m_gName, SIGNAL( textChanged(const QString&) ), this, SLOT( updateTabText(QString const&) ) );
	connect( m_gName, SIGNAL( textChanged(const QString&) ), this, SLOT( flagHasChanged() ) );
	connect( m_gRefreshRate, SIGNAL( valueChanged(double) ), this, SLOT( flagHasChanged() ) );
	connect( m_gAutoReconnect, SIGNAL( stateChanged(int) ), this, SLOT( flagHasChanged() ) );
	connect( m_btnAddHost, SIGNAL( clicked() ), this, SLOT( flagHasChanged() ) );
	connect( m_gSourcesTabs, SIGNAL( tabCloseRequested(int) ), this, SLOT( tabCloseRequested(int) ) );
}

GpsdClient* GpsdTabSetupWidget::getGpsdClient(){
	return m_gpsdClient;
}

void GpsdTabSetupWidget::resetHasChanged(){ m_hasChanged = false; }
bool GpsdTabSetupWidget::getHasChanged() const{ return m_hasChanged; }
void GpsdTabSetupWidget::flagHasChanged(){ m_hasChanged = true; }

int GpsdTabSetupWidget::getNumHosts() const{
	return m_gHostList->count(); // number of hosts in GUI, not in GPSD client (change this if needed)
}

void GpsdTabSetupWidget::updateTabText(QString const& text){
	if( m_gSourcesTabs ){
		int index = m_gSourcesTabs->indexOf( this );
		if( index!=-1 ){
			m_gSourcesTabs->setTabText( index, text );
		}
	}
}

GpsdHostWidget* GpsdTabSetupWidget::addHost(){
	GpsdHostWidget* host = new GpsdHostWidget( this );
	connect( host->m_gHostName, SIGNAL( textChanged(const QString&) ), this, SLOT( flagHasChanged() ) );
	connect( host->m_gPort, SIGNAL( valueChanged(int) ), this, SLOT( flagHasChanged() ) );
	connect( host->m_gProtocol, SIGNAL( currentIndexChanged(int) ), this, SLOT( flagHasChanged() ) );
	connect( host->m_btnRemove, SIGNAL( clicked() ), this, SLOT( flagHasChanged() ) );
	m_gHostList->addWidget( host );
	return host;
}
GpsdHostWidget* GpsdTabSetupWidget::addHost(QString const& hostName, quint16 const& port, QAbstractSocket::SocketType const& protocol, bool symmetric){
	GpsdHostWidget* host = addHost();
	host->m_gHostName->setText( hostName );
	host->m_gPort->setValue( port );
	if( protocol==QAbstractSocket::UdpSocket ){
		if( symmetric ) host->m_gProtocol->setCurrentIndex( 2 );
		else host->m_gProtocol->setCurrentIndex( 1 );
	}
	return host;
}
void GpsdTabSetupWidget::on_m_btnAddHost_clicked(){
	addHost();
}

void GpsdTabSetupWidget::applyConfiguration(){
	resetHasChanged();
	if( m_gpsdClient ){
		m_gpsdClient->setName( m_gName->text() );
		m_gpsdClient->setRefreshRate( m_gRefreshRate->value() );
		m_gpsdClient->setAutoReconnect( m_gAutoReconnect->isChecked() );
		m_gpsdClient->clearHosts();
		for( int i=0; i<m_gHostList->count(); ++i ){
			QLayoutItem* item = m_gHostList->itemAt( i );
			if( item ){
				GpsdHostWidget* host = qobject_cast<GpsdHostWidget*>( item->widget() );
				if( host ){
					QAbstractSocket::SocketType protocol;
					bool symmetric = false;
					int index = host->m_gProtocol->currentIndex();
					PROTO_FROM_PROTO_INDEX( protocol, symmetric, index );
					m_gpsdClient->addHost(
						host->m_gHostName->text(),
						host->m_gPort->value(),
						protocol,
						symmetric
					);
				}
			}
		}
	}
}

void GpsdTabSetupWidget::getConfiguration(struct GpsdTabSetupWidget::Configuration& c) const{
	c.name = m_gName->text();
	c.refreshRate = m_gRefreshRate->value();
	c.autoConnect = m_gAutoConnect->isChecked();
	c.autoReconnect = m_gAutoReconnect->isChecked();
	for( int i=0; i<m_gHostList->count(); ++i ){
		QLayoutItem* item = m_gHostList->itemAt( i );
		if( item ){
			GpsdHostWidget* host = qobject_cast<GpsdHostWidget*>( item->widget() );
			if( host ){
				struct ConfigurationHost c_host;
					QAbstractSocket::SocketType protocol;
					bool symmetric = false;
					int index = host->m_gProtocol->currentIndex();
					PROTO_FROM_PROTO_INDEX( protocol, symmetric, index );
					c_host.hostName = host->m_gHostName->text();
					c_host.port = host->m_gPort->value();
					c_host.protocol = protocol;
					c_host.symmetric = symmetric;
				c.hosts.append( c_host );
			}
		}
	}
}

void GpsdTabSetupWidget::setConfiguration(QJsonObject const& gpsdCSettings){
	if( gpsdCSettings.contains( "name" ) )
		m_gName->setText( gpsdCSettings.value( "name" ).toString() );
	if( gpsdCSettings.contains( "refreshRate" ) )
		m_gRefreshRate->setValue( gpsdCSettings.value( "refreshRate" ).toDouble() );
	if( gpsdCSettings.contains( "autoConnect" ) )
		m_gAutoConnect->setChecked( gpsdCSettings.value( "autoConnect" ).toBool() );
	if( gpsdCSettings.contains( "autoReconnect" ) )
		m_gAutoReconnect->setChecked( gpsdCSettings.value( "autoReconnect" ).toBool() );
	if( gpsdCSettings.contains( "hosts" ) ){
		QJsonArray hostsJson = gpsdCSettings.value( "hosts" ).toArray();
		// Clear the hosts list:
		for( int i=m_gHostList->count(); i>=0; --i ){
			QLayoutItem* item = m_gHostList->takeAt( i );
			if( item ){
				QWidget* host = item->widget();
				if( host ){
					host->deleteLater();
				}
				delete item;
			}
		}
		// Add hosts:
		for( QJsonArray::const_iterator hostJson1=hostsJson.begin(); hostJson1!=hostsJson.end(); ++hostJson1 ){
			QJsonObject hostJson = hostJson1->toObject();
			quint16 port = 2947;
			if( hostJson.contains( "port" ) )
				port = hostJson.value( "port" ).toInt();
			QAbstractSocket::SocketType protocol = QAbstractSocket::TcpSocket;
			if( hostJson.contains( "protocol" ) )
				protocol = ( QAbstractSocket::SocketType )hostJson.value( "protocol" ).toInt();
			bool symmetric = false;
			if( hostJson.contains( "symmetric" ) )
				symmetric = hostJson.value( "symmetric" ).toBool();
			addHost(
				hostJson.value( "hostName" ).toString(),
				port,
				protocol,
				symmetric
			);
		}
	}
	applyConfiguration();
}

void GpsdTabSetupWidget::tabCloseRequested(int index){
	if( m_gSourcesTabs->count()>1 ){ // no empty tabs area!
		if( index==m_gSourcesTabs->indexOf( this ) ){
			m_gpsdClient->deleteLater();
		}
	}
}

#undef PROTO_FROM_PROTO_INDEX
