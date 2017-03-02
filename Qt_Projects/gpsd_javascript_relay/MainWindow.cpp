#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent):
	QWidget( parent ),
	m_httpSetupChanged( false ),
	m_trayIcon( 0 ),
	m_loggingOut( false )
{
	// Translation:
	if( s_messageConnected.isEmpty() ) s_messageConnected = tr( "Connection established" );
	if( s_messageConnectionLost.isEmpty() ) s_messageConnectionLost = tr( "Connection lost" );
	
	// Main interface:
	setupUi( this );
	setWindowTitle( Application::getAppTitle()+' '+Application::getVersion() );
	m_gBuild->setText( Application::getVersion()+QString( " (" )+QString::number( sizeof( void* )<<3 )+tr( "-bit)" ) );
	m_btnGpsdNewTab = new QPushButton( m_gSourcesTabs );
		m_btnGpsdNewTab->setText( "" );
		m_btnGpsdNewTab->setIcon( QIcon( ":/icon16_add.png" ) );
		m_btnGpsdNewTab->setToolTip( tr( "Add a GPSD server" ) );
		connect( m_btnGpsdNewTab, SIGNAL( clicked() ), this, SLOT( makeNewGpsdClient() ) );
		m_gSourcesTabs->setCornerWidget( m_btnGpsdNewTab );
	connect( App, SIGNAL( addedGpsdClient(GpsdClient*) ), this, SLOT( addedGpsdClient(GpsdClient*) ) );
	connect( App, SIGNAL( removedGpsdClient(GpsdClient*) ), this, SLOT( removedGpsdClient(GpsdClient*) ) );
	connect( m_btnStatus, SIGNAL( clicked() ), this, SLOT( selectCategory() ) );
	connect( m_btnSources, SIGNAL( clicked() ), this, SLOT( selectCategory() ) );
	connect( m_btnTargets, SIGNAL( clicked() ), this, SLOT( selectCategory() ) );
	connect( m_btnAbout, SIGNAL( clicked() ), this, SLOT( selectCategory() ) );
	QGuiApplication::setFallbackSessionManagementEnabled( false ); // logout handling
	connect( App, SIGNAL( commitDataRequest(QSessionManager) ), SLOT( prepareLogout(QSessionManager) ) ); // logout handling
	
	// Load configuration:
	QJsonObject settings = Application::getAppSettings();
		QJsonObject httpServerSettings = settings.value( "HttpServer" ).toObject();
		QJsonArray gpsdClients = settings.value( "GpsdClients" ).toArray();
	
	// HTTP server
	// Apply configuration:
	if( httpServerSettings.contains( "address" ) )
		m_gHttpSetIp->setText( httpServerSettings.value( "address" ).toString() );
	if( httpServerSettings.contains( "port" ) )
		m_gHttpSetPort->setValue( httpServerSettings.value( "port" ).toInt() );
	if( httpServerSettings.contains( "autostart" ) )
		m_gHttpAutoStart->setChecked( httpServerSettings.value( "autostart" ).toBool() );
	// Prepare:
	bool httpAutoStart = m_gHttpAutoStart->isChecked();
	HttpServerSetup httpSetup;
		httpSetup.address = m_gHttpSetIp->text();
		httpSetup.port = m_gHttpSetPort->value();
	connect( m_gHttpSetIp, SIGNAL( textChanged(const QString) ), this, SLOT( flagHttpSetupChanged() ) );
	connect( m_gHttpSetPort, SIGNAL( valueChanged(int) ), this, SLOT( flagHttpSetupChanged() ) );
	HttpServer* httpServer = new HttpServer( httpSetup, App );
	updateHttpStatus( httpServer->getStatus() );
	connect( httpServer, SIGNAL( requestsUpdated(quint64 const&) ), this, SLOT( updateHttpRequests(quint64 const&) ) );
	connect( httpServer, SIGNAL( statusUpdated(HttpServerStatus const&) ), this, SLOT( updateHttpStatus(HttpServerStatus const&) ) );
	connect( m_btnHttpStart, SIGNAL( clicked() ), httpServer, SLOT( startServer() ) );
	connect( m_btnHttpStop, SIGNAL( clicked() ), httpServer, SLOT( stopServer() ) );
	if( httpAutoStart && !APP_CLO_NOAUTOSTART ){
		httpServer->startServer();
	}
	
	// GPSD client
	// Create clients:
	if( gpsdClients.size()>0 ){
		for( int i=0; i<gpsdClients.size(); ++i ){
			makeNewGpsdClient();
		}
	}
	else{
		makeNewGpsdClient();
	}
	// Apply configuration:
	if( gpsdClients.size()>0 ){
		GpsdTabSetupWidget* setup;
		TargetTabSetupWidget* targets;
		unsigned int i=0;
		for( QJsonArray::const_iterator gpsdCSettings1=gpsdClients.begin(); gpsdCSettings1!=gpsdClients.end(); ++gpsdCSettings1 ){
			QJsonObject gpsdCSettings = gpsdCSettings1->toObject();
			setup = qobject_cast<GpsdTabSetupWidget*>( m_gSourcesTabs->widget( i ) );
			targets = qobject_cast<TargetTabSetupWidget*>( m_gTargetsTabs->widget( i ) );
			if( !setup || !targets ) continue;
			setup->setConfiguration( gpsdCSettings );
			targets->setConfiguration( gpsdCSettings );
			if( gpsdCSettings.contains( "autoConnect" ) && gpsdCSettings.value( "autoConnect" ).toBool() && !APP_CLO_NOAUTOSTART )
				setup->getGpsdClient()->startClient();
			++i;
		}
	}
	
	// Restore window when new instance run:
	connect( App, SIGNAL( seenNewInstance() ), this, SLOT( restoredMinimizedInTray() ) );
	
	// Start minimized if requested:
	if( APP_CLO_MINIMIZED ) minimizeInTray();
}

void MainWindow::addedGpsdClient(GpsdClient* gpsdClient){
	GpsdTabSetupWidget* setup = new GpsdTabSetupWidget( gpsdClient, m_gSourcesTabs );
		setup->addHost( "192.168.43.1" );
		setup->addHost( "192.168.42.129" );
		m_gSourcesTabs->addTab( setup, "Client #"+QString::number( m_gSourcesTabs->count() ) );
	TargetTabSetupWidget* targets = new TargetTabSetupWidget( gpsdClient, m_gTargetsTabs );
		m_gTargetsTabs->addTab( targets, "Client #"+QString::number( m_gTargetsTabs->count() ) );
	GpsdStatusWidget* monitor = new GpsdStatusWidget( gpsdClient );
		m_gGpsdClientMonitorsLayout->addWidget( monitor );
	targets->applyConfiguration(); // for startup: flagHasChanged() not triggered before 1st display
	setup->applyConfiguration(); // for startup: flagHasChanged() not triggered before 1st display
	connect( gpsdClient, SIGNAL( connectionStateChanged(bool) ), this, SLOT( gpsdConnectionStateChanged(bool) ) );
	connect( gpsdClient, SIGNAL( statusChanged(QAbstractSocket::SocketState, QString const&, quint16 const&, QAbstractSocket::SocketType const&) ), this, SLOT( updateTrayIconColor() ) );
}
void MainWindow::removedGpsdClient(GpsdClient* gpsdClient){
	GpsdTabSetupWidget* setup;
	for( int i=m_gSourcesTabs->count()-1; i>=0; --i ){
		setup = qobject_cast<GpsdTabSetupWidget*>( m_gSourcesTabs->widget( i ) );
		if( setup && setup->getGpsdClient()==gpsdClient ){
			m_gSourcesTabs->removeTab( i );
			setup->deleteLater();
		}
	}
	TargetTabSetupWidget* targets;
	for( int i=m_gTargetsTabs->count()-1; i>=0; --i ){
		targets = qobject_cast<TargetTabSetupWidget*>( m_gTargetsTabs->widget( i ) );
		if( targets && targets->getGpsdClient()==gpsdClient ){
			m_gTargetsTabs->removeTab( i );
			targets->deleteLater();
		}
	}
	GpsdStatusWidget* monitor;
	for( int i=m_gGpsdClientMonitorsLayout->count()-1; i>=0; --i ){
		QLayoutItem* item = m_gGpsdClientMonitorsLayout->itemAt( i );
		if( item ){
			monitor = qobject_cast<GpsdStatusWidget*>( item->widget() );
			if( monitor && monitor->getGpsdClient()==gpsdClient ){
				monitor->deleteLater();
			}
		}
	}
}

void MainWindow::restoredMinimizedInTray(){
	this->show();
	if( m_trayIcon ){
		m_trayIcon->hide();
	}
	this->setWindowState( ( windowState()&( ~Qt::WindowMinimized ) )|Qt::WindowActive );
	this->activateWindow();
}

void MainWindow::minimizeInTray(){
	this->setWindowState( windowState()|Qt::WindowMinimized );
	if( QSystemTrayIcon::isSystemTrayAvailable() ){
		if( !m_trayIcon ){
			m_trayIcon = new QSystemTrayIcon( this );
			connect( m_trayIcon, SIGNAL( activated(QSystemTrayIcon::ActivationReason) ), this, SLOT( trayIconActivated(QSystemTrayIcon::ActivationReason) ) );
			QMenu* contextMenu = new QMenu( this );
				// contextMenu->QObject::setParent( m_trayIcon ); // crash
				connect( contextMenu->addAction( tr( "&Restore" ) ), SIGNAL( triggered() ), this, SLOT( restoredMinimizedInTray() ) );
				connect( contextMenu->addAction( tr( "&Quit" ) ), SIGNAL( triggered() ), this, SLOT( closeWindow() ) );
			m_trayIcon->setContextMenu( contextMenu );
			m_trayIcon->setToolTip( Application::getAppTitle() );
			m_trayIcon->setIcon( this->windowIcon() );
			updateTrayIconColor();
		}
		m_trayIcon->show();
		this->hide();
	}
}

void MainWindow::saveSettings(){
	if( !APP_CLO_CONFIGNOSAVE ){
		QJsonObject settings;
			QJsonObject httpServer;
				httpServer.insert( "address", m_gHttpSetIp->text() );
				httpServer.insert( "port", m_gHttpSetPort->value() );
				httpServer.insert( "autostart", m_gHttpAutoStart->isChecked() );
			settings.insert( "HttpServer", httpServer );
			QJsonArray gpsdClients;
			GpsdTabSetupWidget* setup;
			TargetTabSetupWidget* targets;
			for( int i=0; i<m_gSourcesTabs->count(); ++i ){
				setup = qobject_cast<GpsdTabSetupWidget*>( m_gSourcesTabs->widget( i ) );
				if( !setup ) continue;
				targets = 0;
				for( int j=0; j<m_gTargetsTabs->count(); ++j ){
					TargetTabSetupWidget* targets1 = qobject_cast<TargetTabSetupWidget*>( m_gTargetsTabs->widget( i ) );
					if( targets1 && targets1->getGpsdClient()==setup->getGpsdClient() ){
						targets = targets1; // found matching targets tab
						break;
					}
				}
				if( !targets ) continue;
				QJsonObject gpsdClient;
					struct GpsdTabSetupWidget::Configuration c_setup;
					setup->getConfiguration( c_setup );
					gpsdClient.insert( "name", c_setup.name );
					gpsdClient.insert( "refreshRate", c_setup.refreshRate );
					gpsdClient.insert( "autoConnect", c_setup.autoConnect );
					gpsdClient.insert( "autoReconnect", c_setup.autoReconnect );
					QJsonArray hosts;
					for( QList<struct GpsdTabSetupWidget::ConfigurationHost>::const_iterator host=c_setup.hosts.begin(); host!=c_setup.hosts.end(); ++host){
						QJsonObject hostJson;
							hostJson.insert( "hostName", host->hostName );
							hostJson.insert( "port", host->port );
							hostJson.insert( "protocol", ( int )( host->protocol ) );
							hostJson.insert( "symmetric", host->symmetric );
						hosts.append( hostJson );
					}
					gpsdClient.insert( "hosts", hosts );
					struct TargetTabSetupWidget::Configuration c_targets;
					targets->getConfiguration( c_targets );
					gpsdClient.insert( "httpEnabled", c_targets.httpEnabled );
					gpsdClient.insert( "httpFilename", c_targets.httpFilename );
					gpsdClient.insert( "httpExpire", c_targets.httpExpire );
					gpsdClient.insert( "httpFunction", QLatin1String( c_targets.httpFunction ) );
					gpsdClient.insert( "javascriptEnabled", c_targets.javascriptEnabled );
					gpsdClient.insert( "javascriptFilename", c_targets.javascriptFilename );
					gpsdClient.insert( "javascriptFunction", QLatin1String( c_targets.javascriptFunction ) );
					gpsdClient.insert( "jsonEnabled", c_targets.jsonEnabled );
					gpsdClient.insert( "jsonFilename", c_targets.jsonFilename );
				gpsdClients.append( gpsdClient );
			}
			settings.insert( "GpsdClients", gpsdClients );
		Application::setAppSettings( settings );
	}
}

void MainWindow::closeEvent(QCloseEvent *event){
	if( m_loggingOut ){ // user is logging out
		// Already saved: just quit
		event->accept();
	}
	else if( !isVisible() ){ // minimized in tray
		// Quit without message
		saveSettings();
		event->accept();
	}
	else{
		// Show message
		QMessageBox msgQuitOrMinimize;
			msgQuitOrMinimize.setText( tr( "Do you want to quit or minimize in tray?" ) );
			QPushButton* btnQuit = msgQuitOrMinimize.addButton( tr( "&Quit" ), QMessageBox::YesRole );
			QPushButton* btnMinimize = msgQuitOrMinimize.addButton( tr( "&Minimize in tray" ), QMessageBox::NoRole );
			QPushButton* btnCancel = msgQuitOrMinimize.addButton( QMessageBox::Cancel );
			msgQuitOrMinimize.setDefaultButton( btnMinimize );
			msgQuitOrMinimize.setEscapeButton( btnCancel );
			msgQuitOrMinimize.setWindowModality( Qt::WindowModal );
		msgQuitOrMinimize.exec();
		if( msgQuitOrMinimize.clickedButton()==btnQuit ){
			// Quit
			saveSettings();
			event->accept();
		}
		else if( msgQuitOrMinimize.clickedButton()==btnMinimize ){
			// Minimize in tray
			minimizeInTray();
			event->ignore();
		}
		else{
			// Cancel
			event->ignore();
		}
	}
}

void MainWindow::selectCategory(){
	QObject* button = sender();
	int stackIndex = -1;
	
	if( button==m_btnStatus ) stackIndex = 0;
	else if( button==m_btnSources ) stackIndex = 1;
	else if( button==m_btnTargets ) stackIndex = 2;
	else if( button==m_btnAbout ) stackIndex = 3;
	
	if( stackIndex!=-1 ){
		m_gStack->setCurrentIndex( stackIndex );
		m_btnStatus->setChecked( button==m_btnStatus );
		m_btnSources->setChecked( button==m_btnSources );
		m_btnTargets->setChecked( button==m_btnTargets );
		m_btnAbout->setChecked( button==m_btnAbout );
	}
	
	if( m_httpSetupChanged ){
		HttpServer* httpServer = HttpServer::getHttpServer();
		if( httpServer ){
			m_httpSetupChanged = false;
			HttpServerSetup httpSetup;
				httpSetup.address = m_gHttpSetIp->text();
				httpSetup.port = m_gHttpSetPort->value();
			httpServer->changeSetup( httpSetup );
		}
	}
	
	GpsdTabSetupWidget* setup;
	for( int i=m_gSourcesTabs->count()-1; i>=0; --i ){
		setup = qobject_cast<GpsdTabSetupWidget*>( m_gSourcesTabs->widget( i ) );
		if( setup && setup->getHasChanged() ){
			setup->applyConfiguration();
		}
	}
	
	TargetTabSetupWidget* targets;
	for( int i=m_gTargetsTabs->count()-1; i>=0; --i ){
		targets = qobject_cast<TargetTabSetupWidget*>( m_gTargetsTabs->widget( i ) );
		if( targets && targets->getHasChanged() ){
			targets->applyConfiguration();
		}
	}
}

void MainWindow::updateHttpRequests(const quint64 &newRequests){
	m_gHttpRequests->setText( QString::number( newRequests ) );
}

void MainWindow::updateHttpStatus(HttpServerStatus const& newStatus){
	if( newStatus.active )
		m_gHttpActive->setText( QString( "<span style='color:green;'>" )+tr( "Running" )+QString( "</span>" ) );
	else
		m_gHttpActive->setText( QString( "<span style='color:red;'>" )+tr( "Stopped" )+QString( "</span>" ) );
	if( newStatus.address.length()==0 )
		m_gHttpAddress->setText( "any" );
	else
		m_gHttpAddress->setText( newStatus.address );
	m_gHttpPort->setText( QString::number( newStatus.port ) );
	updateHttpRequests( newStatus.requests );
}

void MainWindow::flagHttpSetupChanged(){
	m_httpSetupChanged = true;
}

GpsdClient* MainWindow::makeNewGpsdClient(){
	GpsdClient* gpsdClient = new GpsdClient( App );
	m_gSourcesTabs->setCurrentIndex( m_gSourcesTabs->count()-1 );
	return gpsdClient;
}

void MainWindow::closeWindow(){
	saveSettings();
	App->quit();
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason){
	switch( reason ){
		case QSystemTrayIcon::DoubleClick:
			restoredMinimizedInTray();
		break;
		default:
			// nothing
		break;
	}
}

QString MainWindow::s_messageConnected;
QString MainWindow::s_messageConnectionLost;

void MainWindow::gpsdConnectionStateChanged(bool connected){
	if( !APP_CLO_TRAYNOMESSAGES && m_trayIcon && m_trayIcon->isVisible() ){
		GpsdClient const* gpsdClient = qobject_cast<GpsdClient const*>( sender() );
		if( gpsdClient ){
			QString title = tr( "GPSD client \"" )+gpsdClient->getName()+tr( "\"" );
			QString const* message;
			if( connected ) message = &s_messageConnected;
			else message = &s_messageConnectionLost;
			m_trayIcon->showMessage(
				title,
				*message,
				QSystemTrayIcon::Information,
				MAINWINDOW_TRAY_GPSD_CONNECTION_MESSAGE_DURATION
			);
		}
	}
}

QPixmap const* MainWindow::s_appIconDefault = 0;
QIcon const* MainWindow::s_appIconStopped = 0;
QIcon const* MainWindow::s_appIconConnecting = 0;
QIcon const* MainWindow::s_appIconConnected = 0;
QIcon const* MainWindow::s_appIconUnconnected = 0;

void MainWindow::makeAppIcon(QIcon const* &icon, QColor color){
	// The design ensures that there is no memory leak.
	if( !s_appIconDefault ){
		s_appIconDefault = new QPixmap( ":/gpsd_javascript_relay_32.png" );
	}
	if( icon ){
		delete icon;
		icon = 0;
	}
	QImage image = s_appIconDefault->toImage();
	for( int x=0; x<=31; ++x ){
		for( int y=14; y<=17; ++y ){
			image.setPixelColor( x, y, color );
		}
	}
	icon = new QIcon( QPixmap::fromImage( image ) );
}

void MainWindow::updateTrayIconColor(){
	// This slot is called when the tray icon is created and whenever a GPSD client update its status.
	// Only the first GPSD client affects the color of the tray icon.
	// The MainWindow icon is changed, then the tray icon is updated if it exists.
	// This will only work properly with the default 32x32 icon.
	
	GpsdClient const* gpsdClientSender = qobject_cast<GpsdClient const*>( sender() );
	GpsdClient const* gpsdClient = 0;
	
	QLayoutItem* item = m_gGpsdClientMonitorsLayout->itemAt( 0 );
	if( item ){
		GpsdStatusWidget const* monitor = qobject_cast<GpsdStatusWidget const*>( item->widget() );
		if( monitor ){
			gpsdClient = monitor->getGpsdClient();
		}
	}
	
	if( gpsdClient && ( !gpsdClientSender || gpsdClient==gpsdClientSender ) ){ // only 1st GPSD client
		QIcon const* icon;
		if( gpsdClient->isManuallyStopped() ){
			if( !s_appIconStopped ) makeAppIcon( s_appIconStopped, "#FF0000" );
			icon = s_appIconStopped;
		}
		else{
			QAbstractSocket::SocketState status = gpsdClient->getConnectionStatus();
			switch( status ){
				case QAbstractSocket::HostLookupState:
				case QAbstractSocket::ConnectingState:
					if( !s_appIconConnecting ) makeAppIcon( s_appIconConnecting, "#FFC000" );
					icon = s_appIconConnecting;
				break;
				case QAbstractSocket::ConnectedState:
					if( !s_appIconConnected ) makeAppIcon( s_appIconConnected, "#00FF00" );
					icon = s_appIconConnected;
				break;
				default:
					if( !s_appIconUnconnected ) makeAppIcon( s_appIconUnconnected, "#FF00FF" );
					icon = s_appIconUnconnected;
			}
		}
		if( icon ){
			App->setWindowIcon( *icon );
			if( m_trayIcon ) m_trayIcon->setIcon( *icon );
		}
	}
}

void MainWindow::prepareLogout(QSessionManager& manager){ // logout handling
	m_loggingOut = true;
	manager.release();
	saveSettings();
}
