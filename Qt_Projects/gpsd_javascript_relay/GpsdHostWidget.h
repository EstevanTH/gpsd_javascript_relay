/** Class GpsdHostWidget
This widget class represents a host in the list of a GpsdTabSetupWidget.
**/

#ifndef GPSDHOSTWIDGET_H
#define GPSDHOSTWIDGET_H

#include <QWidget>
#include <QSerialPortInfo>
#include <QSerialPort>
#include "ui_GpsdHostWidget.h"

class GpsdTabSetupWidget;

namespace Ui{
	class GpsdHostWidget;
}

class GpsdHostWidget:
	public QWidget,
	private Ui::GpsdHostWidget
{
	Q_OBJECT
		friend class GpsdTabSetupWidget;
		
	public:
		
	private:
		explicit GpsdHostWidget(QWidget *parent = 0);
		QList<qint32> s_standardBaudRates;
		static QString s_translationDefault;
		static QString s_placeholderHostName0;
		static QString s_placeholderHostName1;
		
	private slots:
		void buttonClicked();
		void changedConnectionMethod(int method);
		void changedSerialPort(int index);
};

#endif // GPSDHOSTWIDGET_H
