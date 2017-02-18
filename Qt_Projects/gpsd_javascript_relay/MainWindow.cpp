#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent):
	QWidget( parent ),
	m_httpSetupChanged( false ),
	m_trayIcon( 0 )/*,
	m_quitRequested( false )*/
{
	// Main interface:
	setupUi( this );
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
			m_trayIcon->setIcon( this->windowIcon() );
			m_trayIcon->setToolTip( Application::getAppTitle() );
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
		// m_quitRequested = true;
		saveSettings();
		event->accept();
	}
	else if( msgQuitOrMinimize.clickedButton()==btnMinimize ){
		// m_quitRequested = false;
		minimizeInTray();
		event->ignore();
	}
	else{
		event->ignore();
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
