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
#define _HAS_ITERATOR_DEBUGGING 0
//#define LEAK_SEEKER 1
#include <stdio.h>


#ifdef WIN32
// For the windows service version of the server
//#include <atlbase.h> 
//#include <winsvc.h>
// End for the windows service version of the server
#include <winsock2.h>
#include "targetver.h"
#include <windows.h>

#include <tchar.h>
#include <shlobj.h>
#endif 
// stl header
#include <string>
#include <vector>
#include <map>
#include <list>
#include <queue>

#define	SHAREDLIB_EXPORT
#define OPC_UASERVER

#include <opcua_p_os.h>
#include <opcua.h>
#include <opcua_types.h>
#include <opcua_serverstub.h>
#include <opcua_clientproxy.h>
#include <opcua_memory.h>
#include <opcua_core.h>
#include <opcua_mutex.h>
#include <opcua_trace.h>
#include <opcua_string.h>
#include <opcua_timer.h>
#ifdef _GNUC_
#include <openssl/ssl.h>
#endif
#include <opcua_certificates.h>
#include "opcua_p_semaphore.h"

#include "cslock.h"
typedef std::vector<OpcUa_UInt32> CUInt32Collection;

#include "OpenOpcUa.h"
#include "Utils.h"
#include "NumericRange.h"
#include "EndpointDescription.h"
#include "Application.h"
#include "Channel.h"
#include "SessionBase.h"
#include "SubscriptionDiagnosticsDataType.h"
#include "Subscription.h"
#include "MonitoredItemBase.h"

// Support for the XML SAX 
#include "xmlsaxparsertlk.h"

//
#include "UAReference.h"
#include "UABase.h"
#include "DataValue.h"
//
#include "VpiDataValue.h"
#include "SecureChannel.h"

typedef enum NOTIFICATION_MESSAGE_TYPE
{
	NOTIFICATION_MESSAGE_KEEPALIVE,// NOTIFICATION_MESSAGE_KEEPALIVE
	NOTIFICATION_MESSAGE_DATACHANGE,// NOTIFICATION_MESSAGE_DATACHANGE
	NOTIFICATION_MESSAGE_UNKNOW, // NOTIFICATION_MESSAGE_UNKNOW
	NOTIFICATION_MESSAGE_EVENT // Notification of an Event
} NotificationMessageType;
#include "UAStatusChangeNotification.h"
#include "QueueRequest.h"

struct LessNodeId
{
	bool operator()(const OpcUa_NodeId* p1, const OpcUa_NodeId* p2) const
	{
		return OpcUa_NodeId_Compare(p1, p2) < 0;
	}
};
//#include "UAVariable.h"
//#include "Field.h"
//#include "Definition.h"
using namespace OpenOpcUa;
using namespace UAAddressSpace;
//
#include "FileVersionInfo.h"
#include "opcua_trace.h"
//
#define DEFAULT_SAMPLING_INTERVAL 250
#define DEFAULT_NOTIFICATION_INTERVAL 1000
#define MAX_ASYNC_THREAD_INTERVAL 100 // the asyncThread cannot run slower that MAX_ASYNC_THREAD_INTERVAL ms
#define MAX_SESSION_SUPPORTED 500
#define MAX_SUBSCRIPTION_PER_SESSION 50
#define MAX_PUBLISH_PER_SESSION 50
#define MONITOREDITEM_MAX_QUEUESIZE 10
#define MONITOREDITEM_DEFAULT_QUEUESIZE 2
#define PUBLISHING_INTERVAL_MINI 50 // WARNING need to changed for  the CTT
#define PUBLISHING_INTERVAL_MAX 2592000000.00  // 30 days
#define MAX_KEEPALIVE_COUNT_MINI 5
#define MAX_KEEPALIVE_COUNT_MAX 30000
#define OPC_TIMEOUT 7500
#define	PI		3.1415926
#define OPENOPCUA_MAX_BROWSE_CP 1 // NOMBRE MAXIMUM DE ContinuationPoint supporté par ce serveur
//
int getQChar();
#ifdef WIN32
OpcUa_StatusCode AddLoggerMessage(wchar_t* szMessage, DWORD dwLevel);
#endif
OpcUa_StatusCode CreateNodeId(OpcUa_UInt16 IdentifierType, OpcUa_UInt16 namespaceIndex, OpcUa_UInt32 InitialValue, OpcUa_NodeId* pNodeId);
OpcUa_StatusCode ParseNodeId(const char* atts,OpcUa_NodeId* pNodeId);
OpcUa_StatusCode IsNodeIdValid(const OpcUa_NodeId  aNodeId);
//OpcUa_StatusCode FindTypeDefinition(OpcUa_NodeId aNodeId, CDefinition** pDefinition);
OpcUa_StatusCode FindBuiltinType(OpcUa_NodeId aNodeId, OpcUa_Byte* pBuiltInType);
// Pre-evalue le StatusCode a retourner lors de demande de lecture
OpcUa_StatusCode PreCheckedStatusCode(OpcUa_ReadValueId* pNodeToRead, OpenOpcUa::UAAddressSpace::CUABase* pUABase);
// Internal helper function
// Generic ExtensionObject copy function. It copy according to the definition in the NodeSet file
OpcUa_StatusCode Copy(OpcUa_ExtensionObject** pTargetExtensionObject,OpcUa_ExtensionObject* pSourceExtensionObject);
// Copy builtInType from a void* to another one
OpcUa_StatusCode CopyBuiltInType(const OpcUa_Int32 ifieldSize, const OpcUa_Byte builtInType, void** pVoidBufSource, void** pVoidBufTarget);
// Extract the OpcData_Value from pDataValue1 and send in the pDataValue2 based on the pRange 
OpcUa_StatusCode AdjustArrayToRange(OpcUa_DataValue* pDataValue1, CNumericRange* pRange, OpcUa_DataValue** pDataValue2);
//
OpcUa_StatusCode FindConsistentInverseNodeId(OpcUa_NodeId aReferenceNodeId,OpcUa_NodeId* pConsistentInverseNodeId);
OpcUa_StatusCode LoadNodeSetFiles();
OpcUa_StatusCode LoadSimulationFiles();
OpcUa_StatusCode LoadSubSystemFiles();
OpcUa_StatusCode SetServerDescriptionTransportPortListener(CApplicationDescription** pAppDescription);
// Ensemble des traitements réalisés sur l'espace d'adressaeg après que les fichiers XML ai été chargés
OpcUa_StatusCode PostProcessing();

#ifdef _GNUC_
#include <cmath>
OpcUa_StatusCode AddLoggerMessage(char* szMessage, DWORD dwLevel);
#define swprintf_s swprintf
#define sscanf_s   sscanf
#endif
