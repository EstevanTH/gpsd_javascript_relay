/** Class HttpServerClient
This class represents a client of the HTTP server.
It contains all the mechanism to interpret and answer the HTTP request.
HTTP error messages are prepared during startup so they are 100% ready.
**/

#ifndef HTTPSERVERCLIENT_H
#define HTTPSERVERCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include "HttpServerClient.h"
#include "GpsdClient.h"

#define HTTP_MAX_BUFFER 65536

class HttpServer;

class HttpServerClient: public QObject{
	Q_OBJECT
		
	public:
		explicit HttpServerClient(QTcpSocket* socket=0);
		static void buildHttpResponse(
			QByteArray& packetContent,
			QByteArray const& content,
			QByteArray const& mimeType,
			QByteArray const& httpStatus = "200 OK"
		);
		static QByteArray const s_newLine;
		static QByteArray const s_headerStatus;
		static QByteArray const s_headerAccessControlAllowOrigin;
		static QByteArray const s_headerCacheControl;
		static QByteArray const s_headerConnection;
		static QByteArray const s_headerContentLength;
		static QByteArray const s_headerContentType;
		static QByteArray const s_headerPragma;
		static QByteArray const s_headerServer;
		enum ContentType{Unknown, Javascript, JSON};
		
	signals:
		
	public slots:
		
	private slots:
		void incomingData();
		
	protected:
		void sendHttpAnswer(QByteArray const& packet);
		
	private:
		QTcpSocket* m_socket;
		QByteArray bufferIn;
		static bool s_prepared;
		static QByteArray const s_expectedTail;
		static QByteArray const s_expectedRequestType;
		static QByteArray const s_httpStatus200;
		static QByteArray const s_contentTypeApplicationJson;
		static QByteArray const s_contentTypeTextJavascript;
		static QByteArray const s_contentTypeTextPlain;
		static QByteArray s_httpError400; // Bad Request
		static QByteArray s_httpError404; // Not Found
		static QByteArray s_httpError405; // Method Not Allowed
		static QByteArray s_httpError503; // Service Unavailable
		static QByteArray s_httpError504; // Gateway Time-out
};

#endif // HTTPSERVERCLIENT_H
