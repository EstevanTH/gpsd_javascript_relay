/** Class HttpServer
This class represents the HTTP server.
Its purpose is only to accept incoming requests.
Once a request is accepted, an HttpServerClient handles the request.
Only one instance should exist.
**/

#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QMessageBox>
#include <QLinkedList>
#include <QSystemTrayIcon>
#include "HttpServerClient.h"

typedef struct HttpServerStatus{
	bool active;
	QString address;
	quint16 port;
	quint64 requests;
} HttpServerStatus;

typedef struct HttpServerSetup{
	QString address;
	quint16 port;
} HttpServerSetup;

class HttpServer: public QObject{
	Q_OBJECT
		
	public:
		explicit HttpServer(QObject *parent = 0);
		explicit HttpServer(HttpServerSetup const& newSetup, QObject *parent = 0);
		HttpServerStatus getStatus() const;
		quint64 getRequests() const;
		void signalStatusUpdated() const;
		void signalRequestsUpdated() const;
		static HttpServer* getHttpServer();
		
	private:
		void initialize();
		QString m_address;
		quint16 m_port;
		quint64 m_requests;
		QTcpServer* m_tcpServer;
		static HttpServer* s_httpServer;
		
	signals:
		void statusUpdated(HttpServerStatus const& newStatus) const; // sends status from active running members
		void requestsUpdated(quint64 const& newRequests) const;
		
	public slots:
		void changeSetup(HttpServerSetup const& newSetup);
		void startServer();
		void stopServer();
		void welcomeClient();
};

#endif // HTTPSERVER_H
