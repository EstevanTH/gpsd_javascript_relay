#include "GpsdHostWidget.h"

GpsdHostWidget::GpsdHostWidget(QWidget *parent):
	QWidget( parent )
{
	if( s_translationDefault.isEmpty() ){
		s_translationDefault = tr( "Default" );
		s_placeholderHostName0 = tr( "IP address / Hostname" );
		s_placeholderHostName1 = tr( "Serial port name" );
	}
	
	setupUi( this );
	connect( m_btnRemove, SIGNAL( clicked() ), this, SLOT( buttonClicked() ) );
	connect( m_btnConfigure, SIGNAL( clicked() ), this, SLOT( buttonClicked() ) );
	connect( m_btnSerialOk, SIGNAL( clicked() ), this, SLOT( buttonClicked() ) );
	connect( m_gConnectionMethod, SIGNAL( currentIndexChanged(int) ), this, SLOT( changedConnectionMethod(int) ) );
	connect( m_gSerialPortsList, SIGNAL( currentIndexChanged(int) ), this, SLOT( changedSerialPort(int) ) );
	
	// Prepare RS-232 configuration UI:
	// Baud rates:
	if( s_standardBaudRates.isEmpty() ){
		s_standardBaudRates = QSerialPortInfo::standardBaudRates();
	}
	m_gSerialBaudRate->clear();
	m_gSerialBaudRate->addItem( s_translationDefault, ( qint32 )-1 );
	for( QList<qint32>::const_iterator rate = s_standardBaudRates.begin(); rate!=s_standardBaudRates.end(); ++rate ){
		m_gSerialBaudRate->addItem( QString::number( *rate )+" b/s", *rate );
	}
	// Data bits:
	m_gSerialDataBits->clear();
	m_gSerialDataBits->addItem( s_translationDefault, ( int )-1 );
	m_gSerialDataBits->addItem( "8 bits", ( int )QSerialPort::Data8 );
	m_gSerialDataBits->addItem( "7 bits", ( int )QSerialPort::Data7 );
	m_gSerialDataBits->addItem( "6 bits", ( int )QSerialPort::Data6 );
	m_gSerialDataBits->addItem( "5 bits", ( int )QSerialPort::Data5 );
	// Flow control:
	m_gSerialFlowControl->clear();
	m_gSerialFlowControl->addItem( s_translationDefault, ( int )-1 );
	m_gSerialFlowControl->addItem( tr( "None" ), ( int )QSerialPort::NoFlowControl );
	m_gSerialFlowControl->addItem( tr( "Hardware (RTS/CTS)" ), ( int )QSerialPort::HardwareControl );
	m_gSerialFlowControl->addItem( tr( "Software (XON/XOFF)" ), ( int )QSerialPort::SoftwareControl );
	// Parity:
	m_gSerialParity->clear();
	m_gSerialParity->addItem( s_translationDefault, ( int )-1 );
	m_gSerialParity->addItem( tr( "None", "f" ), ( int )QSerialPort::NoParity );
	m_gSerialParity->addItem( tr( "Even" ), ( int )QSerialPort::EvenParity );
	m_gSerialParity->addItem( tr( "Odd" ), ( int )QSerialPort::OddParity );
	m_gSerialParity->addItem( tr( "Space" ), ( int )QSerialPort::SpaceParity );
	m_gSerialParity->addItem( tr( "Mark" ), ( int )QSerialPort::MarkParity );
	// Stop bits:
	m_gSerialStopBits->clear();
	m_gSerialStopBits->addItem( s_translationDefault, ( int )-1 );
	m_gSerialStopBits->addItem( "1 bit", ( int )QSerialPort::OneStop );
	m_gSerialStopBits->addItem( "2 bits", ( int )QSerialPort::TwoStop );
	
	changedConnectionMethod( 0 );
}

void GpsdHostWidget::buttonClicked(){
	if( sender()==m_btnRemove ){
		deleteLater();
	}
	else if( sender()==m_btnConfigure ){
		if( m_gConnectionMethod->currentIndex()==1 ){
			if( !m_gAreaRS232->isVisible() ){
				m_gAreaRS232->setVisible( true );
				// Ports:
				QList<QSerialPortInfo> availablePorts = QSerialPortInfo::availablePorts();
				m_gSerialPortsList->clear();
				m_gSerialPortsList->addItem( tr( "Current port: " )+m_gHostName->text(), m_gHostName->text() );
				for( QList<QSerialPortInfo>::const_iterator port = availablePorts.begin(); port!=availablePorts.end(); ++port ){
					QString description = port->description();
						if( description.isEmpty() ) description = tr( "Serial port" );
					m_gSerialPortsList->addItem( description+' '+port->portName(), port->portName() );
				}
			}
			else{
				m_gAreaRS232->setVisible( false );
			}
		}
	}
	else if( sender()==m_btnSerialOk ){
		m_gAreaRS232->setVisible( false );
	}
}

void GpsdHostWidget::changedConnectionMethod(int method){
	switch( method ){
		case 1: // serial port
			m_btnConfigure->setVisible( true );
			m_gPort->setVisible( false );
			m_gProtocol->setVisible( false );
			m_gAreaRS232->setVisible( false );
			m_gHostName->setToolTip( s_placeholderHostName1 );
			m_gHostName->setPlaceholderText( s_placeholderHostName1 );
		break;
		default: // IP link
			m_btnConfigure->setVisible( false );
			m_gPort->setVisible( true );
			m_gProtocol->setVisible( true );
			m_gAreaRS232->setVisible( false );
			m_gHostName->setToolTip( s_placeholderHostName0 );
			m_gHostName->setPlaceholderText( s_placeholderHostName0 );
		break;
	}
}

void GpsdHostWidget::changedSerialPort(int index){
	( void )index;
	if( !m_gSerialPortsList->currentData().toString().isNull() ){ // prevent running just after clicking Configure
		m_gHostName->setText( m_gSerialPortsList->currentData().toString() );
	}
}

QString GpsdHostWidget::s_translationDefault;
QString GpsdHostWidget::s_placeholderHostName0;
QString GpsdHostWidget::s_placeholderHostName1;
