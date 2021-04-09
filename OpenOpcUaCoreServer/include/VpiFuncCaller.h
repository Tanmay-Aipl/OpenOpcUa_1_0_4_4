/*****************************************************************************
	  Author
		©. Michel Condemine, 4CE Industry (2010-2012)
	  
	  Contributors


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

		typedef OpcUa_UInt32 OpcUa_Vpi_StatusCode;
		typedef void* OpcUa_Vpi_Handle;
		typedef OpcUa_StatusCode (*PFUNCGLOBALSTART)(OpcUa_String szSubSystemName, OpcUa_NodeId SubsystemId, OpcUa_Vpi_Handle* hVpi);
		typedef OpcUa_StatusCode (*PFUNCGLOBALSTOP)(OpcUa_Vpi_Handle hVpi);
		typedef OpcUa_StatusCode (*PFUNCCOLDSTART)(OpcUa_Vpi_Handle hVpi);
		typedef OpcUa_StatusCode (*PFUNCWARMSTART)(OpcUa_Vpi_Handle hVpi);
		typedef OpcUa_StatusCode(*PFUNCREADVALUE)(OpcUa_Vpi_Handle hVpi, OpcUa_UInt32 UiNbOfValueRead, OpcUa_NodeId* Ids, OpcUa_DataValue** pValue);
		typedef OpcUa_StatusCode(*PFUNCWRITEVALUE)(OpcUa_Vpi_Handle hVpi, OpcUa_UInt32 UiNbOfValueWrite, OpcUa_NodeId* Ids, OpcUa_DataValue** ppValue);
		typedef OpcUa_StatusCode (*PFUNCPARSEADDID)(OpcUa_Vpi_Handle hVpi,OpcUa_NodeId Id, OpcUa_Byte Datatype, OpcUa_UInt32 iNbElt,OpcUa_Byte AccessRight, OpcUa_String ParsedAddress );
		typedef OpcUa_StatusCode (*PFUNCPARSEREMOVEID)(OpcUa_Vpi_Handle hVpi,OpcUa_NodeId Id);

		typedef OpcUa_Vpi_StatusCode(__stdcall *PFUNCNOTIFYCALLBACK)(OpcUa_UInt32 uiNoOfNotifiedObject, OpcUa_NodeId* Id, OpcUa_DataValue* pDataValue);
		typedef OpcUa_StatusCode (*PFUNCSETNOTFIFYCALLBACK)(OpcUa_Vpi_Handle hVpi,PFUNCNOTIFYCALLBACK func);

		// la structure ci-dessous contient les pointeurs de fonctions qui sont appelé au sein de VPI
		// chaque VPI est une DLL qui expose les 8 fonctions du contrat défini entre le serveur UA et les VPI
		// L'ensemble de VPIs pris en charge par ce servveur UA sont placé dans un vecteur appelé VpisFuncCaller
		// ce vecteur sera instancié dans la classe CVpiScheduler
		class CVpiFuncCaller
		{
		public:
			CVpiFuncCaller();
			~CVpiFuncCaller();
			OpcUa_String* GetLibraryName();
			void SetLibraryName(OpcUa_String LibraryName);
			// Verifie que le VPI a été correctement initialisé. Ce bool indique si les pointeurs de fonction sont initialisés
			OpcUa_Boolean IsVpiInitialized();
			void VpiInitialized(OpcUa_Boolean bVal);
			OpcUa_Boolean IsColdStarted();
			void ColdStarted(OpcUa_Boolean bVal);
			OpcUa_Boolean IsWarmStarted();
			void WarmStarted(OpcUa_Boolean bVal);
			OpcUa_StatusCode LoadVpi(unsigned int eAccessDataMode);

		public:
			PFUNCGLOBALSTART			GlobalStart;
			PFUNCGLOBALSTOP				GlobalStop;
			PFUNCCOLDSTART				ColdStart;
			PFUNCWARMSTART				WarmStart ; 
			PFUNCREADVALUE				ReadValue; 
			PFUNCWRITEVALUE				WriteValue;
			PFUNCPARSEADDID				ParseAddId;
			PFUNCPARSEREMOVEID			ParseRemoveId;
			PFUNCSETNOTFIFYCALLBACK		SetNotifyCallback;

			OpcUa_Vpi_Handle	m_hVpi; 
		private:
			OpcUa_String*		m_pLibraryName;
#ifdef WIN32
			HINSTANCE			m_phInst;
#endif

#ifdef _GNUC_
			void*				m_phInst;
#endif
			OpcUa_Boolean		m_bVpiInitialized; // indique si la fonction LoadVpi() a été appélé. 
												   // Si oui l'ensemble des pointeurs de fonctions sont initialisés
			OpcUa_Boolean		m_bColdStartDone;  // indique si la fonction ColdStart() a été appélé
			OpcUa_Boolean		m_bWarmStartDone;  // indique si la fonction ColdStart() a été appélé
		};
	}
}