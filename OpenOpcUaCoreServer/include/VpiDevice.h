/*****************************************************************************
	  Author
		©. Philippe Buchet, JRC (2011)
	  
	  Contributors
	©. Michel Condemine, 4CE Industry (2010-2012)

	This software is a computer program whose purpose is to 
			implement behavior describe in the OPC UA specification.
		see wwww.opcfoundation.org for more details about OPC.
	This software is governed by the CeCILL-C license under French law and
	abiding by the rules of distribution of free software.  You can  use, 
	modify and/ or redistribute the software under the terms of the CeCILL-C
	license as circulated by CEA, CNRS and INRIA at the following URL
	"http://www.cecill.info". 

	As a counterpart to the access to the source code and  rights to copy,
	modify and redistribute granted by the license, users are provided only
	with a limited warranty  and the software's author,  the holder of the
	economic rights,  and the successive licensors  have only  limited
	liability. 

	In this respect, the user's attention is drawn to the risks associated
	with loading,  using,  modifying and/or developing or reproducing the
	software by the user in light of its specific status of free software,
	that may mean  that it is complicated to manipulate,  and  that  also
	therefore means  that it is reserved for developers  and  experienced
	professionals having in-depth computer knowledge. Users are therefore
	encouraged to load and test the software's suitability as regards their
	requirements in conditions enabling the security of their systems and/or 
	data to be ensured and,  more generally, to use and operate it in the 
	same conditions as regards security. 

	The fact that you are presently reading this means that you have had
		knowledge of the CeCILL-C license and that you accept its terms.

*****************************************************************************/
 
#pragma once

namespace OpenOpcUa
{
	namespace UASubSystem 
	{
		enum ACCESSDATA_MODE 
		{ 
			POLL=0, 
			SUBSCRIBE=1, 
			BOTH=2
		};
		class CVPIScheduler;
		class CVpiDevice 
		{
		public:
			CVpiDevice(const char **atts,CVpiFuncCaller* pVpiFuncCaller=OpcUa_Null);
			~CVpiDevice();
			
			void			SetName(OpcUa_String szName);
			OpcUa_String	GetName();
			OpcUa_NodeId	GetSubSystemId();
			void			SetSubSystemId(OpcUa_NodeId aNodeId);
			CVpiFuncCaller* GetVpiFuncCaller();
			void			SetVpiFuncCaller(CVpiFuncCaller* pVpiFuncCaller);
			
			void			AddTag(CVpiTag* pTag);
			OpcUa_StatusCode RemoveTag(CVpiTag* pTag);
			CVpiTagList*	GetTags();
			unsigned int	GetAccessDataMode();
			void			SetAccessDataMode(unsigned int	 uiVal);
			CVPIScheduler*	GetVPIScheduler();
			void			AddWriteObject(CVpiWriteObject* pVpiWriteObject);
			void			SetSamplingInterval(OpcUa_Double dblVal);
			void			Wakeup();
			OpcUa_StatusCode	GetInternalStatus();
			void SetInternalStatus(OpcUa_StatusCode uStatus);
		protected:
			CVpiFuncCaller*		m_pVpiFuncCaller; // Lien avec la couche Vpi. Cette structure contient les points d'entrée de pointeur de fonction permettant l'accès au device
			OpcUa_String		m_szName;
			OpcUa_NodeId		m_SubSystemId;
			OpcUa_Mutex			m_TagsMutex;
			CVpiTagList*		m_pTags; 
			unsigned int		m_uiAccessDataMode; //Cette attribut permet d'indiquer le mode de fonctionnement du VPI : Polling, Subscription ou les deux
			CVPIScheduler*		m_pVPIScheduler; // Objet assurant la prise en charge des sous-systèmes de communication
			OpcUa_StatusCode	m_InternalStatus; // This is the status receive from the Vpi. 
												  // If the value of m_InternalStatus is 0x83050000 - OpcUa_Vpi_WarmStartNeedthe server will call WarmStart
		};
		typedef vector<CVpiDevice*> CVpiDeviceList;
	}

}