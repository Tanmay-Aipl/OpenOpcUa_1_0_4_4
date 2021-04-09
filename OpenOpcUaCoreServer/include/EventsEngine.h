#pragma once
namespace OpenOpcUa
{
	namespace UAEvents
	{
		class CEventsEngine
		{
		public:
			CEventsEngine();
			~CEventsEngine();
			void AddEventDefinition(CEventDefinition* pEventDefinition);
			OpcUa_StatusCode GetEventDefinition(OpcUa_UInt32 iEvtIndex, CEventDefinition** ppEventDefinition);
			OpcUa_StatusCode GetEventDefinition(OpcUa_NodeId aNodeId, CEventDefinitionList* pEventDefinitionList);
			OpcUa_StatusCode RemoveEventDefinition(CEventDefinition* pEventDefinition);
			OpcUa_UInt32 GetEventDefinitionListSize();
			OpcUa_Mutex	GetEventDefinitionListMutex();
			OpcUa_StatusCode SearchForEventDefinitionOnMethod(CUAMethod* pUAMethod, CEventDefinition** ppEventDefinition);
		protected:
			void StartEventsThread(void);
			void StopEventsThread(void);
			static void EventsThread(LPVOID arg);
		public:
			OpcUa_Semaphore					m_EventsThreadSem;
		private:
			OpcUa_Boolean					m_bRunEventsThread;
			OpcUa_Thread					m_hEventsThread;
			OpcUa_Mutex						m_hEventsThreadMutex;
			OpcUa_Mutex						m_pEventDefinitionListMutex;
			CEventDefinitionList*			m_pEventDefinitionList;
			OpcUa_UInt32					m_EventDefinitionIndex;
		};

	}
}