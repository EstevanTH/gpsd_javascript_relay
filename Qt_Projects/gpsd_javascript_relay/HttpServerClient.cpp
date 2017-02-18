#include "HttpServerClient.h"

// qint64 QIODevice::bytesAvailable() const
// QByteArray QIODevice::readAll()
// QByteArray QIODevice::read(qint64 maxSize)
// QByteArray QIODevice::readLine(qint64 maxSize = 0)
// void QAbstractSocket::disconnectFromHost()
// qint64 QIODevice::write(const QByteArray &byteArray)
// qint64 QIODevice::readLine(char *data, qint64 maxSize)
// qint64 QAbstractSocket::readLineData(char *data, qint64 maxlen)
// qint64 QAbstractSocket::writeData(const char *data, qint64 size)
// bool QAbstractSocket::isValid() const

// QByteArray QString::toUtf8() const
// QByteArray QString::toLatin1() const

bool HttpServerClient::s_prepared = false;
QByteArray const HttpServerClient::s_contentTypeApplicationJson = "application/json";
QByteArray const HttpServerClient::s_contentTypeTextJavascript = "text/javascript";
QByteArray const HttpServerClient::s_contentTypeTextPlain = "text/plain";
QByteArray HttpServerClient::s_httpError400;
QByteArray HttpServerClient::s_httpError404;
QByteArray HttpServerClient::s_httpError405;
QByteArray HttpServerClient::s_httpError503;
QByteArray HttpServerClient::s_httpError504;

HttpServerClient::HttpServerClient(QTcpSocket* socket):
	QObject( socket ),
	m_socket( socket )
{
	if( !s_prepared ){
		// Initialize static stuff:
		s_prepared = true;
		buildHttpResponse(
			s_httpError400,
			"Bad Request - What are you doing?",
			s_contentTypeTextPlain,
			"400 Bad Request"
		);
		buildHttpResponse(
			s_httpError404,
			"Not Found - Wrong address?",
			s_contentTypeTextPlain,
			"404 Not Found"
		);
		buildHttpResponse(
			s_httpError405,
			"Method Not Allowed - Not using a GET request?",
			s_contentTypeTextPlain,
			"405 Method Not Allowed"
		);
		buildHttpResponse(
			s_httpError503,
			"Service Unavailable - GPSD client stopped?",
			s_contentTypeTextPlain,
			"503 Service Unavailable"
		);
		buildHttpResponse(
			s_httpError504,
			"Gateway Time-out - Not connected to GPSD server?",
			s_contentTypeTextPlain,
			"504 Gateway Time-out"
		);
	}
	
	connect( m_socket, SIGNAL( readyRead() ), this, SLOT( incomingData() ) );
	connect( m_socket, SIGNAL( disconnected() ), m_socket, SLOT( deleteLater() ) );
}

QByteArray const HttpServerClient::s_expectedTail = "\x0D\x0A\x0D\x0A";
QByteArray const HttpServerClient::s_expectedRequestType = "GET ";
QByteArray const HttpServerClient::s_httpStatus200 = "200 OK";

void HttpServerClient::incomingData(){
	bufferIn.append( m_socket->readAll() );
	QByteArray tail = bufferIn.right( 4 );
	if( tail==s_expectedTail ){ // complete request
		if( bufferIn.left( 4 )==s_expectedRequestType ){ // is a GET request
			// Extract URI from request:
			QString uri;
			{
				bool finished = false;
				for( QByteArray::const_iterator it=bufferIn.begin()+4; ( !finished && it!=bufferIn.end() ); ++it ){
					switch( *it ){
						case '\x0D':
						case '\x0A':
						case ' ':
						case '?':
							finished = true;
						break;
						default:
							uri += *it;
					}
				}
			}
			// Extract filename and extension:
			QString ext;
			QString filename;
			{
				int start = uri.length()-1;
				for( ; start>0; --start ){
					if( uri[start]=='.' ) break;
				}
				if( start ){ // found extension
					++start; // position where extension is
					ext += uri.midRef( start ); // extension alone
					filename += uri.midRef( 1, uri.length()-ext.length()-2 ); // strip extension, dot and slash from URI
				}
				else{ // no extension
					filename += uri.midRef( 1 ); // strip slash from URI
				}
			}
			// Find valid GpsdClient if valid extension
			HttpServerClient::ContentType contentType = Unknown;
			if( ext=="js" ){
				contentType = Javascript;
			}
			else if( ext=="json" ){
				contentType = JSON;
			}
			if( contentType==Javascript || contentType==JSON ){
				GpsdClient* gpsdClient = GpsdClient::findByHttpFilename( filename );
				if( gpsdClient ){
					if( gpsdClient->isManuallyStopped() ){
						sendHttpAnswer( s_httpError503 );
					}
					else{
						if( gpsdClient->isDataTpvExpired() ){
							sendHttpAnswer( s_httpError504 );
						}
						else{
							QByteArray packetContent;
							if( contentType==Javascript ){
								buildHttpResponse(
									packetContent,
									gpsdClient->getDataTpvJavascriptHttp(),
									s_contentTypeTextJavascript,
									s_httpStatus200
								);
							}
							else{
								buildHttpResponse(
									packetContent,
									gpsdClient->getDataTpvJson(),
									s_contentTypeApplicationJson,
									s_httpStatus200
								);
							}
							sendHttpAnswer( packetContent );
						}
					}
				}
				else{
					sendHttpAnswer( s_httpError404 );
				}
			}
			else{
				sendHttpAnswer( s_httpError404 );
			}
		}
		else{
			sendHttpAnswer( s_httpError405 );
		}
	}
	else if( bufferIn.size()>=HTTP_MAX_BUFFER ){ // flooded socket
		sendHttpAnswer( s_httpError400 );
	}
}

void HttpServerClient::sendHttpAnswer(QByteArray const& packet){
	bufferIn.clear();
	m_socket->write( packet );
	m_socket->close(); // should close connection with a TCP FIN packet
}

QByteArray const HttpServerClient::s_newLine = "\x0D\x0A";
QByteArray const HttpServerClient::s_headerStatus = "HTTP/1.0 ";
QByteArray const HttpServerClient::s_headerAccessControlAllowOrigin = "Access-Control-Allow-Origin: *\x0D\x0A";
QByteArray const HttpServerClient::s_headerCacheControl = "Cache-Control: no-cache, no-store, must-revalidate\x0D\x0A";
QByteArray const HttpServerClient::s_headerConnection = "Connection: close\x0D\x0A";
QByteArray const HttpServerClient::s_headerContentLength = "Content-Length: ";
QByteArray const HttpServerClient::s_headerContentType = "Content-Type: ";
QByteArray const HttpServerClient::s_headerPragma = "Pragma: no-cache\x0D\x0A";
QByteArray const HttpServerClient::s_headerServer = "Server: GPSD to JavaScript relay\x0D\x0A";

void HttpServerClient::buildHttpResponse(
	QByteArray& packetContent,
	QByteArray const& content,
	QByteArray const& mimeType,
	QByteArray const& httpStatus
){
	// packetContent: empty QByteArray that will be filled
	// content: the content of the answer
	// mimeType: the MIME type of the content
	// httpStatus: the HTTP status full string, such as "404 Not Found"
	
	packetContent.clear();
	packetContent.append( s_headerStatus );
		packetContent.append( httpStatus );
		packetContent.append( s_newLine );
	packetContent.append( s_headerAccessControlAllowOrigin );
	packetContent.append( s_headerCacheControl );
	packetContent.append( s_headerConnection );
	packetContent.append( s_headerContentLength );
		packetContent.append( QString::number( content.size() ).toLatin1() );
		packetContent.append( s_newLine );
	packetContent.append( s_headerContentType );
		packetContent.append( mimeType );
		packetContent.append( s_newLine );
	packetContent.append( s_headerPragma );
	packetContent.append( s_headerServer );
	packetContent.append( s_newLine );
	packetContent.append( content );
}
