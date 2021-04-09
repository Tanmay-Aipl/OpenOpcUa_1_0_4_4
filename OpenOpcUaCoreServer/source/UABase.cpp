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
//#include <typeinfo>

#include "UABase.h"
#include "Utils.h"
using namespace OpenOpcUa;
using namespace UASharedLib;
using namespace UAAddressSpace;
CUABase::CUABase(void)
{
	m_pBrowseName=OpcUa_Null;
	m_pNodeId = (OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
	OpcUa_NodeId_Initialize(m_pNodeId);
	m_pReferences = new CUAReferenceList();
	OpcUa_LocalizedText_Initialize(&m_DisplayName);
	OpcUa_LocalizedText_Initialize(&m_Description);
	m_pBrowseName = (OpcUa_QualifiedName*)OpcUa_Alloc(sizeof(OpcUa_QualifiedName));
	if (m_pBrowseName)
		OpcUa_QualifiedName_Initialize(m_pBrowseName);
	OpcUa_NodeId_Initialize(&m_TypeDefinition);
}

void CUABase::Init(const char** atts)
{
	OpcUa_NodeId_Initialize(m_pNodeId);
	int ii=0;
	while (atts[ii])
	{
		if (m_NodeClass==OpcUa_NodeClass_Object)
		{
			// On va rechercher le nodeID
			////////////////////////////////////////////////////
			int jj=0;
			while (atts[jj])
			{
				if (OpcUa_StrCmpA(atts[jj],"NodeId")==0)
				{
					// NodeId
					OpcUa_NodeId aNodeId;
					OpcUa_NodeId_Initialize(&aNodeId);
					
					if (ParseNodeId(atts[jj+1],&aNodeId)==OpcUa_Good)
						SetNodeId(aNodeId);
					else
					{
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR," XML file corrupted CUABase::Init \n");
						throw std::exception();
					}
					OpcUa_NodeId_Clear(&aNodeId);
				}
				
				if (OpcUa_StrCmpA(atts[jj],"BrowseName")==0)
				{
					////////////////////////////////////////////////////
					// BrowseName
					OpcUa_QualifiedName* qName=(OpcUa_QualifiedName*)OpcUa_Alloc(sizeof(OpcUa_QualifiedName));
					OpcUa_QualifiedName_Initialize(qName);	
					if (m_pNodeId)
						qName->NamespaceIndex = m_pNodeId->NamespaceIndex;
					if (strlen(atts[jj+1])>0)
					{
						OpcUa_String_AttachCopy(&(qName->Name),OpcUa_StringA(atts[jj+1]));
					}
					else
					{
						OpcUa_String_AttachCopy(&(qName->Name),OpcUa_StringA(" "));
					}
					SetBrowseName(qName);
					OpcUa_QualifiedName_Clear(qName);
					OpcUa_Free(qName);
				}
				//if (OpcUa_StrCmpA(atts[jj],"DisplayName")==0)
				//{
				//	////////////////////////////////////////////////////
				//	// DisplayName
				//	OpcUa_LocalizedText lName;
				//	OpcUa_LocalizedText_Initialize(&lName);
				//	if (strlen(atts[jj+1])>0)
				//	{
				//		OpcUa_String_AttachCopy(&(lName.Locale), "en-us");
				//		OpcUa_String_AttachCopy(&(lName.Text),(OpcUa_StringA)atts[jj+1]);
				//		SetDisplayName(lName);
				//	}
				//	else
				//	{
				//		OpcUa_String_AttachCopy(&(lName.Locale), (OpcUa_StringA)("en"));
				//		OpcUa_String_AttachCopy(&(lName.Text), (OpcUa_StringA)(" "));
				//	}
				//	SetDisplayName(lName);
				//}
				//if (OpcUa_StrCmpA(atts[jj],"Description")==0)
				//{
				//	////////////////////////////////////////////////////
				//	// Description
				//	OpcUa_LocalizedText lName;
				//	OpcUa_LocalizedText_Initialize(&lName);
				//	if (strlen(atts[jj+1])>0)
				//	{
				//		OpcUa_String_AttachCopy(&(lName.Locale), "en-us");
				//		OpcUa_String_AttachCopy(&(lName.Text), (OpcUa_StringA)atts[jj+1]);
				//	}
				//	else
				//	{
				//		OpcUa_String_AttachCopy(&(lName.Locale), (OpcUa_StringA)("en"));
				//		OpcUa_String_AttachCopy(&(lName.Text), (OpcUa_StringA)(" "));
				//	}
				//	SetDescription(lName);
				//}
				if (OpcUa_StrCmpA(atts[jj],"UserWriteMask")==0)
				{
					OpcUa_UInt32_Initialize(&m_UserWriteMask);
					m_UserWriteMask=atoi(atts[jj+1]);
					SetUserWriteMask(m_UserWriteMask);
				}
				if (OpcUa_StrCmpA(atts[jj],"WriteMask")==0)
				{
					OpcUa_UInt32_Initialize(&m_WriteMask);
					m_WriteMask=atoi(atts[jj+1]);
					SetWriteMask(m_WriteMask);
				}
				jj=jj+2;
			}
		}
		else
		{
			if (m_NodeClass==OpcUa_NodeClass_Variable)
			{
				int jj=0;
				while (atts[jj])
				{
					if (OpcUa_StrCmpA(atts[jj],"NodeId")==0)
					{
						// NodeId
						OpcUa_NodeId aNodeId;
						OpcUa_NodeId_Initialize(&aNodeId);
						
						if (ParseNodeId(atts[jj+1],&aNodeId)==OpcUa_Good)
							SetNodeId(aNodeId);
						else
						{
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR," XML file corrupted CUABase::Init\n");
						}
						OpcUa_NodeId_Clear(&aNodeId);
					}
				
					
					if (OpcUa_StrCmpA(atts[jj],"BrowseName")==0)
					{
						////////////////////////////////////////////////////
						// BrowseName
						OpcUa_QualifiedName qName;
						OpcUa_QualifiedName_Initialize(&qName);	
						//int iSize=strlen(atts[jj+1]);
						if (strlen(atts[jj+1])>0)
						{
							OpcUa_String_AttachCopy(&(qName.Name),OpcUa_StringA(atts[jj+1]));								
							SetBrowseName(&qName);
						}
						OpcUa_QualifiedName_Clear(&qName);
					}
					//if (OpcUa_StrCmpA(atts[jj],"DisplayName")==0)
					//{
					//	////////////////////////////////////////////////////
					//	// DisplayName
					//	OpcUa_LocalizedText lName;
					//	OpcUa_LocalizedText_Initialize(&lName);
					//	
					//	if (strlen(atts[jj+1])>0)
					//	{
					//		OpcUa_String_AttachCopy(&(lName.Locale), "en-us");
					//		OpcUa_String_AttachCopy(&(lName.Text), (OpcUa_StringA)atts[jj+1]);
					//		SetDisplayName(lName);
					//	}
					//}
					//if (OpcUa_StrCmpA(atts[jj],"Description")==0)
					//{
					//	////////////////////////////////////////////////////
					//	// Description
					//	OpcUa_LocalizedText lName;
					//	OpcUa_LocalizedText_Initialize(&lName);
					//	if (strlen(atts[jj+1])>0)
					//	{
					//		OpcUa_String_AttachCopy(&(lName.Locale), "en-us");
					//		OpcUa_String_AttachCopy(&(lName.Text), (OpcUa_StringA)atts[jj+1]);
					//	}
					//	else
					//	{
					//		OpcUa_String_AttachCopy(&(lName.Locale), "en-us");
					//		OpcUa_String_AttachCopy(&(lName.Text), (OpcUa_StringA)(" "));
					//	}
					//	SetDescription(lName);
					//}
					if (OpcUa_StrCmpA(atts[jj],"UserWriteMask")==0)
					{
						m_UserWriteMask=atoi(atts[jj+1]);
					}
					if (OpcUa_StrCmpA(atts[jj],"WriteMask")==0)
					{
						m_WriteMask=atoi(atts[jj+1]);
					}
					jj=jj+2;
				}

			}
			else
			{
				if (m_NodeClass==OpcUa_NodeClass_Method)
				{
					int jj=0;
					while (atts[jj])
					{
						if (OpcUa_StrCmpA(atts[jj],"NodeId")==0)
						{
							// NodeId
							OpcUa_NodeId aNodeId;
							OpcUa_NodeId_Initialize(&aNodeId);
							
							if (ParseNodeId(atts[jj+1],&aNodeId)==OpcUa_Good)
								SetNodeId(aNodeId);
							else
							{
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR," XML file corrupted CUABase::Init\n");
							}
							OpcUa_NodeId_Clear(&aNodeId);
						}					
						
						if (OpcUa_StrCmpA(atts[jj],"BrowseName")==0)
						{
							////////////////////////////////////////////////////
							// BrowseName
							OpcUa_QualifiedName qName;
							OpcUa_QualifiedName_Initialize(&qName);	
							if (strlen(atts[jj+1])>0)
							{
								OpcUa_String_AttachCopy(&(qName.Name),OpcUa_StringA(atts[jj+1]));
								
								SetBrowseName(&qName);
							}
							OpcUa_QualifiedName_Clear(&qName);
						}
						//if (OpcUa_StrCmpA(atts[jj],"DisplayName")==0)
						//{
						//	////////////////////////////////////////////////////
						//	// DisplayName
						//	OpcUa_LocalizedText lName;
						//	OpcUa_LocalizedText_Initialize(&lName);
						//	if (strlen(atts[jj+1])>0)
						//	{
						//		//lName.Locale.strContent=(OpcUa_CharA*)OpcUa_Alloc(strlen(atts[jj+1]));
						//		OpcUa_String_AttachCopy(&(lName.Locale), "en-us");
						//		//lName.Text.strContent=(OpcUa_CharA*)OpcUa_Alloc(strlen(atts[jj+1]));
						//		OpcUa_String_AttachCopy(&(lName.Text), (OpcUa_StringA)atts[jj+1]);
						//		SetDisplayName(lName);
						//	}
						//}
						//if (OpcUa_StrCmpA(atts[jj],"Description")==0)
						//{
						//	////////////////////////////////////////////////////
						//	// Description
						//	OpcUa_LocalizedText lName;
						//	OpcUa_LocalizedText_Initialize(&lName);
						//	if (strlen(atts[jj+1])>0)
						//	{
						//		//lName.Locale.strContent=(OpcUa_CharA*)OpcUa_Alloc(strlen(atts[jj+1]));
						//		OpcUa_String_AttachCopy(&(lName.Locale), "en-us");
						//		//lName.Text.strContent=(OpcUa_CharA*)OpcUa_Alloc(strlen(atts[jj+1]));
						//		OpcUa_String_AttachCopy(&(lName.Text), (OpcUa_StringA)atts[jj+1]);
						//		SetDescription(lName);
						//	}
						//}
						if (OpcUa_StrCmpA(atts[jj],"UserWriteMask")==0)
						{
							m_UserWriteMask=atoi(atts[jj+1]);
						}
						if (OpcUa_StrCmpA(atts[jj],"WriteMask")==0)
						{
							m_UserWriteMask=atoi(atts[jj+1]);
						}
						jj=jj+2;
					}
				}
				else
				{
					if (m_NodeClass==OpcUa_NodeClass_ObjectType)
					{
						int jj=0;
						while (atts[jj])
						{
							if (OpcUa_StrCmpA(atts[jj],"NodeId")==0)
							{
								// NodeId
								OpcUa_NodeId aNodeId;
								OpcUa_NodeId_Initialize(&aNodeId);
								
								if (ParseNodeId(atts[jj+1],&aNodeId)==OpcUa_Good)
									SetNodeId(aNodeId);
								else
								{
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR," XML file corrupted CUABase::Init\n");
								}
								OpcUa_NodeId_Clear(&aNodeId);
							}				
							
							if (OpcUa_StrCmpA(atts[jj],"BrowseName")==0)
							{
								// BrowseName
								OpcUa_QualifiedName qName;
								OpcUa_QualifiedName_Initialize(&qName);	
								if (strlen(atts[jj+1])>0)
								{
									OpcUa_String_AttachCopy(&(qName.Name),OpcUa_StringA(atts[jj+1]));
									
									SetBrowseName(&qName);
								}
								OpcUa_QualifiedName_Clear(&qName);
							}
							//if (OpcUa_StrCmpA(atts[jj],"DisplayName")==0)
							//{
							//	////////////////////////////////////////////////////
							//	// DisplayName
							//	OpcUa_LocalizedText lName;
							//	OpcUa_LocalizedText_Initialize(&lName);
							//	if (strlen(atts[jj+1])>0)
							//	{
							//		OpcUa_String_AttachCopy(&(lName.Locale), "en-us");
							//		OpcUa_String_AttachCopy(&(lName.Text), (OpcUa_StringA)atts[jj+1]);
							//		SetDisplayName(lName);
							//	}
							//}
							//if (OpcUa_StrCmpA(atts[jj],"Description")==0)
							//{
							//	////////////////////////////////////////////////////
							//	// Description
							//	OpcUa_LocalizedText lName;
							//	OpcUa_LocalizedText_Initialize(&lName);
							//	if (strlen(atts[jj+1])>0)
							//	{
							//		OpcUa_String_AttachCopy(&(lName.Locale), "en-us");
							//		OpcUa_String_AttachCopy(&(lName.Text), (OpcUa_StringA)atts[jj+1]);
							//		SetDescription(lName);
							//	}
							//}
							if (OpcUa_StrCmpA(atts[jj],"UserWriteMask")==0)
							{
								m_UserWriteMask=atoi(atts[jj+1]);
							}
							if (OpcUa_StrCmpA(atts[jj],"WriteMask")==0)
							{
								m_UserWriteMask=atoi(atts[jj+1]);
							}
							jj=jj+2;
						}
					}
					else
					{
						if (m_NodeClass==OpcUa_NodeClass_VariableType)
						{
							int jj=0;
							while (atts[jj])
							{
								if (OpcUa_StrCmpA(atts[jj],"NodeId")==0)
								{
									// NodeId
									OpcUa_NodeId aNodeId;
									OpcUa_NodeId_Initialize(&aNodeId);
									
									if (ParseNodeId(atts[jj+1],&aNodeId)==OpcUa_Good)
										SetNodeId(aNodeId);
									else
									{
										OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR," XML file corrupted CUABase::Init\n");
									}
									OpcUa_NodeId_Clear(&aNodeId);
								}										
								if (OpcUa_StrCmpA(atts[jj],"BrowseName")==0)
								{
									////////////////////////////////////////////////////
									// BrowseName
									OpcUa_QualifiedName qName;
									OpcUa_QualifiedName_Initialize(&qName);	
									if (strlen(atts[jj+1])>0)
									{
										OpcUa_String_AttachCopy(&(qName.Name),OpcUa_StringA(atts[jj+1]));												
										SetBrowseName(&qName);
									}
									OpcUa_QualifiedName_Clear(&qName);
								}
								//if (OpcUa_StrCmpA(atts[jj],"DisplayName")==0)
								//{
								//	////////////////////////////////////////////////////
								//	// DisplayName
								//	OpcUa_LocalizedText lName;
								//	OpcUa_LocalizedText_Initialize(&lName);
								//	if (strlen(atts[jj+1])>0)
								//	{
								//		OpcUa_String_AttachCopy(&(lName.Locale), "en-us");
								//		OpcUa_String_AttachCopy(&(lName.Text), (OpcUa_StringA)atts[jj+1]);
								//		SetDisplayName(lName);
								//	}
								//}
								//if (OpcUa_StrCmpA(atts[jj],"Description")==0)
								//{
								//	////////////////////////////////////////////////////
								//	// Description
								//	OpcUa_LocalizedText lName;
								//	OpcUa_LocalizedText_Initialize(&lName);
								//	if (strlen(atts[jj+1])>0)
								//	{
								//		OpcUa_String_AttachCopy(&(lName.Locale), "en-us");
								//		OpcUa_String_AttachCopy(&(lName.Text), (OpcUa_StringA)atts[jj+1]);
								//		SetDescription(lName);
								//	}
								//}
								if (OpcUa_StrCmpA(atts[jj],"UserWriteMask")==0)
								{
									m_UserWriteMask=atoi(atts[jj+1]);
								}
								if (OpcUa_StrCmpA(atts[jj],"WriteMask")==0)
								{
									m_UserWriteMask=atoi(atts[jj+1]);
								}
								jj=jj+2;
							}
						}
						else
						{
							if (m_NodeClass==OpcUa_NodeClass_ReferenceType)
							{
								int jj=0;
								while (atts[jj])
								{
									if (OpcUa_StrCmpA(atts[jj],"NodeId")==0)
									{
										// NodeId
										OpcUa_NodeId aNodeId;
										OpcUa_NodeId_Initialize(&aNodeId);
										
										if (ParseNodeId(atts[jj+1],&aNodeId)==OpcUa_Good)
											SetNodeId(aNodeId);
										else
										{
											OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR," XML file corrupted CUABase::Init\n");
										}
										OpcUa_NodeId_Clear(&aNodeId);
									}
									
									if (OpcUa_StrCmpA(atts[jj],"BrowseName")==0)
									{
										////////////////////////////////////////////////////
										// BrowseName
										OpcUa_QualifiedName qName;
										OpcUa_QualifiedName_Initialize(&qName);	
										int iLen=strlen(atts[jj+1]);
										if (iLen)
										{
											OpcUa_String_AttachCopy(&(qName.Name), (OpcUa_StringA)atts[jj+1]);											
											SetBrowseName(&qName);
										}
										OpcUa_QualifiedName_Clear(&qName);
									}
									//if (OpcUa_StrCmpA(atts[jj],"DisplayName")==0)
									//{
									//	////////////////////////////////////////////////////
									//	// DisplayName
									//	OpcUa_LocalizedText lName;
									//	OpcUa_LocalizedText_Initialize(&lName);
									//	int iLen=strlen(atts[jj+1]);
									//	if (iLen>0)
									//	{
									//		OpcUa_String_AttachCopy(&(lName.Locale), "en-us");
									//		OpcUa_String_AttachCopy(&(lName.Text), (OpcUa_StringA)atts[jj+1]);
									//	}
									//	else
									//	{
									//		OpcUa_String_AttachCopy(&(lName.Locale), "en-us");
									//		OpcUa_String_AttachCopy(&(lName.Text), (OpcUa_StringA)(" "));
									//	}
									//	SetDisplayName(lName);
									//}
									//if (OpcUa_StrCmpA(atts[jj],"Description")==0)
									//{
									//	////////////////////////////////////////////////////
									//	// Description
									//	OpcUa_LocalizedText lName;
									//	OpcUa_LocalizedText_Initialize(&lName);
									//	int iLen=strlen(atts[jj+1]);
									//	if (iLen)
									//	{
									//		OpcUa_String_AttachCopy(&(lName.Locale), "en-us");
									//		OpcUa_String_AttachCopy(&(lName.Text), (OpcUa_StringA)atts[jj+1]);
									//	}
									//	else
									//	{
									//		OpcUa_String_AttachCopy(&(lName.Locale), "en-us");
									//		OpcUa_String_AttachCopy(&(lName.Text), (OpcUa_StringA)(" "));
									//	}
									//	SetDescription(lName);
									//}
									if (OpcUa_StrCmpA(atts[jj],"UserWriteMask")==0)
									{
										m_UserWriteMask=atoi(atts[jj+1]);
									}
									if (OpcUa_StrCmpA(atts[jj],"WriteMask")==0)
									{
										m_WriteMask=atoi(atts[jj+1]);
									}
									jj+=2;
								}
							}
							else
							{
								if (m_NodeClass==OpcUa_NodeClass_DataType)
								{
									int jj=0;
									while (atts[jj])
									{
										if (OpcUa_StrCmpA(atts[jj],"NodeId")==0)
										{
											// NodeId
											OpcUa_NodeId aNodeId;
											OpcUa_NodeId_Initialize(&aNodeId);
											
											if (ParseNodeId(atts[jj+1],&aNodeId)==OpcUa_Good)
												SetNodeId(aNodeId);
											else
											{
												OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR," XML file corrupted CUABase::Init\n");
											}
											OpcUa_NodeId_Clear(&aNodeId);
										}
																
										if (OpcUa_StrCmpA(atts[jj],"BrowseName")==0)
										{
											////////////////////////////////////////////////////
											// BrowseName
											OpcUa_QualifiedName qName;
											OpcUa_QualifiedName_Initialize(&qName);	
											if (strlen(atts[jj+1])>0)
											{
												OpcUa_String_AttachCopy(&(qName.Name),atts[jj+1]);														
												SetBrowseName(&qName);
											}
											OpcUa_QualifiedName_Clear(&qName);
										}

										if (OpcUa_StrCmpA(atts[jj],"UserWriteMask")==0)
										{
											m_UserWriteMask=atoi(atts[jj+1]);
										}
										if (OpcUa_StrCmpA(atts[jj],"WriteMask")==0)
										{
											m_WriteMask=atoi(atts[jj+1]);
										}
										jj=jj+2;
									}
								}
								else
								{
									if (m_NodeClass==OpcUa_NodeClass_View)
									{
										int jj=0;
										while (atts[jj])
										{
											if (OpcUa_StrCmpA(atts[jj],"NodeId")==0)
											{
												// NodeId
												OpcUa_NodeId aNodeId;
												OpcUa_NodeId_Initialize(&aNodeId);
												
												if (ParseNodeId(atts[jj+1],&aNodeId)==OpcUa_Good)
													SetNodeId(aNodeId);
												else
												{
													OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR," XML file corrupted CUABase::Init\n");
												}
												OpcUa_NodeId_Clear(&aNodeId);
											}						
											
											if (OpcUa_StrCmpA(atts[jj],"BrowseName")==0)
											{
												////////////////////////////////////////////////////
												// BrowseName
												OpcUa_QualifiedName qName;
												OpcUa_QualifiedName_Initialize(&qName);	
												if (strlen(atts[jj+1])>0)
												{
													OpcUa_String_AttachCopy(&(qName.Name),OpcUa_StringA(atts[jj+1]));													
													SetBrowseName(&qName);
												}
												OpcUa_QualifiedName_Clear(&qName);
											}
											//if (OpcUa_StrCmpA(atts[jj],"DisplayName")==0)
											//{
											//	////////////////////////////////////////////////////
											//	// DisplayName
											//	OpcUa_LocalizedText lName;
											//	OpcUa_LocalizedText_Initialize(&lName);
											//	if (strlen(atts[jj+1])>0)
											//	{
											//		OpcUa_String_AttachCopy(&(lName.Locale), "en-us");
											//		OpcUa_String_AttachCopy(&(lName.Text), (OpcUa_StringA)atts[jj+1]);
											//		SetDisplayName(lName);
											//	}
											//}
											//if (OpcUa_StrCmpA(atts[jj],"Description")==0)
											//{
											//	////////////////////////////////////////////////////
											//	// Description
											//	OpcUa_LocalizedText lName;
											//	OpcUa_LocalizedText_Initialize(&lName);
											//	if (strlen(atts[jj+1])>0)
											//	{
											//		OpcUa_String_AttachCopy(&(lName.Locale), "en-us");
											//		OpcUa_String_AttachCopy(&(lName.Text), (OpcUa_StringA)atts[jj+1]);
											//		SetDescription(lName);
											//	}
											//}
											if (OpcUa_StrCmpA(atts[jj],"UserWriteMask")==0)
											{
												m_UserWriteMask=atoi(atts[jj+1]);
											}
											if (OpcUa_StrCmpA(atts[jj],"WriteMask")==0)
											{
												m_WriteMask=atoi(atts[jj+1]);
											}
											jj=jj+2;
										}
									}
									else
									{
										// Nodeclass inconnue ... Erreur de configuration
									}
								}
							}
						}
					}
				}
			}
		}
		ii=ii+2;
	}
}
CUABase::CUABase(OpcUa_NodeClass aNodeClass,const char **atts):m_NodeClass(aNodeClass)
{	
	// initialisation des valeurs par défaut 
	OpcUa_LocalizedText_Initialize(&m_DisplayName);
	OpcUa_LocalizedText_Initialize(&m_Description);
	m_pBrowseName= (OpcUa_QualifiedName*)OpcUa_Alloc(sizeof(OpcUa_QualifiedName));
	if (m_pBrowseName)
		OpcUa_QualifiedName_Initialize(m_pBrowseName);

	m_UserWriteMask=0;
	m_WriteMask=0;
	OpcUa_NodeId_Initialize(&m_TypeDefinition);
	//
	m_pNodeId = (OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
	OpcUa_NodeId_Initialize(m_pNodeId);
	// initialisation des variables de classe a partir des attributs de l'element XML
	m_pReferences=new CUAReferenceList();
	Init(atts);
	// DisplayName
	OpcUa_LocalizedText lName=GetDisplayName();
	if (OpcUa_String_StrLen(&(lName.Text))<=0)
	{
		OpcUa_QualifiedName* aDefaultValue=GetBrowseName();
		OpcUa_Int32 iSize=OpcUa_String_StrLen(&(aDefaultValue->Name));// DisplayName
		if (iSize>0)
		{
			OpcUa_LocalizedText_Initialize(&lName);
			OpcUa_String_AttachCopy(&(lName.Locale), "en-us");
			OpcUa_String_StrnCpy(&(lName.Text),&(aDefaultValue->Name),iSize);
			SetDisplayName(&lName);
			OpcUa_LocalizedText_Clear(&lName);
		}
	}
	//OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"New Node>DisplayName=%s\n",OpcUa_String_GetRawString(&(lName.Text)));
	// Description
	OpcUa_LocalizedText lName1=GetDescription();
	if (OpcUa_String_StrLen(&(lName1.Text))<=0)
	{
		OpcUa_QualifiedName* pDefaultValue=GetBrowseName();
		OpcUa_Int32 iSize=OpcUa_String_StrLen(&(pDefaultValue->Name));// DisplayName
		if (iSize>0)
		{
			OpcUa_LocalizedText_Initialize(&lName1);

			OpcUa_String_AttachCopy(&(lName1.Locale), "en-us");
			OpcUa_String_CopyTo(&(pDefaultValue->Name), &(lName1.Text));
			SetDescription(lName1);
			OpcUa_LocalizedText_Clear(&lName1);
		}
	}
}
CUABase::~CUABase(void)
{
	//if ((m_NodeId.Identifier.Numeric==1) && (m_NodeId.NamespaceIndex==1) )
	//	printf("debug purpose>100\n");
	OpcUa_LocalizedText_Clear(&m_DisplayName);
	if (m_pReferences)
	{
		CUAReferenceList::iterator it;
		for (it = m_pReferences->begin(); it != m_pReferences->end(); it++)
		{
			CUAReference* pUAReference = *it;
			delete pUAReference;
		}
		m_pReferences->clear();
		delete m_pReferences;
	}
	OpcUa_LocalizedText_Clear(&m_Description);
	if (m_pBrowseName)
	{
		OpcUa_QualifiedName_Clear(m_pBrowseName);
		OpcUa_Free(m_pBrowseName);
		m_pBrowseName = OpcUa_Null;
	}
	OpcUa_NodeId_Clear(m_pNodeId);
	OpcUa_Free(m_pNodeId);
	m_pNodeId = OpcUa_Null;
	OpcUa_NodeId_Clear(&m_TypeDefinition);
}
OpcUa_StatusCode CUABase::Write(OpcUa_UInt32 AttributeId, OpcUa_String /*szIndexRange*/, OpcUa_DataValue Value)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (Utils::IsWritable(AttributeId,GetWriteMask()))
	{
		if (AttributeId==OpcUa_Attributes_NodeId)
		{
			// changement de BrowseName
		}
		if (AttributeId==OpcUa_Attributes_NodeClass)
		{
			// changement de NodeClass
			if (Value.Value.Datatype == OpcUaType_QualifiedName)
				SetBrowseName(Value.Value.Value.QualifiedName);
			else
				uStatus = OpcUa_BadInvalidArgument;
		}
		if (AttributeId==OpcUa_Attributes_BrowseName)
		{
			// changement de BrowseName
		}
		if (AttributeId==OpcUa_Attributes_DisplayName)
		{
			// changement de DisplayName
			if (Value.Value.Datatype == OpcUaType_LocalizedText)
			{
				if (Value.Value.Value.LocalizedText)
					SetDisplayName(Value.Value.Value.LocalizedText);
				else
					uStatus = OpcUa_BadInvalidArgument;
			}
			else
				uStatus = OpcUa_BadInvalidArgument;
		}
		if (AttributeId==OpcUa_Attributes_Description)
		{
			// changement de Description
			if (Value.Value.Datatype == OpcUaType_LocalizedText)
			{
				if (Value.Value.Value.LocalizedText)
					SetDescription(*(Value.Value.Value.LocalizedText));
				else
					uStatus = OpcUa_BadInvalidArgument;
			}
			else
				uStatus = OpcUa_BadInvalidArgument;
		}
		if (AttributeId==OpcUa_Attributes_WriteMask)
		{
			// changement de WriteMask
			if (Value.Value.Datatype == OpcUaType_UInt32)
				SetWriteMask(Value.Value.Value.UInt32);
			else
				uStatus = OpcUa_BadInvalidArgument;
		}
		if (AttributeId==OpcUa_Attributes_UserWriteMask)
		{
			// changement de UserWriteMask
			if (Value.Value.Datatype == OpcUaType_UInt32)
				SetUserWriteMask(Value.Value.Value.UInt32);
			else
				uStatus = OpcUa_BadInvalidArgument;
		}
	}
	else
		uStatus=OpcUa_BadAttributeIdInvalid;
	return uStatus;
}
// détermine si la reference pRefNode existe déja sur cette node
// Return OpcUa_BadNodeIdExists if the node already exists
OpcUa_StatusCode CUABase::IsReferenceExist(CUAReference* pRefNode)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_UInt32 iSize=m_pReferences->size();
	OpcUa_ExpandedNodeId aRefTargetNodeId=pRefNode->GetTargetId();
	for (OpcUa_UInt32 ii=0;ii<iSize;ii++)
	{
		CUAReference* pReference=m_pReferences->at(ii);
		if (m_pReferences)
		{
			OpcUa_ExpandedNodeId aExpandedTargetNodeId = pReference->GetTargetId();
			OpcUa_NodeId aReferenceTypeId = pReference->GetReferenceTypeId();
			OpcUa_NodeId  aRefNodeTypeId = pRefNode->GetReferenceTypeId();

			if ((Utils::IsEqual(&aExpandedTargetNodeId, &aRefTargetNodeId))
				&& (Utils::IsEqual(&aReferenceTypeId, &aRefNodeTypeId))
				&& (pReference->IsInverse() == pRefNode->IsInverse()))
			{
				uStatus = OpcUa_BadNodeIdExists;
				break;
			}
		}
		else
			uStatus = OpcUa_BadInternalError;
	}
	return uStatus;
}
void CUABase::SetNodeId(OpcUa_NodeId aNodeId)
{
	if (m_pNodeId)
	{
		OpcUa_NodeId_Clear(m_pNodeId);
		OpcUa_Free(m_pNodeId);
		m_pNodeId = OpcUa_Null;
	}
	m_pNodeId = (OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
	OpcUa_NodeId_Initialize(m_pNodeId);
	OpcUa_NodeId_CopyTo(&aNodeId,m_pNodeId);
}
void CUABase::SetTypeDefinition(OpcUa_NodeId* pNodeId) 
{
	if (pNodeId)
		OpcUa_NodeId_CopyTo(pNodeId,&m_TypeDefinition);
}
CUABase CUABase::operator=(CUABase* pUABase)
{
	OpcUa_NodeId_CopyTo(pUABase->m_pNodeId,m_pNodeId);	
	m_NodeClass=pUABase->m_NodeClass;
	m_pBrowseName=pUABase->m_pBrowseName;
	m_DisplayName=pUABase->m_DisplayName;
	m_Description=pUABase->m_Description;
	m_WriteMask=pUABase->m_WriteMask;
	m_UserWriteMask=pUABase->m_UserWriteMask;
	CUAReferenceList::iterator myIterator;
	for (myIterator=pUABase->m_pReferences->begin();myIterator!=pUABase->m_pReferences->end();myIterator++)
	{
		CUAReference* pReference=*myIterator;
		m_pReferences->push_back(pReference);
	}
	return *this;
}

CUAReferenceList* CUABase::GetReferenceNodeList() 
{ 
	return m_pReferences; 
}
OpcUa_NodeId CUABase::GetTypeDefinition() const 
{ 
	return m_TypeDefinition; 
}

OpcUa_NodeId* CUABase::GetNodeId()
{
	return m_pNodeId;
}

// NodeClass
OpcUa_NodeClass CUABase::GetNodeClass() const
{
	return m_NodeClass;
}
void CUABase::SetNodeClass(OpcUa_NodeClass aNodeClass)
{
	m_NodeClass = aNodeClass;
}
// BrowseName
OpcUa_QualifiedName* CUABase::GetBrowseName() const
{
	if (m_pNodeId)
		m_pBrowseName->NamespaceIndex = m_pNodeId->NamespaceIndex;
	// Exception
	char* cleanedName = OpcUa_String_GetRawString(&(m_pBrowseName->Name));
	if (cleanedName)
	{
		if (OpcUa_StrCmpA(cleanedName, "EnumStrings") == 0)
			m_pBrowseName->NamespaceIndex = 0;
	}
	return m_pBrowseName;
}
void CUABase::SetBrowseName(OpcUa_QualifiedName* aName)
{
	if (aName)
	{
		OpcUa_QualifiedName_CopyTo(aName, m_pBrowseName);
	}
}
OpcUa_LocalizedText CUABase::GetDisplayName() const
{
	return m_DisplayName;
}
void CUABase::SetDisplayName(OpcUa_LocalizedText* pValue)
{
	if (pValue)
	{
		OpcUa_LocalizedText_Clear(&m_DisplayName);
		OpcUa_LocalizedText_Initialize(&m_DisplayName);
		OpcUa_LocalizedText_CopyTo(pValue, &m_DisplayName);
	}
}
OpcUa_LocalizedText CUABase::GetDescription()
{
	return m_Description;
}
void CUABase::SetDescription(OpcUa_LocalizedText aValue)
{
	//OpcUa_LocalizedText_Initialize(&m_Description);
	OpcUa_LocalizedText_CopyTo(&aValue, &m_Description);
}
OpcUa_UInt32 CUABase::GetWriteMask()
{
	return m_WriteMask;
}
void CUABase::SetWriteMask(OpcUa_UInt32 aValue)
{
	m_WriteMask = aValue;
}
OpcUa_UInt32 CUABase::GetUserWriteMask()
{
	return m_UserWriteMask;
}
void CUABase::SetUserWriteMask(OpcUa_UInt32 aValue)
{
	m_UserWriteMask = aValue;
}
OpcUa_Int32 CUABase::GetNoOfReferences()
{
	if (m_pReferences)
		return m_pReferences->size();
	else
		return 0;
}
CUAReference* CUABase::operator[](int index)
{
	int ii = 0;
	if (m_pReferences)
	{
		for (OpcUa_UInt16 iii = 0; iii<m_pReferences->size(); iii++)
		{
			CUAReference* aRefNode = m_pReferences->at(iii);
			if (ii++ == index)
				return aRefNode;
		}
	}
	return OpcUa_Null;
}