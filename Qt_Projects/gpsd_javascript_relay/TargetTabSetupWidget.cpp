#include "TargetTabSetupWidget.h"

ValidatorHttpFilename s_validatorHttpFilename;
ValidatorJavascriptFunction s_validatorJavascriptFunction;

TargetTabSetupWidget::TargetTabSetupWidget(GpsdClient* gpsdClient, QTabWidget* sourcesTabs, QWidget *parent):
	QWidget( parent ),
	m_gpsdClient( gpsdClient ),
	m_gTargetsTabs( sourcesTabs ),
	m_hasChanged( false )
{
	setupUi( this );
	connect( m_gHttpGroup, SIGNAL( toggled(bool) ), this, SLOT( flagHasChanged() ) );
	connect( m_gJavascriptGroup, SIGNAL( toggled(bool) ), this, SLOT( flagHasChanged() ) );
	connect( m_gJsonGroup, SIGNAL( toggled(bool) ), this, SLOT( flagHasChanged() ) );
	connect( m_gHttpFilename, SIGNAL( textChanged(const QString&) ), this, SLOT( flagHasChanged() ) );
	connect( m_gHttpFunction, SIGNAL( textChanged(const QString&) ), this, SLOT( flagHasChanged() ) );
	connect( m_gJavascriptFilename, SIGNAL( textChanged(const QString&) ), this, SLOT( flagHasChanged() ) );
	connect( m_gJavascriptFunction, SIGNAL( textChanged(const QString&) ), this, SLOT( flagHasChanged() ) );
	connect( m_gJsonFilename, SIGNAL( textChanged(const QString&) ), this, SLOT( flagHasChanged() ) );
	connect( m_gHttpExpire, SIGNAL( valueChanged(double) ), this, SLOT( flagHasChanged() ) );
	connect( m_gpsdClient, SIGNAL( nameChanged(QString const&) ), this, SLOT( nameChanged(QString const&) ) );
	m_gHttpFilename->setValidator( &s_validatorHttpFilename );
	m_gHttpFunction->setValidator( &s_validatorJavascriptFunction );
	m_gJavascriptFunction->setValidator( &s_validatorJavascriptFunction );
}

GpsdClient* TargetTabSetupWidget::getGpsdClient() const{
	return m_gpsdClient;
}

void TargetTabSetupWidget::on_m_btnJavascriptBrowse_clicked(){
	m_gJavascriptFilename->setText( QFileDialog::getSaveFileName(
		this,
		tr( "JavaScript target file" ),
		QString(),
		"JavaScript (*.js)",
		Q_NULLPTR
	) );
}
void TargetTabSetupWidget::on_m_btnJsonBrowse_clicked(){
	m_gJsonFilename->setText( QFileDialog::getSaveFileName(
		this,
		tr( "JSON target file" ),
		QString(),
		"JavaScript Object Notation (*.json)",
		Q_NULLPTR
	) );
}

void TargetTabSetupWidget::nameChanged(QString const& name){
	int index = m_gTargetsTabs->indexOf( this );
	if( index!=-1 ){
		m_gTargetsTabs->setTabText( index, name );
	}
}

void TargetTabSetupWidget::resetHasChanged(){ m_hasChanged = false; }
bool TargetTabSetupWidget::getHasChanged() const{ return m_hasChanged; }
void TargetTabSetupWidget::flagHasChanged(){ m_hasChanged = true; }

void TargetTabSetupWidget::applyConfiguration(){
	resetHasChanged();
	if( m_gpsdClient ){
		m_gpsdClient->setHttpEnabled( m_gHttpGroup->isChecked() );
			m_gpsdClient->setHttpFilename( m_gHttpFilename->text().toLatin1() );
			m_gpsdClient->setHttpFunction( m_gHttpFunction->text().toLatin1() );
			m_gpsdClient->setHttpExpire( m_gHttpExpire->value() );
		m_gpsdClient->setJavascriptEnabled( m_gJavascriptGroup->isChecked() );
			m_gpsdClient->setJavascriptFilename( m_gJavascriptFilename->text() );
			m_gpsdClient->setJavascriptFunction( m_gJavascriptFunction->text().toLatin1() );
		m_gpsdClient->setJsonEnabled( m_gJsonGroup->isChecked() );
			m_gpsdClient->setJsonFilename( m_gJsonFilename->text() );
	}
}

void TargetTabSetupWidget::getConfiguration(struct TargetTabSetupWidget::Configuration& c) const{
	c.httpEnabled = m_gHttpGroup->isChecked();
	c.httpFilename = m_gHttpFilename->text();
	c.httpExpire = m_gHttpExpire->value();
	c.httpFunction = m_gHttpFunction->text().toLatin1();
	c.javascriptEnabled = m_gJavascriptGroup->isChecked();
	c.javascriptFilename = m_gJavascriptFilename->text();
	c.javascriptFunction = m_gJavascriptFunction->text().toLatin1();
	c.jsonEnabled = m_gJsonGroup->isChecked();
	c.jsonFilename = m_gJsonFilename->text();
}

void TargetTabSetupWidget::setConfiguration(QJsonObject const& gpsdCSettings){
	if( gpsdCSettings.contains( "httpEnabled" ) )
		m_gHttpGroup->setChecked( gpsdCSettings.value( "httpEnabled" ).toBool() );
	if( gpsdCSettings.contains( "httpFilename" ) )
		m_gHttpFilename->setText( gpsdCSettings.value( "httpFilename" ).toString() );
	if( gpsdCSettings.contains( "httpExpire" ) )
		m_gHttpExpire->setValue( gpsdCSettings.value( "httpExpire" ).toDouble() );
	if( gpsdCSettings.contains( "httpFunction" ) )
		m_gHttpFunction->setText( gpsdCSettings.value( "httpFunction" ).toString() );
	if( gpsdCSettings.contains( "javascriptEnabled" ) )
		m_gJavascriptGroup->setChecked( gpsdCSettings.value( "javascriptEnabled" ).toBool() );
	if( gpsdCSettings.contains( "javascriptFilename" ) )
		m_gJavascriptFilename->setText( gpsdCSettings.value( "javascriptFilename" ).toString() );
	if( gpsdCSettings.contains( "javascriptFunction" ) )
		m_gJavascriptFunction->setText( gpsdCSettings.value( "javascriptFunction" ).toString() );
	if( gpsdCSettings.contains( "jsonEnabled" ) )
		m_gJsonGroup->setChecked( gpsdCSettings.value( "jsonEnabled" ).toBool() );
	if( gpsdCSettings.contains( "jsonFilename" ) )
		m_gJsonFilename->setText( gpsdCSettings.value( "jsonFilename" ).toString() );
	applyConfiguration();
}
