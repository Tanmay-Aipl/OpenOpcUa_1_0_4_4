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

/* core extended */
#include <opcua_datetime.h>
#include <opcua_guid.h>
#include <opcua_list.h>
#include <opcua_timer.h>
#include <opcua_mutex.h>

/* stackcore */
#include <opcua_securechannel.h>

/* security */
#include <opcua_tcpsecurechannel.h>
#include <opcua_soapsecurechannel.h>
#include <opcua_securelistener_channelmanager.h>

#define OPCUA_SECURELISTENER_CHANNELMANAGER_DIAGNOSTIC_LOCKING OPCUA_CONFIG_NO

#if OPCUA_SECURELISTENER_CHANNELMANAGER_DIAGNOSTIC_LOCKING

#define OPCUA_SECURELISTENER_CHANNELMANAGER_LOCK(xFunctionName, xHandle)            \
    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "%s: Manager lock\n", xFunctionName);     \
    OpcUa_List_Enter(xHandle);

#define OPCUA_SECURELISTENER_CHANNELMANAGER_UNLOCK(xFunctionName, xHandle)          \
    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "%s: Manager unlock\n", xFunctionName);   \
    OpcUa_List_Leave(xHandle);

#else /* OPCUA_SECURELISTENER_CHANNELMANAGER_DIAGNOSTIC_LOCKING */

#define OPCUA_SECURELISTENER_CHANNELMANAGER_LOCK(xUnused, xHandle)                  \
    OpcUa_List_Enter(xHandle);

#define OPCUA_SECURELISTENER_CHANNELMANAGER_UNLOCK(xUnused, xHandle)                \
    OpcUa_List_Leave(xHandle);

#endif /* OPCUA_SECURELISTENER_CHANNELMANAGER_DIAGNOSTIC_LOCKING */


/*==============================================================================*/
/* OpcUa_SecureListener_ChannelManager                                          */
/*==============================================================================*/
/**
* @brief Being part of a specific SecureListener, it manages the secure channel and connections.
*/
struct _OpcUa_SecureListener_ChannelManager
{
    /* @brief A list with current secure connections of type OpcUa_TcpSecureChannel. */
    OpcUa_List*                                                 SecureChannels;
    /* @brief Timer which periodically checks the secure channels for expired lifetimes. */
    OpcUa_Timer                                                 hLifeTimeWatchDog;
    /* @brief Called if a channel gets removed due timeout. */
    OpcUa_SecureListener_ChannelManager_ChannelRemovedCallback* pfCallback;
    /* @brief Cookie for the channel removed callback. */
    OpcUa_Void*                                                 pvCallbackData;
};

/*==============================================================================*/
/* OpcUa_SecureListener_ChannelManager_TimerCallback                            */
/*==============================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_SecureListener_ChannelManager_TimerCallback(   OpcUa_Void*     a_pvCallbackData,
                                                                                    OpcUa_Timer     a_hTimer,
                                                                                    OpcUa_UInt32    a_msecElapsed)
{
    OpcUa_SecureListener_ChannelManager*    pChannelManager     = (OpcUa_SecureListener_ChannelManager*)a_pvCallbackData;
    OpcUa_SecureChannel*                    pTmpSecureChannel   = OpcUa_Null;
    OpcUa_UInt32                            nToDelete           = 0;
    OpcUa_List                              TmpList;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "ChannelManager_TimerCallback");

    OpcUa_ReferenceParameter(a_hTimer);
    OpcUa_ReferenceParameter(a_msecElapsed);

    uStatus = OpcUa_List_Initialize(&TmpList);
    OpcUa_ReturnErrorIfBad(uStatus);

    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ChannelManager_TimerCallback: Checking Channels for lifetime expiration!\n");

    if(     OpcUa_Null != pChannelManager
        &&  OpcUa_Null != pChannelManager->SecureChannels)
    {
        /* remove all channels and delete list */
        OpcUa_UInt32 uiElementCount = 0;

        OPCUA_SECURELISTENER_CHANNELMANAGER_LOCK("OpcUa_SecureListener_ChannelManager_TimerCallback", pChannelManager->SecureChannels);

        OpcUa_List_GetNumberOfElements( pChannelManager->SecureChannels,
                                        &uiElementCount);

        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ChannelManager_TimerCallback: Checking %u channels!\n", uiElementCount);

        OpcUa_List_ResetCurrent(pChannelManager->SecureChannels);
        pTmpSecureChannel = (OpcUa_SecureChannel*)OpcUa_List_GetCurrentElement(pChannelManager->SecureChannels);

        /* check all channels for timeout */
        while(pTmpSecureChannel != OpcUa_Null)
        {
            /* Each SecureChannel exists until it is explicitly closed or
               until the last token has expired and the overlap period has elapsed. */

            OPCUA_SECURECHANNEL_LOCK(pTmpSecureChannel);
            if(pTmpSecureChannel->State == OpcUa_SecureChannelState_Closed && pTmpSecureChannel->uRefCount == 0)
            {
                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ChannelManager_TimerCallback: removing SecureChannel %u after it was closed!\n", pTmpSecureChannel->SecureChannelId);
                OpcUa_List_DeleteCurrentElement(pChannelManager->SecureChannels);
                OPCUA_SECURECHANNEL_UNLOCK(pTmpSecureChannel);
                OpcUa_TcpSecureChannel_Delete(&pTmpSecureChannel);
                pTmpSecureChannel = (OpcUa_SecureChannel*)OpcUa_List_GetCurrentElement(pChannelManager->SecureChannels);
            }
            else if(pTmpSecureChannel->State == OpcUa_SecureChannelState_Opened)
            {
                if(pTmpSecureChannel->uExpirationCounter != 0)
                {
                    pTmpSecureChannel->uExpirationCounter--;
                    OPCUA_SECURECHANNEL_UNLOCK(pTmpSecureChannel);
                    pTmpSecureChannel = (OpcUa_SecureChannel*)OpcUa_List_GetNextElement(pChannelManager->SecureChannels);
                }
                else if(pTmpSecureChannel->uOverlapCounter != 0)
                {
                    pTmpSecureChannel->uOverlapCounter--;
                    OPCUA_SECURECHANNEL_UNLOCK(pTmpSecureChannel);
                    pTmpSecureChannel = (OpcUa_SecureChannel*)OpcUa_List_GetNextElement(pChannelManager->SecureChannels);
                }
                else if (pTmpSecureChannel->uRefCount == 0)
                {
                    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ChannelManager_TimerCallback: removing SecureChannel %u after lifetime expired!\n", pTmpSecureChannel->SecureChannelId);

                    /* remove from channel manager and put into temp list for later notification */
                    OpcUa_List_DeleteCurrentElement(pChannelManager->SecureChannels);
                    OpcUa_List_AddElementToEnd(&TmpList, (OpcUa_Void*)pTmpSecureChannel);
                    OPCUA_SECURECHANNEL_UNLOCK(pTmpSecureChannel);
                    nToDelete++;

                    pTmpSecureChannel = (OpcUa_SecureChannel*)OpcUa_List_GetCurrentElement(pChannelManager->SecureChannels);
                }
                else
                {
                    pTmpSecureChannel->State = OpcUa_SecureChannelState_Closed;
                    OPCUA_SECURECHANNEL_UNLOCK(pTmpSecureChannel);
                    pTmpSecureChannel = (OpcUa_SecureChannel*)OpcUa_List_GetNextElement(pChannelManager->SecureChannels);
                }
            }
            else
            {
                /* inactive securechannel only use overlap counter */
                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ChannelManager_TimerCallback: inactive SecureChannel!\n");
                if(pTmpSecureChannel->uOverlapCounter != 0)
                {
                    pTmpSecureChannel->uOverlapCounter--;
                    OPCUA_SECURECHANNEL_UNLOCK(pTmpSecureChannel);
                    pTmpSecureChannel = (OpcUa_SecureChannel*)OpcUa_List_GetNextElement(pChannelManager->SecureChannels);
                }
                else if(pTmpSecureChannel->uRefCount == 0)
                {
                    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ChannelManager_TimerCallback: removing inactive SecureChannel!\n");

                    /* remove from channel manager and put into temp list for later notification */
                    OpcUa_List_DeleteCurrentElement(pChannelManager->SecureChannels);
                    OpcUa_List_AddElementToEnd(&TmpList, (OpcUa_Void*)pTmpSecureChannel);
                    OPCUA_SECURECHANNEL_UNLOCK(pTmpSecureChannel);
                    pTmpSecureChannel = OpcUa_Null;
                    nToDelete++;

                    pTmpSecureChannel = (OpcUa_SecureChannel*)OpcUa_List_GetCurrentElement(pChannelManager->SecureChannels);
                }
                else
                {
                    OPCUA_SECURECHANNEL_UNLOCK(pTmpSecureChannel);
                    pTmpSecureChannel = (OpcUa_SecureChannel*)OpcUa_List_GetNextElement(pChannelManager->SecureChannels);
                }
            }
        }

        OPCUA_SECURELISTENER_CHANNELMANAGER_UNLOCK("OpcUa_SecureListener_ChannelManager_TimerCallback", pChannelManager->SecureChannels);

        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ChannelManager_TimerCallback: Checking %u channels done!\n", uiElementCount);
    }

    /* notify application about all deleted securechannels and free their resources */
    if(nToDelete != 0)
    {
        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ChannelManager_TimerCallback: deleting %u SecureChannel!\n", nToDelete);
        OpcUa_List_ResetCurrent(&TmpList);
        pTmpSecureChannel = (OpcUa_SecureChannel*)OpcUa_List_GetCurrentElement(&TmpList);
        while(pTmpSecureChannel != OpcUa_Null)
        {
            pChannelManager->pfCallback(pTmpSecureChannel,
                                        pChannelManager->pvCallbackData);

            OpcUa_List_DeleteCurrentElement(&TmpList);
            OpcUa_TcpSecureChannel_Delete(&pTmpSecureChannel);

            pTmpSecureChannel = (OpcUa_SecureChannel*)OpcUa_List_GetCurrentElement(&TmpList);
        }
    }

    OpcUa_List_Clear(&TmpList);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*==============================================================================*/
/* OpcUa_SecureListener_ChannelManager_TimerKillCallback                        */
/*==============================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_SecureListener_ChannelManager_TimerKillCallback(   OpcUa_Void*     a_pvCallbackData,
                                                                                        OpcUa_Timer     a_hTimer,
                                                                                        OpcUa_UInt32    a_msecElapsed)
{
OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "ChannelManager_TimerKillCallback");

    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ChannelManager_TimerKillCallback: Lifetime expiration timer stopped!\n");

    OpcUa_ReferenceParameter(a_pvCallbackData);
    OpcUa_ReferenceParameter(a_hTimer);
    OpcUa_ReferenceParameter(a_msecElapsed);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*==============================================================================*/
/* OpcUa_SecureListener_ChannelManager_Create                                   */
/*==============================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_ChannelManager_Create(
    OpcUa_SecureListener_ChannelManager_ChannelRemovedCallback* a_pfChannelTimeoutCallback,
    OpcUa_Void*                                                 a_pvChannelTimeoutCallbackData,
    OpcUa_SecureListener_ChannelManager**                       a_ppChannelManager)
{
    OpcUa_SecureListener_ChannelManager* pSecureChannelMngr = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "ChannelManager_Create");

    OpcUa_ReturnErrorIfArgumentNull(a_ppChannelManager);

    *a_ppChannelManager = OpcUa_Null;

    pSecureChannelMngr = (OpcUa_SecureListener_ChannelManager*)OpcUa_Alloc(sizeof(OpcUa_SecureListener_ChannelManager));
    OpcUa_ReturnErrorIfAllocFailed(pSecureChannelMngr);

    uStatus = OpcUa_SecureListener_ChannelManager_Initialize(
        a_pfChannelTimeoutCallback,
        a_pvChannelTimeoutCallbackData,
        pSecureChannelMngr);

    OpcUa_GotoErrorIfBad(uStatus);

    if(pSecureChannelMngr->SecureChannels == OpcUa_Null)
    {
        OpcUa_SecureListener_ChannelManager_Delete(&pSecureChannelMngr);
    }

    *a_ppChannelManager = pSecureChannelMngr;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_Free(pSecureChannelMngr);
    pSecureChannelMngr = OpcUa_Null;

OpcUa_FinishErrorHandling;
}

/*==============================================================================*/
/* OpcUa_SecureListener_ChannelManager_Initialize                               */
/*==============================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_ChannelManager_Initialize(
    OpcUa_SecureListener_ChannelManager_ChannelRemovedCallback* a_pfChannelTimeoutCallback,
    OpcUa_Void*                                                 a_pvChannelTimeoutCallbackData,
    OpcUa_SecureListener_ChannelManager*                        a_pChannelManager)
{
OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "ChannelManager_Initialize");

    OpcUa_ReturnErrorIfArgumentNull(a_pChannelManager);

    uStatus = OpcUa_List_Create(&(a_pChannelManager->SecureChannels));
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = OpcUa_Timer_Create(   &(a_pChannelManager->hLifeTimeWatchDog),
                                    OPCUA_SECURELISTENER_WATCHDOG_INTERVAL,
                                    OpcUa_SecureListener_ChannelManager_TimerCallback,
                                    OpcUa_SecureListener_ChannelManager_TimerKillCallback,
                                    (OpcUa_Void*)a_pChannelManager);
    OpcUa_GotoErrorIfBad(uStatus);

    a_pChannelManager->pfCallback       = a_pfChannelTimeoutCallback;
    a_pChannelManager->pvCallbackData   = a_pvChannelTimeoutCallbackData;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_SecureListener_ChannelManager_Clear(a_pChannelManager);

OpcUa_FinishErrorHandling;
}

/*==============================================================================*/
/* OpcUa_SecureListener_ChannelManager_Clear                                    */
/*==============================================================================*/
OpcUa_Void OpcUa_SecureListener_ChannelManager_Clear(
    OpcUa_SecureListener_ChannelManager* a_pChannelManager)
{
    OpcUa_SecureChannel* pTmpSecureChannel = OpcUa_Null;

    if(OpcUa_Null != a_pChannelManager->SecureChannels)
    {
        /* remove all channels and delete list */
        OPCUA_SECURELISTENER_CHANNELMANAGER_LOCK("OpcUa_SecureListener_ChannelManager_Clear", a_pChannelManager->SecureChannels);

        OpcUa_List_ResetCurrent(a_pChannelManager->SecureChannels);
        pTmpSecureChannel = (OpcUa_SecureChannel*)OpcUa_List_GetCurrentElement(a_pChannelManager->SecureChannels);

        while(pTmpSecureChannel != OpcUa_Null)
        {
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ChannelManager_Clear: SecureChannel removed!\n");
            OpcUa_List_DeleteCurrentElement(a_pChannelManager->SecureChannels);
            OpcUa_TcpSecureChannel_Delete(&pTmpSecureChannel);
            pTmpSecureChannel = (OpcUa_SecureChannel*)OpcUa_List_GetCurrentElement(a_pChannelManager->SecureChannels);
        }

        OPCUA_SECURELISTENER_CHANNELMANAGER_UNLOCK("OpcUa_SecureListener_ChannelManager_Clear", a_pChannelManager->SecureChannels);
        OpcUa_List_Delete(&(a_pChannelManager->SecureChannels));
    }

    if(OpcUa_Null != a_pChannelManager->hLifeTimeWatchDog)
    {
        OpcUa_Timer_Delete(&(a_pChannelManager->hLifeTimeWatchDog));
    }
}

/*==============================================================================*/
/* OpcUa_SecureListener_ChannelManager_Delete                                   */
/*==============================================================================*/
OpcUa_Void OpcUa_SecureListener_ChannelManager_Delete(
    OpcUa_SecureListener_ChannelManager** a_ppChannelManager)
{
    if (a_ppChannelManager != OpcUa_Null)
    {
        OpcUa_SecureListener_ChannelManager_Clear(*a_ppChannelManager);

        OpcUa_Free(*a_ppChannelManager);
        *a_ppChannelManager = OpcUa_Null;
        return;
    }

    return;
}

/*==============================================================================*/
/* OpcUa_SecureListener_ChannelManager_IsValidChannel                           */
/*==============================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_ChannelManager_IsValidChannel(
    OpcUa_SecureListener_ChannelManager* a_pChannelManager,
    OpcUa_UInt32                         a_uSecureChannelID,
    OpcUa_SecureChannel**                a_ppSecureChannel)
{
    OpcUa_SecureChannel*      pTmpSecureChannel     = OpcUa_Null;
    OpcUa_TcpSecureChannel*   pTmpTcpSecureChannel  = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "ChannelManager_IsValidChannel");

    *a_ppSecureChannel = OpcUa_Null;

    OPCUA_SECURELISTENER_CHANNELMANAGER_LOCK("OpcUa_SecureListener_ChannelManager_IsValidChannel", a_pChannelManager->SecureChannels);
    OpcUa_List_ResetCurrent(a_pChannelManager->SecureChannels);

    pTmpSecureChannel = (OpcUa_SecureChannel*)OpcUa_List_GetCurrentElement(a_pChannelManager->SecureChannels);

    while(pTmpSecureChannel)
    {
        pTmpTcpSecureChannel = (OpcUa_TcpSecureChannel*)pTmpSecureChannel->Handle;

        if(pTmpSecureChannel->SecureChannelId == a_uSecureChannelID)
        {
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ChannelManager_IsValidChannel: Searched securechannel found!\n");
            *a_ppSecureChannel = pTmpSecureChannel;
            pTmpSecureChannel = OpcUa_Null;
            break;
        }
        pTmpSecureChannel = (OpcUa_SecureChannel *)OpcUa_List_GetNextElement(a_pChannelManager->SecureChannels);
    }

    OPCUA_SECURELISTENER_CHANNELMANAGER_UNLOCK("OpcUa_SecureListener_ChannelManager_IsValidChannel", a_pChannelManager->SecureChannels);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OPCUA_SECURELISTENER_CHANNELMANAGER_UNLOCK("OpcUa_SecureListener_ChannelManager_IsValidChannel", a_pChannelManager->SecureChannels);

    OpcUa_Free(*a_ppSecureChannel);
    *a_ppSecureChannel = OpcUa_Null;

    pTmpSecureChannel = OpcUa_Null;

OpcUa_FinishErrorHandling;
}

/*==============================================================================*/
/* OpcUa_SecureListener_ChannelManager_AddChannel                               */
/*==============================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_ChannelManager_AddChannel(
    OpcUa_SecureListener_ChannelManager* a_pChannelManager,
    OpcUa_SecureChannel*                 a_pChannel)
{
    OpcUa_UInt32    nChannelCount = 0;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "ChannelManager_AddChannel");

    OpcUa_ReturnErrorIfArgumentNull(a_pChannel);
    OpcUa_ReturnErrorIfArgumentNull(a_pChannelManager);
    OpcUa_ReturnErrorIfArgumentNull(a_pChannelManager->SecureChannels);

    OPCUA_SECURELISTENER_CHANNELMANAGER_LOCK("OpcUa_SecureListener_ChannelManager_AddChannel", a_pChannelManager->SecureChannels);

    OpcUa_List_GetNumberOfElements(a_pChannelManager->SecureChannels, &nChannelCount);

    a_pChannel->uRefCount = 0;
    a_pChannel->ReleaseMethod = OpcUa_SecureListener_ChannelManager_ReleaseChannel;
    a_pChannel->ReleaseParam  = a_pChannelManager;
    uStatus = OpcUa_List_AddElement(a_pChannelManager->SecureChannels, a_pChannel);
    OpcUa_GotoErrorIfBad(uStatus);

    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ChannelManager_AddChannel: SecureChannel added! %u in list\n", nChannelCount);

    OPCUA_SECURELISTENER_CHANNELMANAGER_UNLOCK("OpcUa_SecureListener_ChannelManager_AddChannel", a_pChannelManager->SecureChannels);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OPCUA_SECURELISTENER_CHANNELMANAGER_UNLOCK("OpcUa_SecureListener_ChannelManager_AddChannel", a_pChannelManager->SecureChannels);

OpcUa_FinishErrorHandling;
}

/*==============================================================================*/
/* OpcUa_SecureListener_ChannelManager_ReleaseChannel                           */
/*==============================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_ChannelManager_ReleaseChannel(
    OpcUa_SecureListener_ChannelManager* a_pChannelManager,
    OpcUa_SecureChannel**                a_ppSecureChannel)
{
OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "ChannelManager_ReleaseChannel");

    OpcUa_ReturnErrorIfArgumentNull(a_pChannelManager);
    OpcUa_ReturnErrorIfArgumentNull(a_pChannelManager->SecureChannels);
    OpcUa_ReturnErrorIfArgumentNull(a_ppSecureChannel);

    OpcUa_List_Enter(a_pChannelManager->SecureChannels);

    if (*a_ppSecureChannel == OpcUa_Null)
    {
        OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
    }
    else if ((*a_ppSecureChannel)->uRefCount == 0)
    {
        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "SecureListener - ChannelManager_ReleaseChannel: invalid ref count!\n");
        OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidState);
    }
    else
    {
        if((*a_ppSecureChannel)->uRefCount > 0)
        {
            (*a_ppSecureChannel)->uRefCount--;
            /* OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ChannelManager_ReleaseChannel: Searched SecureChannel with id %u refs %u!\n", (*a_ppSecureChannel)->SecureChannelId, (*a_ppSecureChannel)->uRefCount); */
        }
        else
        {
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ChannelManager_ReleaseChannel: Reference counter underflow at SecureChannel with id %u!\n", (*a_ppSecureChannel)->SecureChannelId);
            OpcUa_GotoErrorWithStatus(OpcUa_BadInternalError);
        }

        *a_ppSecureChannel = OpcUa_Null;
    }

    OpcUa_List_Leave(a_pChannelManager->SecureChannels);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
    OpcUa_List_Leave(a_pChannelManager->SecureChannels);
OpcUa_FinishErrorHandling;
}

/*==============================================================================*/
/* OpcUa_SecureListener_ChannelManager_GetChannelBySecureChannelID              */
/*==============================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_ChannelManager_GetChannelBySecureChannelID(
    OpcUa_SecureListener_ChannelManager* a_pChannelManager,
    OpcUa_UInt32                         a_uSecureChannelID,
    OpcUa_SecureChannel**                a_ppSecureChannel)
{
    OpcUa_SecureChannel*      pTmpSecureChannel     = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "GetChannelBySecureChannelID");

    *a_ppSecureChannel = OpcUa_Null;

    OPCUA_SECURELISTENER_CHANNELMANAGER_LOCK("OpcUa_SecureListener_ChannelManager_GetChannelBySecureChannelID", a_pChannelManager->SecureChannels);

    uStatus = OpcUa_List_ResetCurrent(a_pChannelManager->SecureChannels);
    OpcUa_GotoErrorIfBad(uStatus);

    pTmpSecureChannel = (OpcUa_SecureChannel*)OpcUa_List_GetCurrentElement(a_pChannelManager->SecureChannels);

    while(pTmpSecureChannel != OpcUa_Null)
    {
        if(pTmpSecureChannel->SecureChannelId == a_uSecureChannelID)
        {
            *a_ppSecureChannel = pTmpSecureChannel;
            if(pTmpSecureChannel->uRefCount < OpcUa_UInt32_Max)
            {
                pTmpSecureChannel->uRefCount++;
                OPCUA_SECURELISTENER_CHANNELMANAGER_UNLOCK("OpcUa_SecureListener_ChannelManager_GetChannelBySecureChannelID", a_pChannelManager->SecureChannels);
                /* OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ChannelManager_GetChannelBySecureChannelID: Searched SecureChannel with id %u refs %u!\n", a_uSecureChannelID, pTmpSecureChannel->uRefCount); */
                OpcUa_ReturnStatusCode;
            }
            else
            {
				OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ChannelManager_GetChannelBySecureChannelID: Reference counter overflow at SecureChannel with id %u!\n", a_uSecureChannelID);
                OpcUa_GotoErrorWithStatus(OpcUa_BadInternalError);
            }
        }

        pTmpSecureChannel = (OpcUa_SecureChannel *)OpcUa_List_GetNextElement(a_pChannelManager->SecureChannels);
    }

    OPCUA_SECURELISTENER_CHANNELMANAGER_UNLOCK("OpcUa_SecureListener_ChannelManager_GetChannelBySecureChannelID", a_pChannelManager->SecureChannels);

    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ChannelManager_GetChannelBySecureChannelID: Searched SecureChannel with id %u NOT found!\n", a_uSecureChannelID);
    uStatus = OpcUa_BadSecureChannelIdInvalid;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OPCUA_SECURELISTENER_CHANNELMANAGER_UNLOCK("OpcUa_SecureListener_ChannelManager_GetChannelBySecureChannelID", a_pChannelManager->SecureChannels);

OpcUa_FinishErrorHandling;
}

/*==============================================================================*/
/* OpcUa_SecureListener_ChannelManager_GetChannelByTransportConnection          */
/*==============================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_ChannelManager_GetChannelByTransportConnection(
    OpcUa_SecureListener_ChannelManager* a_pChannelManager,
    OpcUa_Handle                         a_hTransportConnection,
    OpcUa_SecureChannel**                a_ppSecureChannel)
{
    OpcUa_SecureChannel*      pTmpSecureChannel     = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "GetChannelByTransportConnection");

    *a_ppSecureChannel = OpcUa_Null;

    OPCUA_SECURELISTENER_CHANNELMANAGER_LOCK("OpcUa_SecureListener_ChannelManager_GetChannelByTransportConnection", a_pChannelManager->SecureChannels);

    uStatus = OpcUa_List_ResetCurrent(a_pChannelManager->SecureChannels);
    OpcUa_GotoErrorIfBad(uStatus);

    pTmpSecureChannel = (OpcUa_SecureChannel*)OpcUa_List_GetCurrentElement(a_pChannelManager->SecureChannels);

    while(pTmpSecureChannel != OpcUa_Null)
    {
        if(     pTmpSecureChannel->TransportConnection != OpcUa_Null
            &&  pTmpSecureChannel->TransportConnection == a_hTransportConnection) /* pointer valid and not reused till after this call */
        {
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ChannelManager_GetChannelByTransportConnection: Searched securechannel found!\n");
            *a_ppSecureChannel = pTmpSecureChannel;

            if(pTmpSecureChannel->uRefCount < OpcUa_UInt32_Max)
            {
                pTmpSecureChannel->uRefCount++;
                OPCUA_SECURELISTENER_CHANNELMANAGER_UNLOCK("OpcUa_SecureListener_ChannelManager_GetChannelByTransportConnection", a_pChannelManager->SecureChannels);
                /* OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ChannelManager_GetChannelByTransportConnection: Searched SecureChannel with id %u refs %u!\n", pTmpSecureChannel->SecureChannelId, pTmpSecureChannel->uRefCount); */
                OpcUa_ReturnStatusCode;
            }
            else
            {
				OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ChannelManager_GetChannelByTransportConnection: Reference counter overflow at SecureChannel with id %u!\n", pTmpSecureChannel->SecureChannelId);
                OpcUa_GotoErrorWithStatus(OpcUa_BadInternalError);
            }
        }

        pTmpSecureChannel = (OpcUa_SecureChannel *)OpcUa_List_GetNextElement(a_pChannelManager->SecureChannels);
    }

    OPCUA_SECURELISTENER_CHANNELMANAGER_UNLOCK("OpcUa_SecureListener_ChannelManager_GetChannelByTransportConnection", a_pChannelManager->SecureChannels);

    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_SecureListener_ChannelManager_GetChannelByTransportConnection: Searched SecureChannel NOT found!\n");
    uStatus = OpcUa_BadSecureChannelIdInvalid;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OPCUA_SECURELISTENER_CHANNELMANAGER_UNLOCK("OpcUa_SecureListener_ChannelManager_GetChannelByTransportConnection", a_pChannelManager->SecureChannels);

OpcUa_FinishErrorHandling;
}

#endif /* OPCUA_HAVE_SERVERAPI */
