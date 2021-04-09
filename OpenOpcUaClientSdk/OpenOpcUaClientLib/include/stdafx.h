/*****************************************************************************
	  Author
		�. Michel Condemine, 4CE Industry (2010-2012)
	  
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
//

#pragma once
#define _HAS_ITERATOR_DEBUGGING 0
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclure les en-t�tes Windows rarement utilis�s
#define	SHAREDLIB_EXPORT
// Fichiers d'en-t�te Windows :
#ifdef WIN32
#include <windows.h>
#endif

#include <string>
#include <vector>


#ifdef _GNUC_
#include <opcua_p_os.h>
#endif

#include <opcua.h>
#include <opcua_types.h>
#include <opcua_clientproxy.h>
#include <opcua_memory.h>
#include <opcua_core.h>
#include <opcua_trace.h>
#include <opcua_string.h>

#include "OpenOpcUa.h"
#include "Utils.h"
#include "CryptoUtils.h"
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

#define OPC_TIMEOUT 5000
#define CLIENTLIB_DEFAULT_TIMEOUT 10000
//#define SEEK_LEAKER 0
#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif