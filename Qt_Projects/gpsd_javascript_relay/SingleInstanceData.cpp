#include "SingleInstanceData.h"
#include "Application.h"

bool SingleInstanceData::hasNewInstanceSignaled() const{
	return m_newInstanceSignaled;
}

void SingleInstanceData::refresh(){
	m_lastRefresh = QDateTime::currentMSecsSinceEpoch();
	m_newInstanceSignaled = false;
}

void SingleInstanceData::releaseMutex() const{
	s_singleInstanceHandler->unlock();
}

void SingleInstanceData::prepare(){
	if( s_singleInstanceHandler==( QSharedMemory* )-1 ){
		s_singleInstanceHandler = new QSharedMemory( Application::getAppTitle() );
		if( s_singleInstanceHandler->create( sizeof( SingleInstanceData ) ) ){ // data are random
			if( s_singleInstanceHandler->lock() ){
				// Set new fresh data:
				SingleInstanceData* data = ( SingleInstanceData* )s_singleInstanceHandler->data();
				data->m_lastRefresh = QDateTime::currentMSecsSinceEpoch();
				data->m_newInstanceSignaled = false;
				s_singleInstanceHandler->unlock();
			}
		}
		else if( s_singleInstanceHandler->error()==QSharedMemory::AlreadyExists ){ // data are valid
			if( s_singleInstanceHandler->attach() && s_singleInstanceHandler->lock() ){
				// Check for other running instance:
				SingleInstanceData* data = ( SingleInstanceData* )s_singleInstanceHandler->data();
				if( QDateTime::currentMSecsSinceEpoch()>data->m_lastRefresh+APPLICATION_INSTANCE_TIMEOUT ){
					// Set new fresh data:
					data->m_lastRefresh = QDateTime::currentMSecsSinceEpoch();
					data->m_newInstanceSignaled = false;
				}
				else{
					// Prevent execution & signal the other instance:
					data->m_newInstanceSignaled = true;
					Application::flagPreventingMultipleInstances();
				}
				s_singleInstanceHandler->unlock();
			}
		}
		if( !s_singleInstanceHandler->isAttached() ){ // failure, bypass
			delete s_singleInstanceHandler;
			s_singleInstanceHandler = 0;
		}
	}
}

SingleInstanceData* SingleInstanceData::getData(){
	prepare(); // if forgotten at startup
	if( s_singleInstanceHandler && s_singleInstanceHandler->lock() )
		return ( SingleInstanceData* )s_singleInstanceHandler->data();
	else
		return 0;
}

QSharedMemory* SingleInstanceData::s_singleInstanceHandler = ( QSharedMemory* )-1;
