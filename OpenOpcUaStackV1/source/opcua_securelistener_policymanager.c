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

/* core */
#include <opcua.h>

#ifdef OPCUA_HAVE_SERVERAPI

#include <opcua_types.h>
#include <opcua_datetime.h>
#include <opcua_guid.h>
#include <opcua_list.h>

/* stackcore */
#include <opcua_securechannel.h>

/* security */
#include <opcua_tcpsecurechannel.h>
#include <opcua_securelistener.h>
#include <opcua_securelistener_policymanager.h>

/* TODO: Consider this: */
#define OPCUA_SECURELISTENER_POLICYMANAGER_ISVALIDMESSAGESECURITYMODE(xPolicyManager, xMsgSecMode) \
    (!(xPolicyManager->uMessageSecurityModes & xMsgSecMode))

/*==============================================================================*/
/* OpcUa_SecureListener_PolicyManager_Create                                    */
/*==============================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_PolicyManager_Create(
    OpcUa_SecureListener_PolicyManager** a_ppPolicyManager)
{
    OpcUa_SecureListener_PolicyManager* pPolicyMngr = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "PolicyManager_Create");
    
    OpcUa_ReturnErrorIfArgumentNull(a_ppPolicyManager);

    *a_ppPolicyManager = OpcUa_Null;

    pPolicyMngr = (OpcUa_SecureListener_PolicyManager*) OpcUa_Alloc(sizeof(OpcUa_SecureListener_PolicyManager));
    OpcUa_ReturnErrorIfAllocFailed(pPolicyMngr);

    uStatus = OpcUa_SecureListener_PolicyManager_Initialize(pPolicyMngr);
    OpcUa_GotoErrorIfBad(uStatus);

    if(pPolicyMngr->SecurityPolicies == OpcUa_Null)
    {
        OpcUa_SecureListener_PolicyManager_Delete(&pPolicyMngr);
    }

    *a_ppPolicyManager = pPolicyMngr;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_Free(pPolicyMngr);
    pPolicyMngr = OpcUa_Null;

OpcUa_FinishErrorHandling;
}

/*==============================================================================*/
/* OpcUa_SecureListener_PolicyManager_Initialize                                */
/*==============================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_PolicyManager_Initialize(
    OpcUa_SecureListener_PolicyManager* a_pPolicyManager)
{
OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "PolicyManager_Initialize");
    
    if (a_pPolicyManager == OpcUa_Null)
    {
        uStatus = OpcUa_BadInvalidArgument;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    uStatus = OpcUa_List_Create(&(a_pPolicyManager->SecurityPolicies));
    OpcUa_ReturnErrorIfBad(uStatus);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*==============================================================================*/
/* OpcUa_SecureListener_PolicyManager_ClearAll                                  */
/*==============================================================================*/
OpcUa_Void OpcUa_SecureListener_PolicyManager_ClearAll(
    OpcUa_SecureListener_PolicyManager* a_pPolicyManager)
{
    /* OpcUa_UInt32    uModule = OpcUa_Module_SecureListener; */
    /* OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "SecureListener - PolicyManager_ClearAll\n"); */
    OpcUa_SecureListener_PolicyManager_ClearSecurityPolicyConfigurations(a_pPolicyManager);
}

/*==============================================================================*/
/* OpcUa_SecureListener_SecurityPolicyConfiguration_Delete                      */
/*==============================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_SecurityPolicyConfiguration_Delete(OpcUa_SecureListener_SecurityPolicyConfiguration** a_ppPolicyConfiguration)
{
OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "OpcUa_SecureListener_SecurityPolicyConfiguration_Delete");

    OpcUa_ReturnErrorIfArgumentNull(a_ppPolicyConfiguration);
    OpcUa_ReturnErrorIfArgumentNull(*a_ppPolicyConfiguration);

    OpcUa_String_Clear(&((*a_ppPolicyConfiguration)->sSecurityPolicy));

    OpcUa_Free(*a_ppPolicyConfiguration);

    *a_ppPolicyConfiguration = OpcUa_Null;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*==============================================================================*/
/* OpcUa_SecureListener_SecurityPolicyConfiguration_Create                      */
/*==============================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_SecurityPolicyConfiguration_Create(
    OpcUa_String*                                      a_psPolicyUri,
    OpcUa_UInt16                                       a_uMessageSecurityModeMask,
    OpcUa_SecureListener_SecurityPolicyConfiguration** a_ppPolicyConfiguration)
{
    OpcUa_SecureListener_SecurityPolicyConfiguration*   pPolicyConfiguration    = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "OpcUa_SecureListener_SecurityPolicyConfiguration_Create");

    OpcUa_ReturnErrorIfArgumentNull(a_ppPolicyConfiguration);

    pPolicyConfiguration = (OpcUa_SecureListener_SecurityPolicyConfiguration*)OpcUa_Alloc(sizeof(OpcUa_SecureListener_SecurityPolicyConfiguration));
    OpcUa_ReturnErrorIfAllocFailed(pPolicyConfiguration);

    OpcUa_MemSet(pPolicyConfiguration, 0, sizeof(OpcUa_SecureListener_SecurityPolicyConfiguration));

    uStatus = OpcUa_String_StrnCpy( &pPolicyConfiguration->sSecurityPolicy,
                                    a_psPolicyUri,
                                    OPCUA_STRING_LENDONTCARE);

    pPolicyConfiguration->uMessageSecurityModes = a_uMessageSecurityModeMask;

    *a_ppPolicyConfiguration = pPolicyConfiguration;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*==============================================================================*/
/* OpcUa_SecureListener_PolicyManager_ClearSecurityPolicies                     */
/*==============================================================================*/
OpcUa_Void OpcUa_SecureListener_PolicyManager_ClearSecurityPolicyConfigurations(
    OpcUa_SecureListener_PolicyManager* a_pPolicyManager)
{
    if(a_pPolicyManager->SecurityPolicies)
    {
        OpcUa_SecureListener_SecurityPolicyConfiguration* pPolicyConfiguration = OpcUa_Null;

        OpcUa_List_Enter(a_pPolicyManager->SecurityPolicies);

        /* delete all elements */
        OpcUa_List_ResetCurrent(a_pPolicyManager->SecurityPolicies);
        pPolicyConfiguration = (OpcUa_SecureListener_SecurityPolicyConfiguration*)OpcUa_List_GetCurrentElement(a_pPolicyManager->SecurityPolicies);
        while(pPolicyConfiguration != OpcUa_Null)
        {
            OpcUa_List_DeleteCurrentElement(a_pPolicyManager->SecurityPolicies);

            OpcUa_SecureListener_SecurityPolicyConfiguration_Delete(&pPolicyConfiguration);

            pPolicyConfiguration = (OpcUa_SecureListener_SecurityPolicyConfiguration*)OpcUa_List_GetCurrentElement(a_pPolicyManager->SecurityPolicies);
        }

        OpcUa_List_Leave(a_pPolicyManager->SecurityPolicies);

        OpcUa_List_Delete(&(a_pPolicyManager->SecurityPolicies));
    }
}

/*==============================================================================*/
/* OpcUa_SecureListener_PolicyManager_Delete                                    */
/*==============================================================================*/
OpcUa_Void OpcUa_SecureListener_PolicyManager_Delete(
    OpcUa_SecureListener_PolicyManager** a_ppPolicyManager)
{
    /* OpcUa_UInt32    uModule = OpcUa_Module_SecureListener; */
    /* OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "SecureListener - PolicyManager_Delete\n"); */
    
    if (a_ppPolicyManager != OpcUa_Null)
    {
        OpcUa_SecureListener_PolicyManager_ClearAll(*a_ppPolicyManager);
        
        OpcUa_Free(*a_ppPolicyManager);
        *a_ppPolicyManager = OpcUa_Null;
    }
}

/*==============================================================================*/
/* OpcUa_SecureListener_PolicyManager_IsValidSecurityPolicy                     */
/*==============================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_PolicyManager_IsValidSecurityPolicy(
    OpcUa_SecureListener_PolicyManager* a_pPolicyManager, 
    OpcUa_String*                       a_pSecurityPolicyUri)
{
    OpcUa_SecureListener_SecurityPolicyConfiguration*   pTmpSecurityPolicyUri   = OpcUa_Null;
    OpcUa_Int32                                         cmpResult               = 1;
    
OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "PolicyManager_IsValidSecurityPolicy");

    OpcUa_List_Enter(a_pPolicyManager->SecurityPolicies);
    
    uStatus = OpcUa_List_ResetCurrent(a_pPolicyManager->SecurityPolicies);
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = OpcUa_BadSecurityPolicyRejected;

    pTmpSecurityPolicyUri = (OpcUa_SecureListener_SecurityPolicyConfiguration*)OpcUa_List_GetCurrentElement(a_pPolicyManager->SecurityPolicies);

    while(pTmpSecurityPolicyUri)
    {   
        cmpResult = OpcUa_String_StrnCmp(   &pTmpSecurityPolicyUri->sSecurityPolicy,
                                            a_pSecurityPolicyUri,
                                            OPCUA_STRING_LENDONTCARE,
                                            OpcUa_True);

        if(cmpResult == 0)
        {
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "SecureListener - PolicyManager_IsValidSecurityPolicy: Searched security policy found!\n");
            uStatus = OpcUa_Good;
            break;
        }   

        pTmpSecurityPolicyUri = (OpcUa_SecureListener_SecurityPolicyConfiguration*)OpcUa_List_GetNextElement(a_pPolicyManager->SecurityPolicies);
    }
    
    if(cmpResult != 0)
    {
        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "SecureListener - PolicyManager_IsValidSecurityPolicy: Searched security policy NOT found!\n");
    }
    
    OpcUa_List_Leave(a_pPolicyManager->SecurityPolicies);
        
OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
    OpcUa_List_Leave(a_pPolicyManager->SecurityPolicies);
OpcUa_FinishErrorHandling;
}

/*==============================================================================*/
/* OpcUa_SecureListener_PolicyManager_IsValidSecurityPolicy                     */
/*==============================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_PolicyManager_IsValidSecurityPolicyConfiguration(
    OpcUa_SecureListener_PolicyManager*                 a_pPolicyManager, 
    OpcUa_SecureListener_SecurityPolicyConfiguration*   a_pSecurityPolicyConfiguration)
{
    OpcUa_SecureListener_SecurityPolicyConfiguration*   pCurrentSecConfig   = OpcUa_Null;
    OpcUa_Int32                                         iCmpResult          = 0;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "PolicyManager_IsValidSecurityPolicyConfiguration");

    OpcUa_List_Enter(a_pPolicyManager->SecurityPolicies);
    
    uStatus = OpcUa_List_ResetCurrent(a_pPolicyManager->SecurityPolicies);
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = OpcUa_BadSecurityPolicyRejected;

    pCurrentSecConfig = (OpcUa_SecureListener_SecurityPolicyConfiguration*)OpcUa_List_GetCurrentElement(a_pPolicyManager->SecurityPolicies);
    while(pCurrentSecConfig != OpcUa_Null)
    {   
        iCmpResult = OpcUa_String_StrnCmp(  &pCurrentSecConfig->sSecurityPolicy,
                                            &a_pSecurityPolicyConfiguration->sSecurityPolicy,
                                            OPCUA_STRING_LENDONTCARE,
                                            OpcUa_True);

        if(iCmpResult == 0)
        {
            /* policy found, now compare message security modes */
            if((pCurrentSecConfig->uMessageSecurityModes & a_pSecurityPolicyConfiguration->uMessageSecurityModes) != 0 )
            {
                /* complete match */
                uStatus = OpcUa_Good;
                break;
            }
            else
            {
                /* message security mode not found */
                uStatus = OpcUa_BadSecurityModeRejected;
            }
        }

        pCurrentSecConfig = (OpcUa_SecureListener_SecurityPolicyConfiguration*)OpcUa_List_GetNextElement(a_pPolicyManager->SecurityPolicies);
    }
   
    OpcUa_List_Leave(a_pPolicyManager->SecurityPolicies);
        
OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_List_Leave(a_pPolicyManager->SecurityPolicies);

OpcUa_FinishErrorHandling;
}

/*==============================================================================*/
/* OpcUa_SecureListener_PolicyManager_AddSecurityPolicyConfiguration            */
/*==============================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_PolicyManager_AddSecurityPolicyConfiguration(
    OpcUa_SecureListener_PolicyManager*                 a_pPolicyManager,
    OpcUa_SecureListener_SecurityPolicyConfiguration*   a_pPolicyConfiguration)
{
    OpcUa_SecureListener_SecurityPolicyConfiguration* pPolicyConfiguration = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "OpcUa_SecureListener_PolicyManager_AddSecurityPolicyConfiguration");
    
    OpcUa_ReturnErrorIfArgumentNull(a_pPolicyManager);
    OpcUa_ReturnErrorIfArgumentNull(a_pPolicyManager->SecurityPolicies);
    OpcUa_ReturnErrorIfArgumentNull(a_pPolicyConfiguration);

    uStatus = OpcUa_SecureListener_SecurityPolicyConfiguration_Create(  &a_pPolicyConfiguration->sSecurityPolicy,
                                                                        a_pPolicyConfiguration->uMessageSecurityModes,
                                                                        &pPolicyConfiguration);
    OpcUa_ReturnErrorIfBad(uStatus);

    OpcUa_List_Enter(a_pPolicyManager->SecurityPolicies);

    uStatus = OpcUa_List_AddElement(    a_pPolicyManager->SecurityPolicies, 
                                        (OpcUa_Void*)pPolicyConfiguration);
    OpcUa_List_Leave(a_pPolicyManager->SecurityPolicies);

    OpcUa_GotoErrorIfBad(uStatus);
    
OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_SecureListener_SecurityPolicyConfiguration_Delete(&pPolicyConfiguration);

OpcUa_FinishErrorHandling;
}

#endif /* OPCUA_HAVE_SERVERAPI */
