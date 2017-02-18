/** Class TargetTabSetupWidget
This widget class sets up the targets of a GPSD client.
**/

#ifndef TARGETTABSETUPWIDGET_H
#define TARGETTABSETUPWIDGET_H

#include <QWidget>
#include <QFileDialog>
#include <QJsonObject>
#include "ui_TargetTabSetupWidget.h"
#include "GpsdClient.h"
#include "ValidatorHttpFilename.h"
#include "ValidatorJavascriptFunction.h"

namespace Ui {
	class TargetTabSetupWidget;
}

class TargetTabSetupWidget:
	public QWidget,
	private Ui::TargetTabSetupWidget
{
	Q_OBJECT
		
	public:
		struct Configuration{
			bool httpEnabled;
			QString httpFilename;
			double httpExpire;
			QByteArray httpFunction;
			bool javascriptEnabled;
			QString javascriptFilename;
			QByteArray javascriptFunction;
			bool jsonEnabled;
			QString jsonFilename;
		};
		
		explicit TargetTabSetupWidget(GpsdClient* gpsdClient, QTabWidget* targetsTabs, QWidget *parent=0);
		GpsdClient* getGpsdClient() const;
		void resetHasChanged();
		bool getHasChanged() const;
		void applyConfiguration();
		void getConfiguration(struct Configuration& c) const;
		void setConfiguration(QJsonObject const& gpsdCSettings);
		
	private slots:
		void on_m_btnJavascriptBrowse_clicked();
		void on_m_btnJsonBrowse_clicked();
		void flagHasChanged();
		void nameChanged(QString const& name);
		
	private:
		GpsdClient* m_gpsdClient;
		QTabWidget* m_gTargetsTabs; // not contained by this class
		bool m_hasChanged;
		ValidatorHttpFilename s_validatorHttpFilename;
		ValidatorJavascriptFunction s_validatorJavascriptFunction;
};

#endif // TARGETTABSETUPWIDGET_H
