/** Class GpsdClient
This class represents the connection to a GPSD server.
It handles auto-reconnection.
The socket is never re-used.
**/

#ifndef GPSDCLIENT_H
#define GPSDCLIENT_H

#include <QObject>
#include <QLinkedList>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QMessageBox>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <cstdio>
#include <QDateTime>
#include <QFile>
#include "Application.h"
#include "GpsdHost.h"

// #include <QtDebug> // debug

#define GPSD_EXPIRE_UDP_CONNECT 5000 // ms after sending the first ?VERSION; request
#define GPSD_RETRY_UDP_DELAY 1000 // ms between ?VERSION; requests to confirm link
#define GPSD_EXPIRE_UDP_DATA 30000 // ms, m_lastAnswer-m_lastRequest with m_expectedAnswers>1 (when confirmed link)
#define GPSD_SOCKET_FAILURE_WAIT 1000

#define GPSD_RECEIVE_BUFFER 16384 // poor performance if big and full
#define GPSD_RECEIVE_BUFFER_HALF ( GPSD_RECEIVE_BUFFER/2 )

#define GPSD_TPV_NULL_DEVICE ( 1<<0 )
#define GPSD_TPV_NULL_STATUS ( 1<<1 )
#define GPSD_TPV_NULL_MODE ( 1<<2 )
#define GPSD_TPV_NULL_TIME ( 1<<3 )
#define GPSD_TPV_NULL_LAT ( 1<<4 )
#define GPSD_TPV_NULL_LON ( 1<<5 )
#define GPSD_TPV_NULL_ALT ( 1<<6 )
#define GPSD_TPV_NULL_TRACK ( 1<<7 )
#define GPSD_TPV_NULL_SPEED ( 1<<8 )
#define GPSD_TPV_NULL_CLIMB ( 1<<9 )
#define GPSD_TPV_NULL_EPT ( 1<<10 )
#define GPSD_TPV_NULL_EPX ( 1<<11 )
#define GPSD_TPV_NULL_EPY ( 1<<12 )
#define GPSD_TPV_NULL_EPV ( 1<<13 )
#define GPSD_TPV_NULL_EPD ( 1<<14 )
#define GPSD_TPV_NULL_EPS ( 1<<15 )
#define GPSD_TPV_NULL_EPC ( 1<<16 )
#define GPSD_TPV_SET_NULL( dataTpv, field ) ( ( dataTpv ).NotNullFields &= ~field )
#define GPSD_TPV_UNSET_NULL( dataTpv, field ) ( ( dataTpv ).NotNullFields |= field )
#define GPSD_TPV_FIELD_IS_NULL( dataTpv, field ) ( ( ( dataTpv ).NotNullFields&field )==0 )
#define GPSD_TPV_FIELD_IS_VALID( dataTpv, field ) ( ( ( dataTpv ).NotNullFields&field )!=0 )

#define GPSD_JS_MAX_LENGTH 2048 // including final NULL character
#define GPSD_JS_PRECISION 7 // digits after decimal point

class GpsdClient: public QObject{
	Q_OBJECT
		
	public:
		enum TpvMode{
			no_mode = 0,
			no_fix = 1,
			_2D = 2,
			_3D = 3
		};
		typedef struct GpsdClientDataTpv{
			QString device;
			int status;
			GpsdClient::TpvMode mode;
			QString time;
			double lat;
			double lon;
			double alt;
			double track;
			double speed;
			double climb;
			double ept;
			double epx;
			double epy;
			double epv;
			double epd;
			double eps;
			double epc;
			// Fields not included in TPV object, must be initialized:
			quint32 NotNullFields;
			qint64 ClientTime; // last update, QDateTime::currentMSecsSinceEpoch()
		} DataTpv;
		explicit GpsdClient(QObject *parent = 0);
		~GpsdClient();
		// Configuration (read):
		QString const& getName() const;
		double getRefreshRate() const;
		bool getAutoReconnect() const;
		QList<GpsdHost*> const& getHosts() const;
		bool getHttpEnabled() const;
			QByteArray const& getHttpFilename() const;
			QByteArray const& getHttpFunction() const;
			double getHttpExpire() const;
		bool getJavascriptEnabled() const;
			QString const& getJavascriptFilename() const;
			QByteArray const& getJavascriptFunction() const;
		bool getJsonEnabled() const;
			QString const& getJsonFilename() const;
		// Configuration (write):
		void setName(QString const& name);
		void setRefreshRate(double refreshRate);
		void setAutoReconnect(bool autoReconnect);
		GpsdHost* addHost(QString const& hostname, quint16 const& port, QAbstractSocket::SocketType const& protocol, bool symmetric=false);
		void removeHost(GpsdHost* host);
		void clearHosts();
		void setHttpEnabled(bool httpEnabled);
			void setHttpFilename(QByteArray const& httpFilename);
			void setHttpFunction(QByteArray const& httpFunction);
			void setHttpExpire(double httpExpire);
		void setJavascriptEnabled(bool javascriptEnabled);
			void setJavascriptFilename(QString const& javascriptFilename);
			void setJavascriptFunction(QByteArray const& javascriptFunction);
		void setJsonEnabled(bool jsonEnabled);
			void setJsonFilename(QString const& jsonFilename);
		// Data:
		GpsdClient::DataTpv const& getDataTpv() const;
		QByteArray const& getDataTpvJson() const;
		QByteArray const& getDataTpvJavascriptHttp() const;
		bool isDataTpvExpired() const;
		// Internals:
		QAbstractSocket::SocketState getConnectionStatus() const;
		bool isManuallyStopped() const;
		// Static:
		static GpsdClient* findByHttpFilename(QString const& filename);
		static QLinkedList<GpsdClient*> const& getGpsdClients();
		
	signals:
		void nameChanged(QString const& name);
		void statusChanged(QAbstractSocket::SocketState status, QString const& hostname="", quint16 const& port=0, QAbstractSocket::SocketType const& protocol=QAbstractSocket::TcpSocket);
		void hostChanged(QString const& hostname, quint16 const& port, QAbstractSocket::SocketType const& protocol);
		void dataTpvChanged(GpsdClient::DataTpv const& data);
		void connectionStateChanged(bool connected);
		
	public slots:
		void stopClient();
		void startClient();
		void sendRequest(QByteArray const& content);
		void requestTpv();
		
	private slots:
		void validateUdpLink();
		void linkValidated();
		void socketRetry();
		void socketFailure();
		void socketError(QAbstractSocket::SocketError error) const;
		void incomingData();
		void signalSocketStateChanged();
		
	protected:
		// Following methods require a dedicated QByteArray to work.
		const char* serializeJavascriptField(QByteArray& out, QString fieldValue, quint32 field=0) const;
		const char* serializeJavascriptField(QByteArray& out, double fieldValue, quint32 field=0) const;
		const char* serializeJavascriptField(QByteArray& out, int fieldValue, quint32 field=0) const;
		const char* serializeJavascriptField(QByteArray& out, GpsdClient::TpvMode fieldValue, quint32 field=0) const;
		const char* serializeJavascriptField(QByteArray& out, quint32 fieldValue, quint32 field=0) const;
		const char* serializeJavascriptField(QByteArray& out, qint64 fieldValue, quint32 field=0) const;
		
	private:
		// Configuration:
		QString m_name;
		double m_refreshRate;
		bool m_autoReconnect;
		QList<GpsdHost*> m_hosts; // browsed from beginning on new connection attempt, only
		bool m_httpEnabled;
			QByteArray m_httpFilename;
			QByteArray m_httpFunction;
			double m_httpExpire; // seconds
		bool m_javascriptEnabled;
			QString m_javascriptFilename;
			QByteArray m_javascriptFunction;
		bool m_jsonEnabled;
			QString m_jsonFilename;
		// Data:
		GpsdClientDataTpv m_dataTpv;
		QByteArray m_dataTpvJson;
		QByteArray m_dataTpvJavascriptHttp;
		// Internals:
		void clearRequestTimer();
		void clearSocket();
		void connectToHost(GpsdHost const& host);
		bool m_manuallyStopped;
		int m_indexConnectingHost; // to browse m_hosts
		QAbstractSocket* m_socket; // never reused
		bool m_socketValidated; // true when UDP seen answer or TCP connected
		QTimer* m_requestTimer;
		QByteArray m_bufferIn;
		uint m_expectedAnswers;
		qint64 m_beginConnection;
		qint64 m_lastAnswer; // QDateTime::currentMSecsSinceEpoch()
		qint64 m_lastRequest; // unused - QDateTime::currentMSecsSinceEpoch()
		QFile* m_outputJavascript;
		QFile* m_outputJson;
		// Static:
		static QString s_titleGpsdClient;
		static QLinkedList<GpsdClient*> s_gpsdClients;
		static QByteArray const s_requestContentTpv;
		static QByteArray const s_requestContentVersion;
		static QString const s_jsonKeyClass;
		static QString const s_jsonKeyDevice;
		static QString const s_jsonKeyStatus;
		static QString const s_jsonKeyMode;
		static QString const s_jsonKeyTime;
		static QString const s_jsonKeyEpt;
		static QString const s_jsonKeyLat;
		static QString const s_jsonKeyLon;
		static QString const s_jsonKeyAlt;
		static QString const s_jsonKeyEpx;
		static QString const s_jsonKeyEpy;
		static QString const s_jsonKeyEpv;
		static QString const s_jsonKeyTrack;
		static QString const s_jsonKeySpeed;
		static QString const s_jsonKeyClimb;
		static QString const s_jsonKeyEpd;
		static QString const s_jsonKeyEps;
		static QString const s_jsonKeyEpc;
		static QString const s_jsonKeyRelayTime; // custom
		static QString const s_jsonValueClassTpv;
		static char const s_javascriptNull[];
		static QString const s_javascriptUnsafeCRLF;
		static QChar const s_javascriptUnsafeDoubleQuote;
		static QChar const s_javascriptUnsafeAntiSlash;
		static QChar const s_javascriptUnsafeCR;
		static QChar const s_javascriptUnsafeLF;
		static QString const s_javascriptEscapedCRLF;
		static QString const s_javascriptEscapedDoubleQuote;
		static QString const s_javascriptEscapedAntiSlash;
		static QString const s_javascriptEscapedCR;
		static QString const s_javascriptEscapedLF;
		static char const s_javascriptTpvObjectFormat[];
};

#endif // GPSDCLIENT_H
