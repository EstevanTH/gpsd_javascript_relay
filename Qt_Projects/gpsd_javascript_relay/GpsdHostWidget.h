/** Class GpsdHostWidget
This widget class represents a host in the list of a GpsdTabSetupWidget.
**/

#ifndef GPSDHOSTWIDGET_H
#define GPSDHOSTWIDGET_H

#include <QWidget>
#include "ui_GpsdHostWidget.h"

class GpsdTabSetupWidget;

namespace Ui {
	class GpsdHostWidget;
}

class GpsdHostWidget:
	public QWidget,
	private Ui::GpsdHostWidget
{
	Q_OBJECT
		friend class GpsdTabSetupWidget;
		
	public:
		explicit GpsdHostWidget(QWidget *parent = 0);
};

#endif // GPSDHOSTWIDGET_H
