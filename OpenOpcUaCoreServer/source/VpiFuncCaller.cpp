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
#include "stdafx.h"
#ifdef _GNUC_
#include <dlfcn.h>
#endif
#include "VpiFuncCaller.h"
using namespace UASubSystem;
using namespace OpenOpcUa;
CVpiFuncCaller::CVpiFuncCaller()
{
	GlobalStop = OpcUa_Null;
	GlobalStart=OpcUa_Null;
	ColdStart=OpcUa_Null;
	WarmStart=OpcUa_Null ; 
	ReadValue=OpcUa_Null; 
	WriteValue=OpcUa_Null;
	ParseAddId=OpcUa_Null;
	ParseRemoveId=OpcUa_Null;
	SetNotifyCallback=OpcUa_Null;
	m_hVpi=NULL;
	m_pLibraryName=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(m_pLibraryName);
	m_bVpiInitialized=OpcUa_False;
	m_bColdStartDone=OpcUa_False;
	m_bWarmStartDone=OpcUa_False;
}
CVpiFuncCaller::~CVpiFuncCaller() 
{
	if (m_phInst)
	{
		OpcUa_FreeLibrary((void*)m_phInst);
	}
	if (m_pLibraryName)
		OpcUa_String_Clear(m_pLibraryName);
}
OpcUa_StatusCode CVpiFuncCaller::LoadVpi(unsigned int eAccessDataMode)
{ 
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (m_pLibraryName)
	{
		uStatus=OpcUa_LoadLibrary(m_pLibraryName,(void**)&m_phInst);
		if (uStatus!=OpcUa_Good)
		{
			uStatus=OpcUa_BadFileNotFound;
#ifdef WIN32
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"LoadVpiLibrary failed %s error: 0x%05x\n",OpcUa_String_GetRawString(m_pLibraryName),OpcUa_GetLastError());
#endif
#ifdef _GNUC_
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"LoadVpiLibrary failed %s error: %s\n",OpcUa_String_GetRawString(m_pLibraryName),dlerror());
#endif
		}
		else
		{
			// Chargement des fonctions
			// VpiGlobalStop
	#ifdef _WIN32_WCE
			GlobalStop = (PFUNCGLOBALSTOP)OpcUa_GetProcAddress(m_phInst,L"VpiGlobalStop");
	#else
			GlobalStop = (PFUNCGLOBALSTOP)OpcUa_GetProcAddress(m_phInst,"VpiGlobalStop");
	#endif
			if (!GlobalStop)
			{
				uStatus=OpcUa_BadInternalError;
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Cannot Initialize VpiGlobalStop. Your Vpi have problem\n");
			}
			else
			{			
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "VpiGlobalStop successfully loaded\n");
				// VpiGlobalStart
	#ifdef _WIN32_WCE
				GlobalStart = (PFUNCGLOBALSTART)OpcUa_GetProcAddress(m_phInst,L"VpiGlobalStart");
	#else
				GlobalStart = (PFUNCGLOBALSTART)OpcUa_GetProcAddress(m_phInst,"VpiGlobalStart");
	#endif
				if (!GlobalStart)
				{
					uStatus=OpcUa_BadInternalError;
#ifdef WIN32
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Cannot Initialize VpiGlobalStart. Your Vpi have problem\n");
#endif
#ifdef _GNUC_
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Cannot Initialize VpiGlobalStart. Your Vpi have problem: %s\n",dlerror());
#endif
				}
				else
				{
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"VpiGlobalStart successfully loaded\n");
					// VpiColdStart
	#ifdef _WIN32_WCE
					ColdStart = (PFUNCCOLDSTART)OpcUa_GetProcAddress(m_phInst,L"VpiColdStart");
	#else
					ColdStart = (PFUNCCOLDSTART)OpcUa_GetProcAddress(m_phInst,"VpiColdStart");
	#endif
					
					if (!ColdStart)
					{
						uStatus=OpcUa_BadInternalError;
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Cannot Initialize VpiColdStart. Your Vpi have problem\n");
					}
					else
					{
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"VpiColdStart successfully loaded\n");
						// VpiWarmStart
	#ifdef _WIN32_WCE
						WarmStart = (PFUNCWARMSTART)OpcUa_GetProcAddress(m_phInst,L"VpiWarmStart");
	#else
						WarmStart = (PFUNCWARMSTART)OpcUa_GetProcAddress(m_phInst,"VpiWarmStart");
	#endif
						if (!WarmStart)
						{
							uStatus=OpcUa_BadInternalError;
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Cannot Initialize VpiWarmStart. Your Vpi have problem\n");
						}
						else
						{
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"VpiWarmStart successfully loaded\n");
							// VpiReadValue
	#ifdef _WIN32_WCE
							ReadValue = (PFUNCREADVALUE)OpcUa_GetProcAddress(m_phInst,L"VpiReadValue");
	#else
							ReadValue = (PFUNCREADVALUE)OpcUa_GetProcAddress(m_phInst,"VpiReadValue");
	#endif
							if (!ReadValue)
							{
								uStatus=OpcUa_BadInternalError;
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Cannot Initialize VpiReadValue. Your Vpi have problem\n");
							}
							else
							{
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"VpiReadValue successfully loaded\n");
								// VpiWriteValue
	#ifdef _WIN32_WCE
								WriteValue = (PFUNCWRITEVALUE)OpcUa_GetProcAddress(m_phInst,L"VpiWriteValue");
	#else
								WriteValue = (PFUNCWRITEVALUE)OpcUa_GetProcAddress(m_phInst,"VpiWriteValue");
	#endif
								if (!WriteValue)
								{
									uStatus=OpcUa_BadInternalError;
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Cannot Initialize VpiWriteValue. Your Vpi have problem\n");
								}
								else
								{
										OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"VpiWriteValue successfully loaded\n");
									// VpiParseAddId
	#ifdef _WIN32_WCE
									ParseAddId = (PFUNCPARSEADDID)OpcUa_GetProcAddress(m_phInst,L"VpiParseAddId");
	#else
									ParseAddId = (PFUNCPARSEADDID)OpcUa_GetProcAddress(m_phInst,"VpiParseAddId");
	#endif
									if (!ParseAddId)
									{
										uStatus=OpcUa_BadInternalError;
										OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Cannot Initialize VpiParseAddId. Your Vpi have problem\n");
									}
									else
									{
										OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"VpiParseAddId successfully loaded\n");
										// VpiParseRemoveId
	#ifdef _WIN32_WCE
										ParseRemoveId = (PFUNCPARSEREMOVEID)OpcUa_GetProcAddress(m_phInst,L"VpiParseRemoveId");
	#else
										ParseRemoveId = (PFUNCPARSEREMOVEID)OpcUa_GetProcAddress(m_phInst,"VpiParseRemoveId");
	#endif
										if (!ParseRemoveId)
										{
											uStatus=OpcUa_BadInternalError;
											OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Cannot Initialize VpiParseRemoveId. Your Vpi have problem\n");
										}
										else
										{
											OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"VpiParseRemoveId successfully loaded\n");
											// 2 = OpenOpcUa::UASubSystem::BOTH, 1 = UASubSystem::SUBSCRIBE, 0 = UASubSystem::POLL
											// Even in poll mode the Vpi will need to call the callback in case he wants to request a WarmStart from an asyncronous part of the Vpi
											if ((eAccessDataMode==0) ||(eAccessDataMode==1) || (eAccessDataMode==2))
											{
												// mis en place de la fonction callback
												// attention tous les Vpi ne dispose pas forcement de cette fonction
	#ifdef _WIN32_WCE
												SetNotifyCallback= (PFUNCSETNOTFIFYCALLBACK)OpcUa_GetProcAddress(m_phInst,L"VpiSetNotifyCallback");
	#else
												SetNotifyCallback= (PFUNCSETNOTFIFYCALLBACK)OpcUa_GetProcAddress(m_phInst,"VpiSetNotifyCallback");
	#endif
												if (!SetNotifyCallback)
												{
													uStatus=OpcUa_BadInternalError;
													OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Cannot Initialize NotifyCallback. Your Vpi have problem\n");
												}
												else
												{
													OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"NotifyCallback successfully loaded\n");
													m_bVpiInitialized=OpcUa_True;
												}
											}
											else
												m_bVpiInitialized=OpcUa_True;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}
OpcUa_String* CVpiFuncCaller::GetLibraryName() 
{
	return m_pLibraryName;
}
void CVpiFuncCaller::SetLibraryName(OpcUa_String libraryName) 
{
	OpcUa_Int32 iLen=OpcUa_String_StrLen(&libraryName);
	if (iLen)
	{
		if (m_pLibraryName)
			OpcUa_String_Clear(m_pLibraryName);
		else
			m_pLibraryName=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
		
		OpcUa_String_Initialize(m_pLibraryName);
		OpcUa_String_CopyTo(&libraryName,m_pLibraryName);
	}
}
OpcUa_Boolean CVpiFuncCaller::IsVpiInitialized()
{
	return m_bVpiInitialized; 
}
void CVpiFuncCaller::VpiInitialized(OpcUa_Boolean bVal)
{
	m_bVpiInitialized = bVal;
}
OpcUa_Boolean CVpiFuncCaller::IsColdStarted() 
{ 
	return m_bColdStartDone; 
}
void CVpiFuncCaller::ColdStarted(OpcUa_Boolean bVal) 
{ 
	m_bColdStartDone = bVal; 
}
OpcUa_Boolean CVpiFuncCaller::IsWarmStarted() 
{ 
	return m_bWarmStartDone; 
}
void CVpiFuncCaller::WarmStarted(OpcUa_Boolean bVal) 
{ 
	m_bWarmStartDone = bVal; 
}