#include "GpsdTabSetupWidget.h"

#define PROTO_FROM_PROTO_INDEX( protocol, symmetric, index ) \
	switch( index ){ \
		case 2: \
			protocol = QAbstractSocket::UdpSocket; \
			symmetric = true; \
		break; \
		case 1: \
			protocol = QAbstractSocket::UdpSocket; \
			symmetric = false; \
		break; \
		default: \
			protocol = QAbstractSocket::TcpSocket; \
			symmetric = false; \
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
	GpsdHostWidget* const host = new GpsdHostWidget( this );
	connect( host->m_gConnectionMethod, SIGNAL( currentIndexChanged(int) ), this, SLOT( flagHasChanged() ) );
	connect( host->m_gHostName, SIGNAL( textChanged(const QString&) ), this, SLOT( flagHasChanged() ) );
	connect( host->m_gPort, SIGNAL( valueChanged(int) ), this, SLOT( flagHasChanged() ) );
	connect( host->m_gProtocol, SIGNAL( currentIndexChanged(int) ), this, SLOT( flagHasChanged() ) );
	connect( host->m_btnRemove, SIGNAL( clicked() ), this, SLOT( flagHasChanged() ) );
	connect( host->m_gSerialBaudRate, SIGNAL( currentIndexChanged(int) ), this, SLOT( flagHasChanged() ) );
	connect( host->m_gSerialDataBits, SIGNAL( currentIndexChanged(int) ), this, SLOT( flagHasChanged() ) );
	connect( host->m_gSerialFlowControl, SIGNAL( currentIndexChanged(int) ), this, SLOT( flagHasChanged() ) );
	connect( host->m_gSerialParity, SIGNAL( currentIndexChanged(int) ), this, SLOT( flagHasChanged() ) );
	connect( host->m_gSerialPortsList, SIGNAL( currentIndexChanged(int) ), this, SLOT( flagHasChanged() ) );
	connect( host->m_gSerialStopBits, SIGNAL( currentIndexChanged(int) ), this, SLOT( flagHasChanged() ) );
	connect( host->m_gSerialLowLatency, SIGNAL( stateChanged(int) ), this, SLOT( flagHasChanged() ) );
	m_gHostList->addWidget( host );
	return host;
}
GpsdHostWidget* GpsdTabSetupWidget::addHost(
	QString const& hostName,
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
	GpsdHostWidget* const host = addHost();
	host->m_gConnectionMethod->setCurrentIndex( connectionMethod );
	host->m_gHostName->setText( hostName );
	host->m_gPort->setValue( port );
	if( protocol==QAbstractSocket::UdpSocket ){
		if( symmetric ) host->m_gProtocol->setCurrentIndex( 2 );
		else host->m_gProtocol->setCurrentIndex( 1 );
	}
	else{
		host->m_gProtocol->setCurrentIndex( 0 );
	}
	host->m_gSerialBaudRate->setCurrentIndex( host->m_gSerialBaudRate->findData( serialBaudRate ) );
	host->m_gSerialDataBits->setCurrentIndex( host->m_gSerialDataBits->findData( serialDataBits ) );
	host->m_gSerialFlowControl->setCurrentIndex( host->m_gSerialFlowControl->findData( serialFlowControl ) );
	host->m_gSerialParity->setCurrentIndex( host->m_gSerialParity->findData( serialParity ) );
	host->m_gSerialStopBits->setCurrentIndex( host->m_gSerialStopBits->findData( serialStopBits ) );
	host->m_gSerialLowLatency->setChecked( serialLowLatency );
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
					bool symmetric;
					int index = host->m_gProtocol->currentIndex();
					PROTO_FROM_PROTO_INDEX( protocol, symmetric, index );
					m_gpsdClient->addHost(
						host->m_gHostName->text(),
						host->m_gPort->value(),
						protocol,
						symmetric,
						host->m_gConnectionMethod->currentIndex(),
						host->m_gSerialBaudRate->currentData().toInt(),
						host->m_gSerialDataBits->currentData().toInt(),
						host->m_gSerialFlowControl->currentData().toInt(),
						host->m_gSerialParity->currentData().toInt(),
						host->m_gSerialStopBits->currentData().toInt(),
						host->m_gSerialLowLatency->isChecked()
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
					bool symmetric;
					int index = host->m_gProtocol->currentIndex();
					PROTO_FROM_PROTO_INDEX( protocol, symmetric, index );
					c_host.connectionMethod = host->m_gConnectionMethod->currentIndex();
					c_host.hostName = host->m_gHostName->text();
					c_host.port = host->m_gPort->value();
					c_host.protocol = protocol;
					c_host.symmetric = symmetric;
					c_host.serialBaudRate = host->m_gSerialBaudRate->currentData().toInt();
					c_host.serialDataBits = host->m_gSerialDataBits->currentData().toInt();
					c_host.serialFlowControl = host->m_gSerialFlowControl->currentData().toInt();
					c_host.serialParity = host->m_gSerialParity->currentData().toInt();
					c_host.serialStopBits = host->m_gSerialStopBits->currentData().toInt();
					c_host.serialLowLatency = host->m_gSerialLowLatency->isChecked();
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
				if( hostJson.contains( "port" ) ) port = hostJson.value( "port" ).toInt();
			QAbstractSocket::SocketType protocol = QAbstractSocket::TcpSocket;
				if( hostJson.contains( "protocol" ) ) protocol = ( QAbstractSocket::SocketType )hostJson.value( "protocol" ).toInt();
			bool symmetric = false;
				if( hostJson.contains( "symmetric" ) ) symmetric = hostJson.value( "symmetric" ).toBool();
			int connectionMethod = 0;
				if( hostJson.contains( "connectionMethod" ) ) connectionMethod = hostJson.value( "connectionMethod" ).toInt();
			qint32 serialBaudRate = -1;
				if( hostJson.contains( "serialBaudRate" ) ) serialBaudRate = hostJson.value( "serialBaudRate" ).toInt();
			int serialDataBits= -1;
				if( hostJson.contains( "serialDataBits" ) ) serialDataBits = hostJson.value( "serialDataBits" ).toInt();
			int serialFlowControl = -1;
				if( hostJson.contains( "serialFlowControl" ) ) serialFlowControl = hostJson.value( "serialFlowControl" ).toInt();
			int serialParity = -1;
				if( hostJson.contains( "serialParity" ) ) serialParity = hostJson.value( "serialParity" ).toInt();
			int serialStopBits = -1;
				if( hostJson.contains( "serialStopBits" ) ) serialStopBits = hostJson.value( "serialStopBits" ).toInt();
			bool serialLowLatency = true;
				if( hostJson.contains( "serialLowLatency" ) ) serialLowLatency = hostJson.value( "serialLowLatency" ).toBool();
			addHost(
				hostJson.value( "hostName" ).toString(),
				port,
				protocol,
				symmetric,
				connectionMethod,
				serialBaudRate,
				serialDataBits,
				serialFlowControl,
				serialParity,
				serialStopBits,
				serialLowLatency
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
