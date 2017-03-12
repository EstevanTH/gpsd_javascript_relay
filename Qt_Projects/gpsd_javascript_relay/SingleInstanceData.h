#ifndef SINGLEINSTANCEDATA_H
#define SINGLEINSTANCEDATA_H

#include <QSharedMemory>
#include <QDateTime>

class SingleInstanceData{
	// This class has no constructor and no destructor because it cannot be instanciated.
	// Non-static attributes are actually stored in the shared memory.
		
	public:
		virtual ~SingleInstanceData() = 0;
		bool hasNewInstanceSignaled() const;
		void refresh();
		void releaseMutex() const; // must be called when operations finished
		static void prepare(); // should be called at startup & in getData()
		static SingleInstanceData* getData();
		
	private:
		qint64 m_lastRefresh;
		bool m_newInstanceSignaled;
		static QSharedMemory* s_singleInstanceHandler;
};

#endif // SINGLEINSTANCEDATA_H
