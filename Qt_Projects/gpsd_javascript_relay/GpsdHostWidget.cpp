#include "GpsdHostWidget.h"

GpsdHostWidget::GpsdHostWidget(QWidget *parent):
	QWidget( parent )
{
	setupUi( this );
	connect( m_btnRemove, SIGNAL( clicked() ), this, SLOT( deleteLater() ) );
}
