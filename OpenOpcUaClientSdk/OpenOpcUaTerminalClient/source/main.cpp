
#include <stdio.h>
#include <stdlib.h>
#ifdef _GNUC_
#include <unistd.h>
#endif
#include <string.h>
#include "OpenOpcUaSdkClient.h"
OpcUa_StatusCode CreateSessionOpc(char* url, char* szSecurity);
OpcUa_StatusCode ReadOpcValue(OpcUa_NodeId nodeId);
OpcUa_StatusCode WriteOpcValue();
OpcUa_StatusCode WriteMultipleOpcValue();
OpcUa_Handle m_hApplication=OpcUa_Null;
OpcUa_Handle m_hSession=OpcUa_Null;
OpcUa_Handle m_hSubscription=OpcUa_Null;
OpcUa_Variant gLoopValue;
OpcUa_Variant gWriteValue;

int getQChar()
{
	int Choice = 0;
	OpcUa_StatusCode uStatus;
	OpcUa_Boolean bRun = OpcUa_True;
	OpcUa_Variant_Initialize(&gLoopValue);
	printf("Press q or Q to Q - w or W to increment ns=2;i=16\n");
	while (bRun)
	{
		Choice = getchar();
		switch (Choice)
		{
		case 0x77:
		case 0x57:
			//uStatus=WriteOpcValue();
			for (OpcUa_UInt16 i=0;i<100;i++)
				uStatus = WriteMultipleOpcValue();
			printf("Write>uStatus=%ld\n", uStatus);
			break;
		case 0x51:
		case 0x71:
			bRun = OpcUa_False;
			break;
		default:
			break;
		}
	}
	return Choice;
}
OpcUa_StatusCode __stdcall notificationMessage(OpcUa_Handle hSubscription, OpcUa_Int32 NoOfMonitoredItems,
	OpcUa_MonitoredItemNotification* MonitoredItems, void* pParamData)
{
	OpcUa_String *valeur = OpcUa_Null;
	for (OpcUa_Int32 i = 0; i < NoOfMonitoredItems; i++)
	{
		if (OpenOpcUa_VariantToString(MonitoredItems[i].Value.Value, &valeur) == OpcUa_Good)
		{
			OpcUa_Handle hMonitoredItem = (OpcUa_Handle)MonitoredItems[i].ClientHandle;
			OpcUa_Variant_Clear(&gLoopValue);
			OpcUa_Variant_CopyTo(&(MonitoredItems[i].Value.Value), &gLoopValue);
			/*printf("Receive in the delegate %s on ClientHandle 0x%x\n", 
				OpcUa_String_GetRawString(valeur), (unsigned int)hMonitoredItem);*/
			OpcUa_String_Clear(valeur);
			OpcUa_Free(valeur);
			valeur = OpcUa_Null;
		}
	}
	
	return OpcUa_Good;
}

OpcUa_StatusCode createSubscription()
{
	OpcUa_StatusCode uStatus;
	/*	OpcUa_Double dblPublishingInterval = 1000;
	OpcUa_UInt32 uiLifeTimeCount = 0;
	OpcUa_UInt32 uiKeepAliveCount = 0;
	OpcUa_UInt32 uiMaxNotificationPerPublish = 0;
	OpcUa_Boolean bPublishingEnabled = 1;
	OpcUa_UInt32 priority = 0;

	uStatus = OpenOpcUa_CreateSubscription(
		m_hApplication, m_hSession,
		&dblPublishingInterval,
		&uiLifeTimeCount,
		&uiKeepAliveCount,
		uiMaxNotificationPerPublish,
		bPublishingEnabled,
		priority,
		&m_hSubscription);

	if (uStatus == OpcUa_Good)
	{
		int aVal = 1;
		printf("subscription ok\n");
		uStatus = OpenOpcUa_SetPublishCallback(m_hApplication, m_hSession, 
			m_hSubscription, (PFUNC)notificationMessage, &aVal);
		if (uStatus == OpcUa_Good)
			printf("setPublishCallBack ok\n");
	}*/
	return uStatus;
}
OpcUa_StatusCode CreateMonitoredItem(OpcUa_NodeId aNodeId, OpcUa_UInt32 AttributeId)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_UInt32 iNoOfItemsToCreate = 1;
	OpcUa_MonitoredItemToCreate* pMonitoredItemToCreate = (OpcUa_MonitoredItemToCreate*)OpcUa_Alloc(sizeof(OpcUa_MonitoredItemToCreate)*iNoOfItemsToCreate);
	if (pMonitoredItemToCreate)
	{
		OpcUa_MonitoredItemCreated* pMonitoredItemCreated = OpcUa_Null;

		memset(pMonitoredItemToCreate, 0, sizeof(OpcUa_MonitoredItemToCreate)*iNoOfItemsToCreate);
		OpcUa_NodeId_CopyTo(&aNodeId, &(pMonitoredItemToCreate->m_NodeId));
		pMonitoredItemToCreate->m_AttributeId = OpcUa_Attributes_Value;
		pMonitoredItemToCreate->m_ClientHandle = 1;
		pMonitoredItemToCreate->m_DatachangeTrigger = OpcUa_DataChangeTrigger_StatusValueTimestamp;
		pMonitoredItemToCreate->m_DeadbandType = 0; // 0 for None, 1 for Absolute, 2 for Percent
		pMonitoredItemToCreate->m_DeadbandValue = 0;
		pMonitoredItemToCreate->m_DiscardOldest = OpcUa_False;
		OpcUa_String_Initialize(&(pMonitoredItemToCreate->m_IndexRange));
		pMonitoredItemToCreate->m_MonitoringMode = OpcUa_MonitoringMode_Reporting;
		pMonitoredItemToCreate->m_QueueSize = 0;
		pMonitoredItemToCreate->m_SamplingInterval = 0;
		pMonitoredItemToCreate->m_TimestampsToReturn = OpcUa_TimestampsToReturn_Both;
		uStatus = OpenOpcUa_CreateMonitoredItemsEx(
			m_hApplication,
			m_hSession,
			m_hSubscription,
			iNoOfItemsToCreate,
			pMonitoredItemToCreate,
			&pMonitoredItemCreated);
		if (pMonitoredItemCreated)
		{
			OpenOpcUa_InternalNode* pInternalNode;
			OpenOpcUa_GetInternalNode(m_hApplication, m_hSession, m_hSubscription, pMonitoredItemCreated[0].m_hMonitoredItem, &pInternalNode);
			OpcUa_Free(pMonitoredItemCreated);
		}
		OpcUa_Free(pMonitoredItemToCreate);
	}
	else
		uStatus = OpcUa_BadOutOfMemory;
	return uStatus;
}
OpcUa_StatusCode AddMultipleMonitoredItemString()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_UInt32 iNoOfItemsToCreate = 5;
	OpcUa_MonitoredItemToCreate* pMonitoredItemToCreate = (OpcUa_MonitoredItemToCreate*)OpcUa_Alloc(sizeof(OpcUa_MonitoredItemToCreate)*iNoOfItemsToCreate);

	if (pMonitoredItemToCreate)
	{
		memset(pMonitoredItemToCreate, 0, sizeof(OpcUa_MonitoredItemToCreate)*iNoOfItemsToCreate);
		OpcUa_MonitoredItemCreated* pMonitoredItemCreated = OpcUa_Null;
		OpcUa_UInt32 i = 0;

		OpcUa_NodeId aNodeId;
		OpcUa_NodeId_Initialize(&aNodeId);
		aNodeId.IdentifierType = OpcUa_IdentifierType_String;
		aNodeId.NamespaceIndex = 2;
		OpcUa_String_AttachCopy(&aNodeId.Identifier.String, "10#ETAT");

		OpcUa_NodeId_CopyTo(&aNodeId, &(pMonitoredItemToCreate[i].m_NodeId));
		pMonitoredItemToCreate[i].m_AttributeId = OpcUa_Attributes_Value;
		pMonitoredItemToCreate[i].m_ClientHandle = i; // Will be handled by the library so no need to initialize it
		pMonitoredItemToCreate[i].m_DatachangeTrigger = OpcUa_DataChangeTrigger_StatusValueTimestamp;
		pMonitoredItemToCreate[i].m_DeadbandType = 0; // 0 for None, 1 for Absolute, 2 for Percent
		pMonitoredItemToCreate[i].m_DeadbandValue = 0;
		pMonitoredItemToCreate[i].m_DiscardOldest = OpcUa_False;
		OpcUa_String_Initialize(&(pMonitoredItemToCreate[i].m_IndexRange));
		pMonitoredItemToCreate[i].m_MonitoringMode = OpcUa_MonitoringMode_Reporting;
		pMonitoredItemToCreate[i].m_QueueSize = 0;
		pMonitoredItemToCreate[i].m_SamplingInterval = 0;
		pMonitoredItemToCreate[i].m_TimestampsToReturn = OpcUa_TimestampsToReturn_Both;

		i++;
		OpcUa_NodeId_Initialize(&aNodeId);
		aNodeId.IdentifierType = OpcUa_IdentifierType_String;
		aNodeId.NamespaceIndex = 2;
		OpcUa_String_AttachCopy(&aNodeId.Identifier.String, "10#MODE");

		OpcUa_NodeId_CopyTo(&aNodeId, &(pMonitoredItemToCreate[i].m_NodeId));
		pMonitoredItemToCreate[i].m_AttributeId = OpcUa_Attributes_Value;
		pMonitoredItemToCreate[i].m_ClientHandle = i; // Will be handled by the library so no need to initialize it
		pMonitoredItemToCreate[i].m_DatachangeTrigger = OpcUa_DataChangeTrigger_StatusValueTimestamp;
		pMonitoredItemToCreate[i].m_DeadbandType = 0; // 0 for None, 1 for Absolute, 2 for Percent
		pMonitoredItemToCreate[i].m_DeadbandValue = 0;
		pMonitoredItemToCreate[i].m_DiscardOldest = OpcUa_False;
		OpcUa_String_Initialize(&(pMonitoredItemToCreate[i].m_IndexRange));
		pMonitoredItemToCreate[i].m_MonitoringMode = OpcUa_MonitoringMode_Reporting;
		pMonitoredItemToCreate[i].m_QueueSize = 0;
		pMonitoredItemToCreate[i].m_SamplingInterval = 0;
		pMonitoredItemToCreate[i].m_TimestampsToReturn = OpcUa_TimestampsToReturn_Both;


		i++;
		OpcUa_NodeId_Initialize(&aNodeId);
		aNodeId.IdentifierType = OpcUa_IdentifierType_String;
		aNodeId.NamespaceIndex = 2;
		OpcUa_String_AttachCopy(&aNodeId.Identifier.String, "10@10000.0");

		OpcUa_NodeId_CopyTo(&aNodeId, &(pMonitoredItemToCreate[i].m_NodeId));
		pMonitoredItemToCreate[i].m_AttributeId = OpcUa_Attributes_Value;
		pMonitoredItemToCreate[i].m_ClientHandle = i; // Will be handled by the library so no need to initialize it
		pMonitoredItemToCreate[i].m_DatachangeTrigger = OpcUa_DataChangeTrigger_StatusValueTimestamp;
		pMonitoredItemToCreate[i].m_DeadbandType = 0; // 0 for None, 1 for Absolute, 2 for Percent
		pMonitoredItemToCreate[i].m_DeadbandValue = 0;
		pMonitoredItemToCreate[i].m_DiscardOldest = OpcUa_False;
		OpcUa_String_Initialize(&(pMonitoredItemToCreate[i].m_IndexRange));
		pMonitoredItemToCreate[i].m_MonitoringMode = OpcUa_MonitoringMode_Reporting;
		pMonitoredItemToCreate[i].m_QueueSize = 0;
		pMonitoredItemToCreate[i].m_SamplingInterval = 0;
		pMonitoredItemToCreate[i].m_TimestampsToReturn = OpcUa_TimestampsToReturn_Both;


		i++;
		OpcUa_NodeId_Initialize(&aNodeId);
		aNodeId.IdentifierType = OpcUa_IdentifierType_String;
		aNodeId.NamespaceIndex = 2;
		OpcUa_String_AttachCopy(&aNodeId.Identifier.String, "10:15266");

		OpcUa_NodeId_CopyTo(&aNodeId, &(pMonitoredItemToCreate[i].m_NodeId));
		pMonitoredItemToCreate[i].m_AttributeId = OpcUa_Attributes_Value;
		pMonitoredItemToCreate[i].m_ClientHandle = i; // Will be handled by the library so no need to initialize it
		pMonitoredItemToCreate[i].m_DatachangeTrigger = OpcUa_DataChangeTrigger_StatusValueTimestamp;
		pMonitoredItemToCreate[i].m_DeadbandType = 0; // 0 for None, 1 for Absolute, 2 for Percent
		pMonitoredItemToCreate[i].m_DeadbandValue = 0;
		pMonitoredItemToCreate[i].m_DiscardOldest = OpcUa_False;
		OpcUa_String_Initialize(&(pMonitoredItemToCreate[i].m_IndexRange));
		pMonitoredItemToCreate[i].m_MonitoringMode = OpcUa_MonitoringMode_Reporting;
		pMonitoredItemToCreate[i].m_QueueSize = 0;
		pMonitoredItemToCreate[i].m_SamplingInterval = 0;
		pMonitoredItemToCreate[i].m_TimestampsToReturn = OpcUa_TimestampsToReturn_Both;

		i++;
		OpcUa_NodeId_Initialize(&aNodeId);
		aNodeId.IdentifierType = OpcUa_IdentifierType_String;
		aNodeId.NamespaceIndex = 2;
		OpcUa_String_AttachCopy(&aNodeId.Identifier.String, "10:15131");

		OpcUa_NodeId_CopyTo(&aNodeId, &(pMonitoredItemToCreate[i].m_NodeId));
		pMonitoredItemToCreate[i].m_AttributeId = OpcUa_Attributes_Value;
		pMonitoredItemToCreate[i].m_ClientHandle = i; // Will be handled by the library so no need to initialize it
		pMonitoredItemToCreate[i].m_DatachangeTrigger = OpcUa_DataChangeTrigger_StatusValueTimestamp;
		pMonitoredItemToCreate[i].m_DeadbandType = 0; // 0 for None, 1 for Absolute, 2 for Percent
		pMonitoredItemToCreate[i].m_DeadbandValue = 0;
		pMonitoredItemToCreate[i].m_DiscardOldest = OpcUa_False;
		OpcUa_String_Initialize(&(pMonitoredItemToCreate[i].m_IndexRange));
		pMonitoredItemToCreate[i].m_MonitoringMode = OpcUa_MonitoringMode_Reporting;
		pMonitoredItemToCreate[i].m_QueueSize = 0;
		pMonitoredItemToCreate[i].m_SamplingInterval = 0;
		pMonitoredItemToCreate[i].m_TimestampsToReturn = OpcUa_TimestampsToReturn_Both;

		iNoOfItemsToCreate = i;
		uStatus = OpenOpcUa_CreateMonitoredItemsEx(
			m_hApplication,
			m_hSession,
			m_hSubscription,
			iNoOfItemsToCreate,
			pMonitoredItemToCreate,
			&pMonitoredItemCreated);
		if (uStatus == OpcUa_Good)
		{
			printf("OpenOpcUa_CreateMonitoredItemsEx succeed\n");
			for (OpcUa_Int32 i = 0; i < iNoOfItemsToCreate; i++)
			{
				printf("Item[%u] handle=0x%05x uStatus=0x%05x\n", i, pMonitoredItemCreated[i].m_hMonitoredItem, pMonitoredItemCreated[i].m_Result);
			}
		}
	}
	else
		uStatus = OpcUa_BadOutOfMemory;
	return uStatus;
}
OpcUa_StatusCode AddMultipleMonitoredItem()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_UInt32 iNoOfItemsToCreate = 5;
	OpcUa_MonitoredItemToCreate* pMonitoredItemToCreate = (OpcUa_MonitoredItemToCreate*)OpcUa_Alloc(sizeof(OpcUa_MonitoredItemToCreate)*iNoOfItemsToCreate);

	memset(pMonitoredItemToCreate, 0, sizeof(OpcUa_MonitoredItemToCreate)*iNoOfItemsToCreate);
	if (pMonitoredItemToCreate)
	{
		OpcUa_MonitoredItemCreated* pMonitoredItemCreated = OpcUa_Null;
		for (OpcUa_Int32 i = 0; i < iNoOfItemsToCreate; i++)
		{
			OpcUa_NodeId aNodeId;
			OpcUa_NodeId_Initialize(&aNodeId);
			aNodeId.Identifier.Numeric = 4 + i;
			aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
			aNodeId.NamespaceIndex = 2;
			OpcUa_NodeId_CopyTo(&aNodeId, &(pMonitoredItemToCreate[i].m_NodeId));
			pMonitoredItemToCreate[i].m_AttributeId = OpcUa_Attributes_Value;
			pMonitoredItemToCreate[i].m_ClientHandle = 1; // Will be handled by the library so no need to initialize it
			pMonitoredItemToCreate[i].m_DatachangeTrigger = OpcUa_DataChangeTrigger_StatusValueTimestamp;
			pMonitoredItemToCreate[i].m_DeadbandType = 0; // 0 for None, 1 for Absolute, 2 for Percent
			pMonitoredItemToCreate[i].m_DeadbandValue = 0;
			pMonitoredItemToCreate[i].m_DiscardOldest = OpcUa_False;
			OpcUa_String_Initialize(&(pMonitoredItemToCreate[i].m_IndexRange));
			pMonitoredItemToCreate[i].m_MonitoringMode = OpcUa_MonitoringMode_Reporting;
			pMonitoredItemToCreate[i].m_QueueSize = 0;
			pMonitoredItemToCreate[i].m_SamplingInterval = 0;
			pMonitoredItemToCreate[i].m_TimestampsToReturn = OpcUa_TimestampsToReturn_Both;
		}
		uStatus = OpenOpcUa_CreateMonitoredItemsEx(
			m_hApplication,
			m_hSession,
			m_hSubscription,
			iNoOfItemsToCreate,
			pMonitoredItemToCreate,
			&pMonitoredItemCreated);
		if (uStatus == OpcUa_Good)
		{
			printf("OpenOpcUa_CreateMonitoredItemsEx succeed\n");
			for (OpcUa_Int32 i = 0; i < iNoOfItemsToCreate; i++)
			{
				printf("Item[%u] handle=0x%05x uStatus=0x%05x\n", i, pMonitoredItemCreated[i].m_hMonitoredItem, pMonitoredItemCreated[i].m_Result);
			}
		}
	}
	else
		uStatus = OpcUa_BadOutOfMemory;
	return uStatus;
}
/*OpcUa_StatusCode __stdcall OnShutdownMessage(OpcUa_Handle hApplication, OpcUa_Handle hSession,
	OpcUa_String strShutdownMessage,
	void* extraParam)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	char Message[1024];
	memset(Message, 0, 1024);
	OpcUa_StatusCode uClientLibStatus = (OpcUa_StatusCode)(extraParam);
	sprintf(Message, "%s 0x%05x\n", 
		OpcUa_String_GetRawString(&strShutdownMessage), 
		(unsigned int)uClientLibStatus);
	printf("%s\n", Message);
	return uStatus;
}*/

int main(int argc, char* argv[])
{
	OpcUa_Int32 i = 0;
	printf("Welcome in the OpenOpcUaTerminalClient\n");
	OpcUa_StatusCode uStatus = OpenOpcUa_InitializeAbstractionLayer((OpcUa_CharA*)"OpenOpcUaTerminalClient", &m_hApplication);
	gWriteValue.Datatype = OpcUaType_UInt16;
	gWriteValue.Value.Int16 = 0;
	if (uStatus == OpcUa_Good)
	{
		OpcUa_String szCertificateStore;
		OpcUa_String_Initialize(&szCertificateStore);
		OpcUa_String_AttachCopy(&szCertificateStore, (OpcUa_CharA*)"CertificateStore");
		uStatus = OpenOpcUa_InitializeSecurity(m_hApplication, szCertificateStore);
		if (uStatus == OpcUa_Good)
		{
			printf("**************************************\n");
			printf("Will create session : %d\n", i);
			OpenOpcUa_Trace(m_hApplication, OPCUA_TRACE_CLIENT_ERROR, "Will create session : %u\n", i);
			OpcUa_String szParam1;
			OpcUa_String_Initialize(&szParam1);
			if (argv[1])
			{
				OpcUa_String_AttachCopy(&szParam1, argv[1]);
				uStatus = CreateSessionOpc("opc.tcp://GHOST.4CE-INDUSTRY.LAN:16664/OpenOpcUaCoreServer", OpcUa_String_GetRawString(&szParam1));
			}
			else
				uStatus = CreateSessionOpc("opc.tcp://localhost:21381/MatrikonOpcUaWrapper","http://opcfoundation.org/UA/SecurityPolicy#None");//opc.tcp://localhost:4842
				//uStatus = CreateSessionOpc("opc.tcp://127.0.0.1:16664/OpenOpcUaCoreServer");
				//
				//uStatus = CreateSessionOpc("opc.tcp://mars:51210/UA/SampleServer");
				// //opc.tcp://GHOST.4CE-INDUSTRY.LAN:16664/OpenOpcUaCoreServer
			if (uStatus == OpcUa_Good)
			{
				char* pVal = (char*)malloc(5);
				memset(pVal,0, 5);
				//OpenOpcUa_SetShutdownCallback(m_hApplication, m_hSession, (PFUNCSHUTDOWN)OnShutdownMessage, pVal);
				i = 0;
				OpcUa_NodeId aNodeId;
				OpcUa_NodeId_Initialize(&aNodeId);
				///////////////////////////////////////////////////////////////
				aNodeId.IdentifierType = OpcUa_IdentifierType_String;
				aNodeId.NamespaceIndex = 2;
				OpcUa_String_AttachCopy(&aNodeId.Identifier.String, "0:Bucket Brigade.Int2");
				/////////////////////////////////////////////////////////////////
				//aNodeId.NamespaceIndex = 0;
				//aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
				//aNodeId.Identifier.Numeric = 2258;
				while (i < 10)
				{
					if (ReadOpcValue(aNodeId) == OpcUa_Good)
						OpcUa_Thread_Sleep(1);
					else
						printf("ReadOpcValue failed\n");
					i++;
				}
				/*OpcUa_NodeId_Clear(&aNodeId);
				// Now let's prepare the subscription
				OpcUa_NodeId_Initialize(&aNodeId);
				aNodeId.NamespaceIndex = 2;
				aNodeId.Identifier.Numeric = 16;*/
				/*if (createSubscription() != OpcUa_Good) // monitor ns=2;i=16
					printf("createSubscription failed\n");
				else
				{
					//uStatus=AddMultipleMonitoredItem();
					uStatus=AddMultipleMonitoredItemString();
					//uStatus=CreateMonitoredItem(aNodeId, OpcUa_Attributes_Value);
					if (uStatus!=OpcUa_Good)
						printf("CreateMonitoredItem failed 0x%05x\n", uStatus);
					else
						printf("CreateMonitoredItem succeed\n");
				}*/
				OpcUa_NodeId_Clear(&aNodeId);
				getQChar();
				OpenOpcUa_CloseSession(m_hApplication, m_hSession);
			}
			else
			{
				printf("CreateSession failed\n");
				getQChar();
			}
			OpcUa_String_Clear(&szParam1);
		}
		OpcUa_String_Clear(&szCertificateStore);
		OpenOpcUa_ClearAbstractionLayer(m_hApplication);
	}
	return 0;
}
OpcUa_StatusCode WriteOpcValue()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_WriteValue* pWriteValue = OpcUa_Null;
	OpcUa_NodeId aNodeId;
	OpcUa_NodeId_Initialize(&aNodeId);
	aNodeId.NamespaceIndex = 2;
	aNodeId.Identifier.Numeric = 16;
	// //////////////////////////////////////////////

	// Let's write to ns=2;i=16
	OpcUa_StatusCode* pWriteResults = OpcUa_Null;
	OpcUa_Int32 iNoOfNodesToWrite = 1;
	pWriteValue = (OpcUa_WriteValue*)OpcUa_Alloc(sizeof(OpcUa_WriteValue));
	OpcUa_WriteValue_Initialize(pWriteValue);
	OpcUa_NodeId_CopyTo(&aNodeId, &(pWriteValue->NodeId));
	pWriteValue->AttributeId = OpcUa_Attributes_Value;
	OpcUa_Variant* pVariant = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
	OpcUa_Variant_Initialize(pVariant);
	pVariant->Datatype = OpcUaType_Int16;
	pVariant->Value.Int16 = 15;
	if (gLoopValue.Datatype != 0)
	{
		gLoopValue.Value.UInt16++;
		OpcUa_Variant_CopyTo(&gLoopValue, &(pWriteValue->Value.Value));
	}
	else
		OpcUa_Variant_CopyTo(pVariant, &(pWriteValue->Value.Value));
	uStatus = OpenOpcUa_WriteAttributes(m_hApplication, m_hSession, iNoOfNodesToWrite, pWriteValue, &pWriteResults);
	if (pWriteResults)
		OpcUa_Free(pWriteResults);
	OpcUa_Variant_Clear(pVariant);
	OpcUa_Free(pVariant);
	OpcUa_WriteValue_Clear(pWriteValue);
	OpcUa_Free(pWriteValue);
	return uStatus;
}
OpcUa_StatusCode WriteMultipleOpcValue()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_WriteValue* pWriteValue = OpcUa_Null;
	OpcUa_StatusCode* pWriteResults = OpcUa_Null;
	OpcUa_Int32 iNoOfNodesToWrite = 1;
	pWriteValue = (OpcUa_WriteValue*)OpcUa_Alloc(sizeof(OpcUa_WriteValue)*iNoOfNodesToWrite);

	// default Value to write
	OpcUa_Variant* pVariant = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
	OpcUa_Variant_Initialize(pVariant);
	pVariant->Datatype = OpcUaType_UInt16;
	pVariant->Value.UInt16 = 15;
	gWriteValue.Value.UInt16++;
	/*
	// //////////////////////////////////////////////
	OpcUa_NodeId aNodeId1;
	OpcUa_NodeId_Initialize(&aNodeId1);
	aNodeId1.IdentifierType = OpcUa_IdentifierType_String;
	aNodeId1.NamespaceIndex = 2;
	OpcUa_String_AttachCopy(&aNodeId1.Identifier.String, "10#ETAT");
	OpcUa_WriteValue_Initialize(&pWriteValue[0]);
	OpcUa_NodeId_CopyTo(&aNodeId1, &(pWriteValue[0].NodeId));
	pWriteValue[0].AttributeId = OpcUa_Attributes_Value;
	OpcUa_Variant_CopyTo(&gWriteValue, &(pWriteValue[0].Value.Value));
	////////////////////////////////////////////////////////////////
	OpcUa_NodeId aNodeId2;
	OpcUa_NodeId_Initialize(&aNodeId2);
	aNodeId2.IdentifierType = OpcUa_IdentifierType_String;
	aNodeId2.NamespaceIndex = 2;
	OpcUa_String_AttachCopy(&aNodeId2.Identifier.String, "10#MODE");
	OpcUa_WriteValue_Initialize(&pWriteValue[1]);
	OpcUa_NodeId_CopyTo(&aNodeId2, &(pWriteValue[1].NodeId));
	pWriteValue[1].AttributeId = OpcUa_Attributes_Value;
	OpcUa_Variant_CopyTo(&gWriteValue, &(pWriteValue[1].Value.Value));
	//////////////////////////////////////////////////////////////////
	OpcUa_NodeId aNodeId3;
	OpcUa_NodeId_Initialize(&aNodeId3);
	aNodeId3.IdentifierType = OpcUa_IdentifierType_String;
	aNodeId3.NamespaceIndex = 2;
	OpcUa_String_AttachCopy(&aNodeId3.Identifier.String, "10@10000.0");
	OpcUa_WriteValue_Initialize(&pWriteValue[2]);
	OpcUa_NodeId_CopyTo(&aNodeId3, &(pWriteValue[2].NodeId));
	pWriteValue[2].AttributeId = OpcUa_Attributes_Value;
	OpcUa_Variant_CopyTo(&gWriteValue, &(pWriteValue[2].Value.Value));
	/////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	OpcUa_NodeId aNodeId4;
	OpcUa_NodeId_Initialize(&aNodeId4);
	aNodeId4.IdentifierType = OpcUa_IdentifierType_String;
	aNodeId4.NamespaceIndex = 2;
	OpcUa_String_AttachCopy(&aNodeId4.Identifier.String, "10:15266");
	OpcUa_WriteValue_Initialize(&pWriteValue[3]);
	OpcUa_NodeId_CopyTo(&aNodeId4, &(pWriteValue[3].NodeId));
	pWriteValue[3].AttributeId = OpcUa_Attributes_Value;
	OpcUa_Variant_CopyTo(&gWriteValue, &(pWriteValue[3].Value.Value));
	/////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////
	OpcUa_NodeId aNodeId5;
	OpcUa_NodeId_Initialize(&aNodeId5);
	aNodeId5.IdentifierType = OpcUa_IdentifierType_String;
	aNodeId5.NamespaceIndex = 2;
	OpcUa_String_AttachCopy(&aNodeId5.Identifier.String, "10:15131");
	OpcUa_WriteValue_Initialize(&pWriteValue[4]);
	OpcUa_NodeId_CopyTo(&aNodeId5, &(pWriteValue[4].NodeId));
	pWriteValue[4].AttributeId = OpcUa_Attributes_Value;
	OpcUa_Variant_CopyTo(&gWriteValue, &(pWriteValue[4].Value.Value));
	/////////////////////////////////////////////////////////////////////
	*/
	OpcUa_NodeId aNodeId5;
	OpcUa_NodeId_Initialize(&aNodeId5);
	aNodeId5.IdentifierType = OpcUa_IdentifierType_Numeric;
	aNodeId5.NamespaceIndex = 2;
	aNodeId5.Identifier.Numeric = 16;
	OpcUa_WriteValue_Initialize(&pWriteValue[0]);
	OpcUa_NodeId_CopyTo(&aNodeId5, &(pWriteValue[0].NodeId));
	pWriteValue[0].AttributeId = OpcUa_Attributes_Value;
	OpcUa_Variant_CopyTo(&gWriteValue, &(pWriteValue[0].Value.Value));
	uStatus = OpenOpcUa_WriteAttributes(m_hApplication, m_hSession, iNoOfNodesToWrite, pWriteValue, &pWriteResults);
	if (pWriteResults)
		OpcUa_Free(pWriteResults);
	OpcUa_Variant_Clear(pVariant);
	OpcUa_Free(pVariant);
	for (OpcUa_UInt32 i = 0; i < iNoOfNodesToWrite; i++)
		OpcUa_WriteValue_Clear(&pWriteValue[i]);
	OpcUa_Free(pWriteValue);
	return uStatus;
}
/// <summary>
/// Creates the session opc.
/// </summary>
/// <param name="url">The URL.</param>
/// <param name="szSecurity">None, Basic128Rsa15, Basic256.</param>
/// <returns></returns>
OpcUa_StatusCode CreateSessionOpc(char* url,char* szSecurity)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidState;

	OpcUa_EndpointDescription* pEndpointDescription = OpcUa_Null;
	OpcUa_UInt32 uiNbOfEndpointDescription = 0;
	OpcUa_String* pString = OpcUa_Null;
	pString = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pString);

	OpcUa_String_AttachCopy(pString, url);
	uStatus = OpenOpcUa_GetEndpoints(m_hApplication,
		pString,
		&uiNbOfEndpointDescription,
		&pEndpointDescription);

	if (uStatus == OpcUa_Good)
	{
		OpcUa_String aSessionName;
		OpcUa_String_Initialize(&aSessionName);
		OpcUa_String_AttachCopy(&aSessionName, (OpcUa_CharA*)"OpenOpcUaTerminalClient");

		OpcUa_String strSecurity;
		OpcUa_String_Initialize(&strSecurity);
		OpcUa_String_AttachCopy(&strSecurity, (OpcUa_CharA*)szSecurity);
		for (OpcUa_UInt32 i = 0; i < uiNbOfEndpointDescription; i++)
		{
			if (OpcUa_String_Compare(&(pEndpointDescription[i].SecurityPolicyUri), &strSecurity) == 0)
			{
				uStatus = OpenOpcUa_CreateSession(m_hApplication, &pEndpointDescription[i], 600000, aSessionName, &m_hSession);
				if (uStatus == OpcUa_Good)
				{
					printf("Connected\n");
					//uStatus = OpenOpcUa_ActivateSession(m_hApplication, m_hSession);
					uStatus = OpenOpcUa_ActivateSession(m_hApplication, m_hSession,pEndpointDescription,"anonymous","","");
					
					printf("Session activated\n");
				}
				break;
			}
		}
		OpcUa_String_Clear(&strSecurity);
		OpcUa_String_Clear(&aSessionName);
		if (pEndpointDescription)
		{
			for (OpcUa_UInt32 i = 0; i < uiNbOfEndpointDescription; i++)
				OpcUa_EndpointDescription_Clear(&pEndpointDescription[i]);
			OpcUa_Free(pEndpointDescription);
		}
	}
	if (pString)
	{
		OpcUa_String_Clear(pString);
		OpcUa_Free(pString);
		pString = OpcUa_Null;
	}
	return uStatus;
}
OpcUa_StatusCode ReadOpcValue(OpcUa_NodeId nodeId)
{
	OpcUa_String *valeur = NULL;
	OpcUa_StatusCode uStatus = OpcUa_Good;


	OpcUa_DataValue* pResults = OpcUa_Null;
	OpcUa_ReadValueId* pNodesToRead = OpcUa_Null;

	pNodesToRead = (OpcUa_ReadValueId*)OpcUa_Alloc(sizeof(OpcUa_ReadValueId));
	if (pNodesToRead)
	{
		OpcUa_ReadValueId_Initialize(pNodesToRead);
		pNodesToRead->AttributeId = OpcUa_Attributes_Value;
		OpcUa_NodeId_CopyTo(&nodeId, &(pNodesToRead->NodeId));

		uStatus = OpenOpcUa_ReadAttributes(m_hApplication, m_hSession,
			OpcUa_TimestampsToReturn_Both,
			1,
			pNodesToRead, &pResults);
		if (uStatus == OpcUa_Good)
		{
			uStatus = OpenOpcUa_VariantToString(pResults[0].Value, &valeur);
			printf("%s\n", OpcUa_String_GetRawString(valeur));
			if (valeur)
			{
				OpcUa_String_Clear(valeur);
				OpcUa_Free(valeur);
			}
			if (pResults)
			{
				OpcUa_DataValue_Clear(pResults);
				OpcUa_Free(pResults);
			}
		}
		else
			printf("OpenOpcUa_ReadAttributes failed : uStatus=0x%05x\n", (unsigned int)uStatus);
		OpcUa_NodeId_Clear(&(pNodesToRead->NodeId));
		OpcUa_ReadValueId_Clear(pNodesToRead);
		OpcUa_Free(pNodesToRead);
	}
	else
		uStatus = OpcUa_Bad;
	return uStatus;
}
