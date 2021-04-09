/* ========================================================================
 * Copyright (c) 2005-2009 The OPC Foundation, Inc. All rights reserved.
 *
 * OPC Foundation MIT License 1.00
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * The complete license agreement can be found here:
 * http://opcfoundation.org/License/MIT/1.00/
 * ======================================================================*/

/******************************************************************************************************/
/* Platform Portability Layer                                                                         */
/* Modify the content of this file according to the socket implementation on your system.             */
/*                                                                                          */
/******************************************************************************************************/

#define OPCUA_P_SOCKET_ISSUBSCRIBED(xSocket) ((xSocket->pfnEventCallback != OpcUa_Null) && !(xSocket->Flags.bIsShutDown))
/* System Headers */
#include <opcua_p_os.h>

/* UA platform definitions */
#include <opcua_p_internal.h>

/* additional UA dependencies */
#include <opcua_datetime.h>

/* platform layer includes */
#include <opcua_p_thread.h>
#include <opcua_mutex.h>
#include <opcua_p_semaphore.h>
#include <opcua_semaphore.h>
#include <opcua_p_utilities.h>
#include <opcua_p_memory.h>

#include <opcua_trace.h>
/* own headers */
#include <opcua_p_socket.h>
#include <opcua_p_socket_internal.h>
#include <opcua_p_socket_interface.h>

/* platform layer includes */
#include <opcua_p_timer.h> /* for timered select */
#include "opcua_string.h"

#ifdef _MSC_VER
/* this pragma is for win32 */
#pragma warning(disable:4127) /* suppress "conditional expression is constant" in fdset macros */
#endif /* _MSC_VER */


#if OPCUA_MULTITHREADED

#if OPCUA_P_SOCKETMANAGER_STATICARRAY
    extern OpcUa_InternalSocketManager* OpcUa_P_Socket_g_pSocketManagers[OPCUA_P_SOCKETMANAGER_STATICMAX];
#endif /* OPCUA_P_SOCKETMANAGER_STATICARRAY */

    #if OPCUA_USE_SYNCHRONISATION
        extern OpcUa_Mutex  OpcUa_P_Socket_g_SocketManagersMutex;
        extern OpcUa_Mutex  OpcUa_P_Socket_g_ShutdownMutex;
        extern OpcUa_UInt32 OpcUa_P_Socket_g_uNuOfClientThreads;
    #endif /* OPCUA_USE_SYNCHRONISATION */
#endif /* OPCUA_MULTITHREADED */

extern OpcUa_InternalSocketManager OpcUa_Socket_g_SocketManager;
extern OpcUa_InternalSocket OpcUa_Socket_g_SocketArray[OPCUA_P_SOCKETMANAGER_NUMBEROFSOCKETS];
extern int OpcUa_P_OpenSSL_g_iSocketDataIndex;



/*============================================================================
 * Allocate Socket Type
 *===========================================================================*/
OpcUa_Socket OpcUa_Socket_Alloc(OpcUa_Void)
{
    OpcUa_InternalSocket* pInternalSocket = OpcUa_Null;
    pInternalSocket = (OpcUa_InternalSocket*)malloc(sizeof(OpcUa_InternalSocket));
    OpcUa_Socket_Initialize((OpcUa_Socket)pInternalSocket);
    return (OpcUa_Socket)pInternalSocket;
}

/*============================================================================
 * Initialize Socket Type
 *===========================================================================*/
OpcUa_Void OpcUa_Socket_Initialize(OpcUa_Socket a_pSocket)
{
    OpcUa_InternalSocket* pInternalSocket = OpcUa_Null;

    if(a_pSocket == OpcUa_Null)
    {
        return;
    }

    pInternalSocket = (OpcUa_InternalSocket*)a_pSocket;

    OPCUA_SOCKET_INVALIDATE(pInternalSocket);

    pInternalSocket->rawSocket = (OpcUa_RawSocket)OPCUA_P_SOCKET_INVALID;

#if OPCUA_MULTITHREADED
    OpcUa_Mutex_Create(&(pInternalSocket->pMutex));
#endif /* OPCUA_MULTITHREADED */
}

/*============================================================================
 * Clear Socket Type
 *===========================================================================*/
OpcUa_Void OpcUa_Socket_Clear(OpcUa_Socket a_pSocket)
{
    OpcUa_InternalSocket* pInternalSocket = OpcUa_Null;

    if(a_pSocket == OpcUa_Null)
    {
        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_Clear: Invalid handle!\n");
        return;
    }

    pInternalSocket = (OpcUa_InternalSocket*)a_pSocket;

    OPCUA_SOCKET_INVALIDATE(pInternalSocket);

    if(pInternalSocket->rawSocket != (OpcUa_RawSocket)OPCUA_P_SOCKET_INVALID)
    {
        pInternalSocket->rawSocket = (OpcUa_RawSocket)OPCUA_P_SOCKET_INVALID;
    }
#if OPCUA_MULTITHREADED
    if(pInternalSocket->pMutex != OpcUa_Null)
		OpcUa_Mutex_Delete(&(pInternalSocket->pMutex));
#endif /* OPCUA_MULTITHREADED */
}

/*============================================================================
 * Delete Socket Type
 *===========================================================================*/
OpcUa_Void OpcUa_Socket_Delete(OpcUa_Socket* a_ppSocket)
{
    OpcUa_InternalSocket* pInternalSocket = OpcUa_Null;

    if(a_ppSocket == OpcUa_Null)
    {
        return;
    }

    if(*a_ppSocket == OpcUa_Null)
    {
        return;
    }

    pInternalSocket = (OpcUa_InternalSocket*)*a_ppSocket;

    OpcUa_Socket_Clear(pInternalSocket);

    free(pInternalSocket);

    *a_ppSocket = OpcUa_Null;
}

/*============================================================================
 * Allocate SocketManager Type
 *===========================================================================*/
OpcUa_SocketManager OpcUa_SocketManager_Alloc(OpcUa_Void)
{
    OpcUa_InternalSocketManager*    pInternalSocketManager  = OpcUa_Null;

    pInternalSocketManager = (OpcUa_InternalSocketManager*)malloc(sizeof(OpcUa_InternalSocketManager));

    if(pInternalSocketManager == OpcUa_Null)
    {
        return OpcUa_Null;
    }

    return (OpcUa_SocketManager)pInternalSocketManager;
}

/*============================================================================
 * Initialize SocketManager Type
 *===========================================================================*/
OpcUa_Void OpcUa_SocketManager_Initialize(OpcUa_SocketManager a_pSocketManager)
{
    OpcUa_InternalSocketManager* pInternalSocketManager = OpcUa_Null;

    if(a_pSocketManager == OpcUa_Null)
    {
        return;
    }

    OpcUa_MemSet(a_pSocketManager, 0, sizeof(OpcUa_InternalSocketManager));

    pInternalSocketManager = (OpcUa_InternalSocketManager*)a_pSocketManager;

    pInternalSocketManager->pSockets                = OpcUa_Null;
    pInternalSocketManager->uintMaxSockets          = 0;
    pInternalSocketManager->pCookie                 = OpcUa_Null;
    pInternalSocketManager->uintLastExternalEvent   = OPCUA_SOCKET_NO_EVENT;
    pInternalSocketManager->pThread                 = OpcUa_Null;
    pInternalSocketManager->pMutex                  = OpcUa_Null;
}

/*============================================================================
 * Create the Sockets in the List
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SocketManager_CreateSockets(
    OpcUa_SocketManager a_pSocketManager,
    OpcUa_UInt32        a_uMaxSockets)
{
    OpcUa_UInt32                 ntemp                  = 0;
    OpcUa_InternalSocketManager* pInternalSocketManager = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Socket, "CreateSockets");

    OpcUa_GotoErrorIfArgumentNull(a_pSocketManager);

    pInternalSocketManager = (OpcUa_InternalSocketManager*)a_pSocketManager;

    OpcUa_Mutex_Lock(pInternalSocketManager->pMutex);

    pInternalSocketManager->pSockets = (OpcUa_InternalSocket *)malloc(sizeof(OpcUa_InternalSocket) * a_uMaxSockets);
    OpcUa_GotoErrorIfAllocFailed(pInternalSocketManager->pSockets);

    /* initialize the whole socket list with zero */
    OpcUa_MemSet(pInternalSocketManager->pSockets, 0, sizeof(OpcUa_InternalSocket) * a_uMaxSockets);

    for(ntemp = 0; ntemp < a_uMaxSockets; ntemp++)
    {
        OpcUa_Socket_Initialize(&(pInternalSocketManager->pSockets[ntemp]));
    }

    pInternalSocketManager->uintMaxSockets = a_uMaxSockets;

    OpcUa_Mutex_Unlock(pInternalSocketManager->pMutex);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/************************* Internal Helper Functions *************************/

#if OPCUA_P_SOCKETMANAGER_SUPPORT_SSL

#if OPCUA_P_SOCKET_USE_SSL_VERIFY_CALLBACK
/*  If verify_callback returns 0, the verification process is immediately
    stopped with ``verification failed'' state. If SSL_VERIFY_PEER is set,
    a verification failure alert is sent to the peer and the TLS/SSL handshake
    is terminated. If verify_callback returns 1, the verification process
    is continued. If verify_callback always returns 1, the TLS/SSL handshake
    will not be terminated with respect to verification failures and the
    connection will be established. */
int OpcUa_Socket_SslVerifyCallback(int              a_iPreverifyOk,
                                   X509_STORE_CTX*  a_pX509_store_ctx)
{
    SSL*                    pSslContext     = NULL;
    OpcUa_InternalSocket*   pInternalSocket = NULL;

    if(a_iPreverifyOk == 0)
    {
        /* certificate not ok */
        X509*               err_cert    = NULL;
        int                 err         = 0;
        int                 depth       = 0;
        char                buf[256];

        err      = X509_STORE_CTX_get_error(a_pX509_store_ctx);
        err_cert = X509_STORE_CTX_get_current_cert(a_pX509_store_ctx);
        depth    = X509_STORE_CTX_get_error_depth(a_pX509_store_ctx);

        if(err_cert == NULL)
        {
            X509err(X509_F_X509_VERIFY_CERT, X509_R_NO_CERT_SET_FOR_US_TO_VERIFY);
            return 0; /* stop validation */
        }

        if(err == X509_V_ERR_UNABLE_TO_GET_CRL)
        {
            /* there may be no CRL available, so ignore 'unable to get certificate CRL' error */
            return 1; /* continue validation */
        }

        X509_NAME_oneline(X509_get_subject_name(err_cert), buf, 256);
        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "\nverify error:\n\tnum=%d:%s\n\tdepth=%d\n\t%s\n", err, X509_verify_cert_error_string(err), depth, buf);

        /* retrieve context related socket */
        pSslContext = X509_STORE_CTX_get_ex_data(a_pX509_store_ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
        pInternalSocket = (OpcUa_InternalSocket*)SSL_get_ex_data(pSslContext, 0);

        if(     pInternalSocket != OpcUa_Null
            &&  pInternalSocket->pfnCertificateValidation != OpcUa_Null)
        {
            OpcUa_StatusCode uStatus = OpcUa_BadCertificateInvalid;
            OpcUa_ByteString Certificate = OPCUA_BYTESTRING_STATICINITIALIZER;
            unsigned char*   pchTemp = NULL;

            Certificate.Length = i2d_X509(err_cert, NULL);

            if(Certificate.Length <= 0)
            {
                return 0;
            }

            Certificate.Data = (OpcUa_Byte*)OpcUa_P_Memory_Alloc(Certificate.Length);
            if(Certificate.Data == NULL)
            {
                return 0;
            }

            pchTemp = Certificate.Data;
            Certificate.Length = i2d_X509(err_cert, &pchTemp);

            uStatus = pInternalSocket->pfnCertificateValidation(pInternalSocket,
                                                                pInternalSocket->pvUserData,
                                                               &Certificate,
                                                                uStatus);

            OpcUa_P_ByteString_Clear(&Certificate);

            if(OpcUa_IsEqual(OpcUa_BadContinue))
            {
                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "SSL certificate error ignored by application!\n", err, X509_verify_cert_error_string(err), depth, buf);
                return 1; /* continue validation */
            }
        }
    }

    return a_iPreverifyOk; /* keep result */
}
#endif

#if OPCUA_P_SOCKET_SSL_USE_OWN_CERT_VERIFY
/* custom replacement for X509_verify_cert() */
int OpcUa_Socket_SslCertificateVerifyCallback(X509_STORE_CTX*   a_pX509_store_ctx,
                                              void*             a_pvArgument)
{
    OpcUa_InternalSocket* pSocket = OpcUa_Null;

    pSocket = (OpcUa_InternalSocket*)a_pvArgument;

    if(a_pX509_store_ctx->cert == NULL)
    {
        X509err(    X509_F_X509_VERIFY_CERT,
                    X509_R_NO_CERT_SET_FOR_US_TO_VERIFY);

        return -1;
    }

    return 1; /* 1 == accept certificate; < 0 reject certificate and abort connect. */
}
#endif


/*============================================================================
 * Create SSL context for given socket
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Socket_SetSslContext(OpcUa_InternalSocket*   a_pSocket,
                                            OpcUa_ByteString*       a_pServerCertificate,
                                            OpcUa_Key*              a_pServerPrivateKey,
                                            OpcUa_Void*             a_pPKIConfig,
                                            OpcUa_Int               a_iMode,
                                            const SSL_METHOD*       a_pSSLMethod)
{
    int                                         iResult     = 0;
    OpcUa_CertificateStoreConfiguration*        pPKIConfig  = (OpcUa_CertificateStoreConfiguration*)a_pPKIConfig;
    EVP_PKEY*                                   pKey        = NULL;
    X509*                                       pCert       = NULL;
    const unsigned char*                        pData;

OpcUa_InitializeStatus(OpcUa_Module_Socket, "OpcUa_Socket_SetSslContext");

    OpcUa_ReturnErrorIfArgumentNull(a_pSocket);
    OpcUa_ReturnErrorIfArgumentNull(a_pSSLMethod);

    /* Convert certificate and key */
    if(     a_pServerCertificate != OpcUa_Null && a_pServerCertificate->Data    != OpcUa_Null && a_pServerCertificate->Length > 0
        &&  a_pServerPrivateKey  != OpcUa_Null && a_pServerPrivateKey->Key.Data != OpcUa_Null && a_pServerPrivateKey->Key.Length > 0)
    {
        /* convert certificate */
        pData = a_pServerCertificate->Data;
        pCert = d2i_X509(   (X509**)OpcUa_Null,
                            (const unsigned char**)&pData,
                            a_pServerCertificate->Length);
        OpcUa_GotoErrorIfNull(pCert, OpcUa_BadInvalidArgument);

        /* convert key */
        pData = a_pServerPrivateKey->Key.Data;
        pKey = d2i_PrivateKey(EVP_PKEY_RSA,OpcUa_Null, &pData, a_pServerPrivateKey->Key.Length);
        OpcUa_GotoErrorIfNull(pKey, OpcUa_BadInvalidArgument);
    }

    /* mark socket as SSL socket */
    a_pSocket->Flags.bSSL = -1;

    /* create SSL context (TODO: would it be enough for servers to hold a single context at the listen socket?) */
#if OPENSSL_VERSION_NUMBER >= 0x1000000fL
    a_pSocket->pSSLContext = SSL_CTX_new(a_pSSLMethod);
#else
    a_pSocket->pSSLContext = SSL_CTX_new((SSL_METHOD*)a_pSSLMethod);
#endif
    if(a_pSocket->pSSLContext == OpcUa_Null)
    {
        /* error */
        OpcUa_GotoErrorWithStatus(OpcUa_BadInternalError);
    }

    /* set certificate */
    if(pCert != OpcUa_Null && pKey != OpcUa_Null)
    {
        iResult = SSL_CTX_use_certificate(  a_pSocket->pSSLContext,
                                            pCert);
    }
    else
    {
        iResult = 1;
    }

    if(iResult == 0)
    {
        /* error */
        ERR_print_errors_fp(stderr);
        OpcUa_GotoErrorWithStatus(OpcUa_BadInternalError);
    }

    /* set private key */
    if(pCert != OpcUa_Null && pKey != OpcUa_Null)
    {
        iResult = SSL_CTX_use_PrivateKey(a_pSocket->pSSLContext, pKey);
    }
    else
    {
        iResult = 1;
    }

    if(iResult == 0)
    {
        /* error */
        ERR_print_errors_fp(stderr);
        OpcUa_GotoErrorWithStatus(OpcUa_BadInternalError);
    }

    /* Load CA */
    if(     pPKIConfig                                          != OpcUa_Null
        &&  strcmp(pPKIConfig->strPkiType, OPCUA_PKI_TYPE_NONE) != 0
        &&  pPKIConfig->strIssuerCertificateStoreLocation       != OpcUa_Null)
    {
        iResult = SSL_CTX_load_verify_locations(a_pSocket->pSSLContext,
                                                OpcUa_Null, /* multi cert PEM */
                                                pPKIConfig->strIssuerCertificateStoreLocation); /* folder containing multiple CA PEM certs */
    }
    else
    {
        iResult = 1;
    }

    if(iResult == 0)
    {
        /* error */
        ERR_print_errors_fp(stderr);
        OpcUa_GotoErrorWithStatus(OpcUa_BadInternalError);
    }

    if(a_iMode != SSL_VERIFY_NONE)
    {
#if OPCUA_P_SOCKET_SSL_USE_OWN_CERT_VERIFY
        SSL_CTX_set_cert_verify_callback(a_pSocket->pSSLContext, OpcUa_Socket_SslCertificateVerifyCallback, (void*)a_pSocket);
#endif

        /* set requested certificate verification mode */
#if OPCUA_P_SOCKET_USE_SSL_VERIFY_CALLBACK
        /* not tested */
        SSL_CTX_set_verify(a_pSocket->pSSLContext, a_iMode, OpcUa_Socket_SslVerifyCallback);
#else
        SSL_CTX_set_verify(a_pSocket->pSSLContext, a_iMode, NULL);
#endif
    }

    /* create SSL connection */
    a_pSocket->pSSLConnection   = SSL_new(a_pSocket->pSSLContext);
    a_pSocket->eSSLState        = OpcUa_P_SSLState_WaitForTransport;
    a_pSocket->bSSLConnected    = OpcUa_False;

    /* TODO: Test which alternative works best; first one is preferred. */
    /* bind native socket to SSL connection */
#if 0
    iResult = SSL_set_fd(a_pSocket->pSSLConnection, (int)((SOCKET)(a_pSocket->rawSocket)));
    if(iResult == 0)
    {
        /* error */
        ERR_print_errors_fp(stderr);
        OpcUa_GotoErrorWithStatus(OpcUa_BadInternalError);
    }
#else
    {
        BIO *sbio = BIO_new_socket((int)((SOCKET)(a_pSocket->rawSocket)), BIO_NOCLOSE);
        if(sbio != NULL)
        {
            SSL_set_bio(a_pSocket->pSSLConnection, sbio, sbio);
        }
        else
        {
            OpcUa_GotoErrorWithStatus(OpcUa_BadInternalError);
        }
    }
#endif

    iResult = SSL_set_ex_data(a_pSocket->pSSLConnection, OpcUa_P_OpenSSL_g_iSocketDataIndex, a_pSocket);
    if(iResult == 0)
    {
        /* error */
        ERR_print_errors_fp(stderr);
        OpcUa_GotoErrorWithStatus(OpcUa_BadInternalError);
    }

    EVP_PKEY_free(pKey);
    X509_free(pCert);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    if(a_pSocket != OpcUa_Null)
    {
        a_pSocket->Flags.bSSL     = 0;

        /* call SSL service to clean up */
        if(a_pSocket->pSSLConnection != OpcUa_Null)
        {
            /*int ret = SSL_shutdown(pInternalSocket->pSSLConnection);*/
            SSL_free(a_pSocket->pSSLConnection);
        }

        if(a_pSocket->pSSLContext != OpcUa_Null)
        {
            SSL_CTX_free(a_pSocket->pSSLContext);
            a_pSocket->pSSLContext = OpcUa_Null;
        }

        a_pSocket->pSSLConnection = OpcUa_Null;

        if(a_pSocket->pServerCertificate != OpcUa_Null)
        {
            OpcUa_P_ByteString_Clear(a_pSocket->pServerCertificate);
            OpcUa_P_Memory_Free(a_pSocket->pServerCertificate);
            a_pSocket->pServerCertificate = OpcUa_Null;
        }

        if(a_pSocket->pServerPrivateKey != OpcUa_Null)
        {
            OpcUa_Key_Clear(a_pSocket->pServerPrivateKey);
            OpcUa_P_Memory_Free(a_pSocket->pServerPrivateKey);
            a_pSocket->pServerPrivateKey = OpcUa_Null;
        }
    }

    if(pKey != NULL)
    {
        EVP_PKEY_free(pKey);
    }

    if(pCert != NULL)
    {
        X509_free(pCert);
    }

OpcUa_FinishErrorHandling;
}
#endif /* OPCUA_P_SOCKETMANAGER_SUPPORT_SSL */
/*============================================================================
 * Internal helper function to create a server socket
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SocketManager_InternalCreateServer(
    OpcUa_InternalSocketManager*    a_pSocketManager,
    OpcUa_UInt16                    a_uPort,
    OpcUa_ByteString*               a_pServerCertificate,
    OpcUa_Key*                      a_pServerPrivateKey,
    OpcUa_Void*                     a_pPKIConfig,
    OpcUa_Socket_EventCallback      a_pfnSocketCallBack,
    OpcUa_Void*                     a_pCallbackData,
    OpcUa_Socket*                   a_ppSocket,
    OpcUa_Boolean                   a_bUseSSL)
{
    OpcUa_StatusCode        uStatus			= OpcUa_Good;
    OpcUa_InternalSocket*   pInternalSocket = OpcUa_Null;

    *a_ppSocket = OpcUa_Null;

    /* create the main server socket and raise error if no socket is found */
    pInternalSocket = (OpcUa_InternalSocket*)OpcUa_SocketManager_FindFreeSocket(    a_pSocketManager,
                                                                            OpcUa_False);
    /* no free sockets, out of resources.. */
    OpcUa_ReturnErrorIfNull(pInternalSocket, OpcUa_BadMaxConnectionsReached);

    pInternalSocket->rawSocket = OpcUa_P_Socket_CreateServer(a_uPort, &uStatus);

    if(OpcUa_IsBad(uStatus))
    {
        pInternalSocket->Flags.bSocketIsInUse = 0;
        return OpcUa_BadCommunicationError;
    }

	OPCUA_SOCKET_SETVALID(pInternalSocket);

#if OPCUA_P_SOCKETMANAGER_SUPPORT_SSL

    if(a_bUseSSL != OpcUa_False)
    {
        uStatus = OpcUa_Socket_SetSslContext(   pInternalSocket,
                                                a_pServerCertificate,
                                                a_pServerPrivateKey,
                                                a_pPKIConfig,
                                                OPCUA_P_SOCKETMANAGER_SERVER_SSL_VERIFICATION,
                                                TLSv1_server_method());
        OpcUa_GotoErrorIfBad(uStatus);

        /* store for inheritance */
        if(a_pServerCertificate != OpcUa_Null)
        {
            pInternalSocket->pServerCertificate = OpcUa_P_Memory_Alloc(sizeof(OpcUa_ByteString));
            OpcUa_GotoErrorIfAllocFailed(pInternalSocket->pServerCertificate);
            uStatus = OpcUa_P_ByteString_Copy(  a_pServerCertificate,
                                                pInternalSocket->pServerCertificate);
            OpcUa_GotoErrorIfBad(uStatus);
        }

        if(a_pServerPrivateKey != OpcUa_Null)
        {
            pInternalSocket->pServerPrivateKey = OpcUa_P_Memory_Alloc(sizeof(OpcUa_Key));
            OpcUa_GotoErrorIfAllocFailed(pInternalSocket->pServerPrivateKey);
            OpcUa_Key_Copy( a_pServerPrivateKey,
                            pInternalSocket->pServerPrivateKey);
            OpcUa_GotoErrorIfBad(uStatus);
        }

        pInternalSocket->pPKIConfig         = a_pPKIConfig;
    }
    else
    {
        pInternalSocket->Flags.bSSL             = 0;
        pInternalSocket->eSSLState              = OpcUa_P_SSLState_Invalid;
        pInternalSocket->pSSLConnection         = OpcUa_Null;
        pInternalSocket->pSSLContext            = OpcUa_Null;
    }

#else /* OPCUA_P_SOCKETMANAGER_SUPPORT_SSL */

    OpcUa_ReferenceParameter(a_pServerCertificate);
    OpcUa_ReferenceParameter(a_pServerPrivateKey);
    OpcUa_ReferenceParameter(a_pPKIConfig);
    OpcUa_ReferenceParameter(a_bUseSSL);

#endif /* OPCUA_P_SOCKETMANAGER_SUPPORT_SSL */

#if OPCUA_USE_SYNCHRONISATION
    uStatus = OpcUa_Semaphore_Create( &pInternalSocket->hSemaphore, 0, 1);
    if(OpcUa_IsBad(uStatus))
    {
        uStatus = OpcUa_BadCommunicationError;
        goto Error;
    }
#endif

    pInternalSocket->pfnEventCallback       = a_pfnSocketCallBack;
    pInternalSocket->pvUserData             = a_pCallbackData;
    pInternalSocket->Flags.bIsListenSocket  = OpcUa_True;
    pInternalSocket->Flags.bOwnThread       = OpcUa_False;
    pInternalSocket->Flags.EventMask        = OPCUA_SOCKET_READ_EVENT | OPCUA_SOCKET_EXCEPT_EVENT | OPCUA_SOCKET_ACCEPT_EVENT | OPCUA_SOCKET_CLOSE_EVENT | OPCUA_SOCKET_TIMEOUT_EVENT;
    pInternalSocket->pSocketManager         = (OpcUa_InternalSocketManager *)a_pSocketManager;
    pInternalSocket->usPort                 = a_uPort;

    *a_ppSocket = pInternalSocket;

    return OpcUa_Good;

Error:
   if(pInternalSocket != OpcUa_Null)
        OpcUa_P_Socket_Delete(pInternalSocket);
    return uStatus;
}

/*============================================================================
*
*===========================================================================*/
OpcUa_StatusCode OpcUa_Socket_HandleAcceptEvent(    OpcUa_Socket a_pListenSocket,
                                                    OpcUa_Socket a_pAcceptedSocket) /* this is allowed to be null */
{
    OpcUa_UInt32            uAddress                = 0;
    OpcUa_UInt16            uPort                   = 0;
    OpcUa_RawSocket         AcceptedRawSocket       = (OpcUa_RawSocket)OPCUA_P_SOCKET_INVALID;
    OpcUa_InternalSocket*   pListenInternalSocket   = (OpcUa_InternalSocket*)a_pListenSocket;
    OpcUa_InternalSocket*   pAcceptInternalSocket   = (OpcUa_InternalSocket*)a_pAcceptedSocket;

    OpcUa_ReturnErrorIfArgumentNull(a_pListenSocket);

    AcceptedRawSocket = OpcUa_RawSocket_Accept(   pListenInternalSocket->rawSocket,
                                                    &uPort,
                                                    &uAddress,
                                                    OpcUa_True,
                                                    OpcUa_False);

    if(AcceptedRawSocket == (OpcUa_RawSocket)OPCUA_P_SOCKET_INVALID)
    {
        int iLastError = 0;

#ifdef WIN32
        iLastError = WSAGetLastError();
#endif

#ifdef _GNUC_
	iLastError = errno;
#endif

        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_Socket_HandleAcceptEvent: accepting failed with %i\n", iLastError);

        return OpcUa_BadCommunicationError;
    }

    /* accept but close if caller provided a null argument */
    if(a_pAcceptedSocket == OpcUa_Null)
    {
        OpcUa_RawSocket_Close(AcceptedRawSocket);
        AcceptedRawSocket = (OpcUa_RawSocket)OPCUA_P_SOCKET_INVALID;

        return OpcUa_BadMaxConnectionsReached;
    }
    else
    {
        pAcceptInternalSocket->rawSocket = AcceptedRawSocket;
    }

    if( pAcceptInternalSocket->rawSocket == (OpcUa_RawSocket)OPCUA_P_SOCKET_INVALID)
    {
        OPCUA_SOCKET_INVALIDATE(pAcceptInternalSocket);
        pAcceptInternalSocket->Flags.bSocketIsInUse   = OpcUa_False;

        return OpcUa_BadCommunicationError;
    }

    /* inherit from parent (listen) socket */
    pAcceptInternalSocket->pfnEventCallback       = pListenInternalSocket->pfnEventCallback;
    pAcceptInternalSocket->pvUserData             = pListenInternalSocket->pvUserData;
    OPCUA_SOCKET_SETVALID(pAcceptInternalSocket);
    pAcceptInternalSocket->Flags.bSocketIsInUse   = OpcUa_True;
    pAcceptInternalSocket->Flags.bIsListenSocket  = OpcUa_False;
    pAcceptInternalSocket->Flags.bIsShutDown      = OpcUa_False;
    pAcceptInternalSocket->Flags.bOwnThread       = OpcUa_False;
    pAcceptInternalSocket->Flags.EventMask        =   OPCUA_SOCKET_READ_EVENT
                                                    | OPCUA_SOCKET_EXCEPT_EVENT
                                                    | OPCUA_SOCKET_ACCEPT_EVENT
                                                    | OPCUA_SOCKET_CLOSE_EVENT
                                                    | OPCUA_SOCKET_TIMEOUT_EVENT;
    pAcceptInternalSocket->pSocketManager         = pListenInternalSocket->pSocketManager;
    pAcceptInternalSocket->usPort                 = pListenInternalSocket->usPort;
    pAcceptInternalSocket->Flags.bSSL             = pListenInternalSocket->Flags.bSSL;

#ifdef OPCUA_SOCKET_USESSL
    pAcceptInternalSocket->pSSL                   = pSocket->pSSL;
#endif /* OPCUA_SOCKET_USESSL*/

    return OpcUa_Good;
}

#if OPCUA_MULTITHREADED

#if OPCUA_P_SOCKETMANAGER_STATICARRAY
/*============================================================================
*
*===========================================================================*/
static OpcUa_Int32 OpcUa_SocketManager_GetSocketManagerSlot(OpcUa_InternalSocketManager* a_pSocketManager)
{
    OpcUa_Int32 iSlot = 0;

    OpcUa_Mutex_Lock(OpcUa_P_Socket_g_SocketManagersMutex);

    for(iSlot = 0; iSlot < OPCUA_P_SOCKETMANAGER_NUMBEROFSOCKETS; iSlot++)
    {
        if(OpcUa_P_Socket_g_pSocketManagers[iSlot] == OpcUa_Null)
        {
            OpcUa_P_Socket_g_pSocketManagers[iSlot] = a_pSocketManager;

            OpcUa_Mutex_Unlock(OpcUa_P_Socket_g_SocketManagersMutex);

            return iSlot;
        }
    }

    OpcUa_Mutex_Unlock(OpcUa_P_Socket_g_SocketManagersMutex);

    return -1;
}

/*============================================================================
*
*===========================================================================*/
static OpcUa_Void OpcUa_SocketManager_ReleaseSocketManagerSlot(OpcUa_UInt32 a_uSlot)
{
    OpcUa_Mutex_Lock(OpcUa_P_Socket_g_SocketManagersMutex);

    OpcUa_P_Socket_g_pSocketManagers[a_uSlot] = OpcUa_Null;

    OpcUa_Mutex_Unlock(OpcUa_P_Socket_g_SocketManagersMutex);

    return;
}
#endif /* OPCUA_P_SOCKETMANAGER_STATICARRAY */
/*============================================================================
* Takes appropriate action based on an event on a certain socket.
*===========================================================================*/
OpcUa_Void OpcUa_SocketManager_AcceptHandlerThread(OpcUa_Void* a_pArgument)
{

    OpcUa_InternalSocket*       pInternalSocket     = (OpcUa_InternalSocket*)a_pArgument;
    OpcUa_StatusCode            uStatus             = OpcUa_Good;
    OpcUa_Boolean               bEndLoop            = OpcUa_False;
#if OPCUA_P_SOCKETMANAGER_STATICARRAY
    OpcUa_Int32                 iSocketManagerSlot  = -1;
#endif /* OPCUA_P_SOCKETMANAGER_STATICARRAY */
    OpcUa_UInt32                uEventOccured       = OPCUA_SOCKET_NO_EVENT;
    OpcUa_InternalSocket        ClientSocket[2]; /* one for the client, one for _signals_ */
    OpcUa_InternalSocketManager SpawnedSocketManager;


    memset(&ClientSocket, 0, sizeof(OpcUa_InternalSocket) * 2);
    memset(&SpawnedSocketManager, 0, sizeof(OpcUa_SocketManager));

    /* update global control */
    OpcUa_Mutex_Lock(OpcUa_P_Socket_g_ShutdownMutex);
    OpcUa_P_Socket_g_uNuOfClientThreads++;
    OpcUa_Mutex_Unlock(OpcUa_P_Socket_g_ShutdownMutex);

    /* TODO: Check shutdown before dispatching? */

    /* handle event */
    uStatus = OpcUa_Socket_HandleAcceptEvent(  pInternalSocket,    /* listen socket */
                                               &ClientSocket[0]);  /* accepted socket */

    ClientSocket[0].Flags.bOwnThread = 1;

    /* release spawn semaphore */
    if(pInternalSocket->hSemaphore != OpcUa_Null)
    {
        OpcUa_Semaphore_Post( pInternalSocket->hSemaphore,
                                1);
    }

    if(OpcUa_IsGood(uStatus))
    {
#if OPCUA_P_SOCKETMANAGER_STATICARRAY
        /* obtain slot in global socket list array */
        iSocketManagerSlot = OpcUa_SocketManager_GetSocketManagerSlot(&SpawnedSocketManager);
        if(iSocketManagerSlot == -1 )
        {
            /* error, configuration maximum reached */
            OpcUa_P_Socket_Delete(&ClientSocket[0]);

            OpcUa_Mutex_Lock(OpcUa_P_Socket_g_ShutdownMutex);
            OpcUa_P_Socket_g_uNuOfClientThreads--;
            OpcUa_Mutex_Unlock(OpcUa_P_Socket_g_ShutdownMutex);

            return;
        }
#endif /*OPCUA_P_SOCKETMANAGER_STATICARRAY*/
        /* slot obtained, go on */
        SpawnedSocketManager.pThread                    = OpcUa_Null;
        SpawnedSocketManager.uintMaxSockets             = 2;
        SpawnedSocketManager.pSockets                   = ClientSocket;
        SpawnedSocketManager.pCookie                    = OpcUa_Null;
        SpawnedSocketManager.uintLastExternalEvent      = OPCUA_SOCKET_NO_EVENT;

        uStatus = OpcUa_Mutex_Create(&SpawnedSocketManager.pMutex);
		if (uStatus==OpcUa_Good)
		{
			SpawnedSocketManager.Flags.bStopServerLoop      = 0;
			SpawnedSocketManager.Flags.bSpawnThreadOnAccept = 0;
			SpawnedSocketManager.Flags.bRejectOnThreadFail  = -1;

			ClientSocket[0].pSocketManager = &SpawnedSocketManager;

			/* fire accept event */
			ClientSocket[0].pfnEventCallback(   (OpcUa_Socket)&ClientSocket[0],
												OPCUA_SOCKET_ACCEPT_EVENT,
												ClientSocket[0].pvUserData,
												ClientSocket[0].usPort,
												(OpcUa_Boolean)ClientSocket[0].Flags.bSSL);
			bEndLoop = OpcUa_False;

			do
			{
				uStatus = OpcUa_P_SocketManager_ServeLoopInternal(  &SpawnedSocketManager,
																	OPCUA_INFINITE,
																	&ClientSocket[0],
																	OPCUA_SOCKET_NO_EVENT,
																	&uEventOccured);

				if ((uEventOccured & OPCUA_SOCKET_CLOSE_EVENT)    ||
					(uEventOccured & OPCUA_SOCKET_SHUTDOWN_EVENT) ||
					(uEventOccured & OPCUA_SOCKET_EXCEPT_EVENT)   ||
					OpcUa_IsEqual(OpcUa_GoodShutdownEvent))
				{
					bEndLoop = OpcUa_True;
				}
			} while (   (bEndLoop==OpcUa_False)
					  &&(OpcUa_IsGood(uStatus)));
		} /* mutex creation failed */

        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SocketManager_AcceptHandlerThread: Client Handler shutting down! (0x%08X)\n", uStatus);
		if(SpawnedSocketManager.pMutex != OpcUa_Null)
			OpcUa_Mutex_Lock(SpawnedSocketManager.pMutex);

        if(ClientSocket[0].Flags.bInvalidSocket == 0)
        {
			OpcUa_Socket_HandleEvent(&ClientSocket[0], OPCUA_SOCKET_CLOSE_EVENT);
            OpcUa_P_Socket_Delete(&ClientSocket[0]);
        }
        if(ClientSocket[1].Flags.bInvalidSocket == 0)
        {
            OpcUa_P_Socket_Delete(&ClientSocket[0]);
        }
		if(SpawnedSocketManager.pMutex != OpcUa_Null)
		{
			OpcUa_Mutex_Unlock(SpawnedSocketManager.pMutex);
			OpcUa_Mutex_Delete(&SpawnedSocketManager.pMutex);
		}
#if OPCUA_P_SOCKETMANAGER_STATICARRAY
        /* release slot in global socket list array */
        OpcUa_SocketManager_ReleaseSocketManagerSlot(iSocketManagerSlot);
#endif /*OPCUA_P_SOCKETMANAGER_STATICARRAY*/

        /* loop ended */
        OpcUa_Mutex_Lock(OpcUa_P_Socket_g_ShutdownMutex);
        OpcUa_P_Socket_g_uNuOfClientThreads--;
        OpcUa_Mutex_Unlock(OpcUa_P_Socket_g_ShutdownMutex);
    }
    else
    {
        OpcUa_Mutex_Lock(OpcUa_P_Socket_g_ShutdownMutex);
        OpcUa_P_Socket_g_uNuOfClientThreads--;
        OpcUa_Mutex_Unlock(OpcUa_P_Socket_g_ShutdownMutex);
    }

    return;
}
#endif /* OPCUA_MULTITHREADED */
#if OPCUA_P_SOCKETMANAGER_SUPPORT_SSL
/*============================================================================
 * Set socket SSL state depending on error code
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Socket_ProcessSslError(  OpcUa_InternalSocket*   a_pSocket,
                                                OpcUa_Int               a_iSslError)
{
    int             iErr        = SSL_ERROR_NONE;
    OpcUa_StringA   sInfo       = OpcUa_Null;
    OpcUa_UInt32    uTraceLevel = OPCUA_TRACE_STACK_LEVEL_WARNING;

OpcUa_InitializeStatus(OpcUa_Module_Socket, "OpcUa_Socket_ProcessSslError");

    /* error */
    iErr = SSL_get_error(a_pSocket->pSSLConnection, a_iSslError);

    switch(iErr)
    {
    case SSL_ERROR_ZERO_RETURN:
        {
            a_pSocket->eSSLState = OpcUa_P_SSLState_Closed;
            sInfo = "SSL_ERROR_ZERO_RETURN";
            uStatus = OpcUa_BadCommunicationError;
            break;
        }
    case SSL_ERROR_WANT_READ:
        {
            a_pSocket->eSSLState = OpcUa_P_SSLState_WantRead;
            sInfo = "SSL_ERROR_WANT_READ";
            /* common return code -> lowest trace level */
            uTraceLevel = OPCUA_TRACE_STACK_LEVEL_INFO;
            break;
        }
    case SSL_ERROR_WANT_WRITE:
        {
            a_pSocket->eSSLState = OpcUa_P_SSLState_WantWrite;
            OPCUA_P_SET_BITS(a_pSocket->Flags.EventMask, OPCUA_SOCKET_WRITE_EVENT);
            sInfo = "SSL_ERROR_WANT_WRITE";
            break;
        }
    case SSL_ERROR_WANT_CONNECT:
        {
            a_pSocket->eSSLState = OpcUa_P_SSLState_WantConnect;
            sInfo = "SSL_ERROR_WANT_CONNECT";
            break;
        }
    case SSL_ERROR_WANT_ACCEPT:
        {
            a_pSocket->eSSLState = OpcUa_P_SSLState_WantAccept;
            sInfo = "SSL_ERROR_WANT_ACCEPT";
            break;
        }
    case SSL_ERROR_WANT_X509_LOOKUP:
        {
            sInfo = "SSL_ERROR_WANT_X509_LOOKUP";

            ERR_print_errors_fp(stderr);
            /* TODO */
            break;
        }
    case SSL_ERROR_SYSCALL:
        {
            unsigned long ulPeekedError = ERR_peek_error();

            sInfo = "SSL_ERROR_SYSCALL";

            if(ulPeekedError != 0)
            {
                ERR_print_errors_fp(stderr);
                uStatus = OpcUa_BadUnexpectedError;
            }
            else
            {
                if(a_iSslError == -1)
                {
                    /* underlying BIO error */
                    uStatus = OpcUa_Socket_GetLastError(a_pSocket);
                    if(OpcUa_IsGood(uStatus))
                    {
                        /* the system should report an error */
                        uStatus = OpcUa_BadUnexpectedError;
                    }
                    else
                    {
                        /* control trace level for known regular error codes. */
                        switch(uStatus)
                        {
                        case OpcUa_BadWouldBlock:
                            {
                                /* common return code -> lowest trace level */
                                uTraceLevel = OPCUA_TRACE_STACK_LEVEL_INFO;
                                break;
                            }
                        default:
                            {
                                break;
                            }
                        }
                    }
                }
                else
                {
                    /* it was a non fatal error */
                    uStatus = OpcUa_Bad;
                }
            }
            break;
        }
    case SSL_ERROR_SSL:
        {
            sInfo = "SSL_ERROR_SSL";
            ERR_print_errors_fp(stderr);
            uStatus = OpcUa_BadSecurityChecksFailed;
            break;
        }
    default:
        {
            /* unexpected error */
            sInfo = "unknown error";
            ERR_print_errors_fp(stderr);
            uStatus = OpcUa_BadUnexpectedError;
            break;
        }
    }

    OpcUa_Trace(uTraceLevel, "OpcUa_Socket_ProcessSslError: SSL error code is %s; mapped to status code 0x%08X\n", sInfo, uStatus);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * Check certificate verification result and try to obtain peer certificate.
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Socket_FinalizeSslConnect(   OpcUa_InternalSocket*   a_pInternalSocket,
                                                    OpcUa_Boolean           a_bNotify)
{
    long                lVerifyResult   = X509_V_OK;
    const char*         sVerifyResult   = NULL;
    X509*               peerCertificate = NULL;
    const SSL_CIPHER*   pCipher         = NULL;
    const char*         sCipherName     = NULL;

OpcUa_InitializeStatus(OpcUa_Module_Socket, "FinalizeSslConnect");

    OpcUa_ReturnErrorIfArgumentNull(a_pInternalSocket);
    OpcUa_ReturnErrorIfArgumentNull(a_pInternalSocket->pSSLConnection);

    /* collects some information about the certificate and ciphers used for the connection */
    lVerifyResult   = SSL_get_verify_result(    a_pInternalSocket->pSSLConnection);
    peerCertificate = SSL_get_peer_certificate( a_pInternalSocket->pSSLConnection);
    pCipher         = SSL_get_current_cipher(   a_pInternalSocket->pSSLConnection);

    /* get human readable strings */
    sVerifyResult   = X509_verify_cert_error_string(lVerifyResult);

    switch(lVerifyResult)
    {
    case X509_V_OK:
        {
            break;
        }
    default:
        {
            /* Handle verification error here */
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_Socket_FinalizeSslConnect: SSL certificate verification result %u -> \"%s\"!\n", lVerifyResult, sVerifyResult);
        }
    }

    if(peerCertificate == NULL)
    {
        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_Socket_FinalizeSslConnect: Could not retrieve peer certificate!\n");
    }
    else
    {
        if(     a_pInternalSocket                           != OpcUa_Null
            &&  a_bNotify                                   != OpcUa_False
            &&  a_pInternalSocket->pfnCertificateValidation != OpcUa_Null)
        {
            OpcUa_ByteString Certificate = OPCUA_BYTESTRING_STATICINITIALIZER;
            unsigned char*   pchTemp = NULL;

            Certificate.Length = i2d_X509(peerCertificate, NULL);

            if(Certificate.Length <= 0)
            {
                return 0;
            }

            Certificate.Data = (OpcUa_Byte*)OpcUa_P_Memory_Alloc(Certificate.Length);
            if(Certificate.Data == NULL)
            {
                return 0;
            }

            pchTemp = Certificate.Data;
            Certificate.Length = i2d_X509(peerCertificate, &pchTemp);

            uStatus = a_pInternalSocket->pfnCertificateValidation(a_pInternalSocket,
                                                                  a_pInternalSocket->pvUserData,
                                                                 &Certificate,
                                                                  OpcUa_Good);

            if(OpcUa_IsBad(uStatus) && OpcUa_IsNotEqual(OpcUa_BadContinue))
            {
                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_FinalizeSslConnect: Application rejects certificate with status 0x%08X!\n", uStatus);
            }
            else
            {
                uStatus = OpcUa_Good;
            }

            OpcUa_P_ByteString_Clear(&Certificate);
        }

        X509_free(peerCertificate);
    }

    if(OpcUa_IsGood(uStatus))
    {
        if(pCipher != NULL)
        {
            sCipherName = SSL_CIPHER_get_name(pCipher);
            if(sCipherName != NULL)
            {
                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_FinalizeSslConnect: Used cipher \"%s\"!\n", sCipherName);
            }
            else
            {
                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_Socket_FinalizeSslConnect: Could not obtain name of used cipher!\n");
            }
        }
        else
        {
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_Socket_FinalizeSslConnect: Could not obtain information about used cipher!\n");
        }
    }
    else
    {
        /* error happened or application rejected certificate */
        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_FinalizeSslConnect: Cancelling connection due status 0x%08X!\n", uStatus);
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

#endif /* OPCUA_P_SOCKETMANAGER_SUPPORT_SSL */
/*============================================================================
* Takes appropriate action based on an event on a certain socket.
*===========================================================================*/
OpcUa_StatusCode OpcUa_Socket_HandleEvent(  OpcUa_Socket a_pSocket,
                                            OpcUa_UInt32 a_uEvent)
{
    OpcUa_Socket            pAcceptedSocket = OpcUa_Null;
    OpcUa_InternalSocket*   pInternalSocket = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Socket, "HandleEvent");

    OpcUa_GotoErrorIfArgumentNull(a_pSocket);

    pInternalSocket = (OpcUa_InternalSocket*)a_pSocket;

    /* update last access variable */
    pInternalSocket->uintLastAccess = OpcUa_P_GetTickCount()/1000;

    switch(a_uEvent)
    {
    case OPCUA_SOCKET_READ_EVENT:
        {
			if (OPCUA_P_SOCKET_ISSUBSCRIBED(pInternalSocket))
			{
#if OPCUA_MULTITHREADED
				OpcUa_Mutex_Unlock(pInternalSocket->pSocketManager->pMutex);
#endif

#if OPCUA_P_SOCKETMANAGER_SUPPORT_SSL
                if(pInternalSocket->Flags.bSSL != OpcUa_False)
                {
                    if(pInternalSocket->bSSLConnected == OpcUa_False)
                    {
                        if(pInternalSocket->bClient != OpcUa_False)
                        {
                            /* continue client side SSL handshake */
                            int iResult = 0;

                            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_HandleEvent: Continuing SSL connect.\n");
                            iResult = SSL_connect(pInternalSocket->pSSLConnection);

                            switch(iResult)
                            {
                            case 1: /* successfully completed */
                                {
                                    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_HandleEvent: SSL connect complete.\n");

                                    uStatus = OpcUa_Socket_FinalizeSslConnect(pInternalSocket, OpcUa_True);

                                    if(OpcUa_IsGood(uStatus))
                                    {
                                        pInternalSocket->bSSLConnected  = OpcUa_True;
                                        pInternalSocket->eSSLState      = OpcUa_P_SSLState_Established;

                                        if(OPCUA_P_SOCKET_ISSUBSCRIBED(pInternalSocket))
                                        {
                                            uStatus = pInternalSocket->pfnEventCallback(a_pSocket, OPCUA_SOCKET_CONNECT_EVENT, pInternalSocket->pvUserData, pInternalSocket->usPort, (OpcUa_Boolean)pInternalSocket->Flags.bSSL);
                                        }
                                    }
                                    else
                                    {
                                        if(OPCUA_P_SOCKET_ISSUBSCRIBED(pInternalSocket))
                                        {
                                            uStatus = pInternalSocket->pfnEventCallback(a_pSocket, OPCUA_SOCKET_EXCEPT_EVENT, pInternalSocket->pvUserData, pInternalSocket->usPort, (OpcUa_Boolean)pInternalSocket->Flags.bSSL);
                                        }

                                        pInternalSocket->eSSLState      = OpcUa_P_SSLState_Closed;
                                        OpcUa_P_Socket_Delete(a_pSocket);
                                    }

                                    break;
                                }
                            case 0: /* not successful, shut down controlled */
                            default: /* < 0 fatal error */
                                {
                                    uStatus = OpcUa_Socket_ProcessSslError(pInternalSocket, iResult);
                                    if(OpcUa_IsBad(uStatus))
                                    {
                                        if(OpcUa_IsNotEqual(OpcUa_BadWouldBlock))
                                        {
                                            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_HandleEvent: SSL connect failed.\n");

                                            if(OpcUa_IsEqual(OpcUa_BadSecurityChecksFailed))
                                            {
                                                OpcUa_Socket_FinalizeSslConnect(pInternalSocket, OpcUa_False);
                                            }

                                            if(OPCUA_P_SOCKET_ISSUBSCRIBED(pInternalSocket))
                                            {
                                                pInternalSocket->pfnEventCallback(a_pSocket, OPCUA_SOCKET_EXCEPT_EVENT, pInternalSocket->pvUserData, pInternalSocket->usPort, (OpcUa_Boolean)pInternalSocket->Flags.bSSL);
                                            }
                                            else
                                            {
                                                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_HandleEvent: pfnEventCallback is OpcUa_Null\n");
                                            }

                                            pInternalSocket->eSSLState = OpcUa_P_SSLState_Closed;
                                            OpcUa_P_Socket_Delete(a_pSocket);
                                        }
                                    }
                                    break;
                                }
                            } /* switch */
                        }
                        else /* server */
                        {
                            /* continue server side SSL handshake */
                            int iResult = 0;

                            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_HandleEvent: Continuing SSL accept.\n");
                            iResult = SSL_accept(pInternalSocket->pSSLConnection);

                            switch(iResult)
                            {
                            case 1: /* successfully completed */
                                {
                                    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_HandleEvent: SSL accept complete.\n");
                                    pInternalSocket->bSSLConnected  = OpcUa_True;
                                    pInternalSocket->eSSLState      = OpcUa_P_SSLState_Established;
                                    if(OPCUA_P_SOCKET_ISSUBSCRIBED(pInternalSocket))
                                    {
                                        uStatus = pInternalSocket->pfnEventCallback(a_pSocket, OPCUA_SOCKET_ACCEPT_EVENT, pInternalSocket->pvUserData, pInternalSocket->usPort, (OpcUa_Boolean)pInternalSocket->Flags.bSSL);
                                    }
                                    break;
                                }
                            case 0: /* not successful, shut down controlled */
                            default: /* < 0 fatal error */
                                {
                                    uStatus = OpcUa_Socket_ProcessSslError(pInternalSocket, iResult);
                                    if(OpcUa_IsBad(uStatus))
                                    {
                                        if(OpcUa_IsNotEqual(OpcUa_BadWouldBlock))
                                        {
                                            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_HandleEvent: SSL accept failed.\n");
                                            pInternalSocket->eSSLState = OpcUa_P_SSLState_Closed;
                                            OpcUa_P_Socket_Delete(a_pSocket);
                                        }
                                    }
                                    break;
                                }
                            } /* switch */
                        }
                    }
                    else /* SSL already established */
                    {
                        if(OPCUA_P_SOCKET_ISSUBSCRIBED(pInternalSocket))
                        {
                            /* forwarding read event to socket owner */
                            pInternalSocket->pfnEventCallback(a_pSocket, a_uEvent, pInternalSocket->pvUserData, pInternalSocket->usPort, (OpcUa_Boolean) pInternalSocket->Flags.bSSL);
                        }
                    }
                }
                else /* is no SSL socket */
#endif /* OPCUA_P_SOCKETMANAGER_SUPPORT_SSL */
                {
                    if(OPCUA_P_SOCKET_ISSUBSCRIBED(pInternalSocket))
                    {
                        /* forwarding read event to socket owner */
                        pInternalSocket->pfnEventCallback(a_pSocket, a_uEvent, pInternalSocket->pvUserData, pInternalSocket->usPort, (OpcUa_Boolean) pInternalSocket->Flags.bSSL);
                    }
                    else
                    {
                        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_HandleEvent: pfnEventCallback is OpcUa_Null\n");
                    }
                }
#if OPCUA_MULTITHREADED
				OpcUa_Mutex_Lock(pInternalSocket->pSocketManager->pMutex);
#endif
			}
            else
            {
                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_HandleEvent: unsubscribed socket -> closing socket\n");
                OpcUa_P_Socket_Delete(a_pSocket);
            }
            return OpcUa_Good;
        }
    case OPCUA_SOCKET_WRITE_EVENT:
        {
#if OPCUA_MULTITHREADED
            OpcUa_Mutex_Unlock(pInternalSocket->pSocketManager->pMutex);
#endif
			if(OPCUA_P_SOCKET_ISSUBSCRIBED(pInternalSocket))
            {
                uStatus = pInternalSocket->pfnEventCallback(pInternalSocket, OPCUA_SOCKET_WRITE_EVENT, pInternalSocket->pvUserData, pInternalSocket->usPort, OpcUa_False);
            }
            else
            {
                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_HandleEvent: pfnEventCallback is OpcUa_Null\n");
            }
#if OPCUA_MULTITHREADED
            OpcUa_Mutex_Lock(pInternalSocket->pSocketManager->pMutex);
#endif

            if(uStatus != OpcUa_GoodCallAgain)
            {
                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_P_Socket_Write: unregister for write!\n");
                OPCUA_P_CLR_BITS(pInternalSocket->Flags.EventMask, OPCUA_SOCKET_WRITE_EVENT);
                OPCUA_P_SET_BITS(pInternalSocket->Flags.EventMask, OPCUA_SOCKET_READ_EVENT);
            }

            return OpcUa_Good;
        }
    case OPCUA_SOCKET_CONNECT_EVENT:
        {
            {
#if OPCUA_MULTITHREADED
                OpcUa_Mutex hSocketManagerMutex = pInternalSocket->pSocketManager->pMutex;
#endif
                OpcUa_RawSocket_GetLocalInfo(pInternalSocket->rawSocket, OpcUa_Null, &(pInternalSocket->usPort));
#if OPCUA_MULTITHREADED
                OpcUa_Mutex_Unlock(hSocketManagerMutex);
#endif
#if OPCUA_P_SOCKETMANAGER_SUPPORT_SSL
            if(pInternalSocket->Flags.bSSL != OpcUa_False)
            {
                int iResult = 0;

                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_HandleEvent: Initiating SSL connect.\n");

                iResult = SSL_connect(pInternalSocket->pSSLConnection);

                switch(iResult)
                {
                case 1: /* successfully completed */
                    {
                        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_HandleEvent: SSL connect complete.\n");

                        uStatus = OpcUa_Socket_FinalizeSslConnect(pInternalSocket, OpcUa_True);

                        if(OpcUa_IsGood(uStatus))
                        {
                            pInternalSocket->bSSLConnected  = OpcUa_True;
                            pInternalSocket->eSSLState      = OpcUa_P_SSLState_Established;

                            if(OPCUA_P_SOCKET_ISSUBSCRIBED(pInternalSocket))
                            {
                                uStatus = pInternalSocket->pfnEventCallback(a_pSocket, OPCUA_SOCKET_CONNECT_EVENT, pInternalSocket->pvUserData, pInternalSocket->usPort, (OpcUa_Boolean)pInternalSocket->Flags.bSSL);
                            }
                        }
                        else
                        {
                            if(OPCUA_P_SOCKET_ISSUBSCRIBED(pInternalSocket))
                            {
                                uStatus = pInternalSocket->pfnEventCallback(a_pSocket, OPCUA_SOCKET_EXCEPT_EVENT, pInternalSocket->pvUserData, pInternalSocket->usPort, (OpcUa_Boolean)pInternalSocket->Flags.bSSL);
                            }

                            pInternalSocket->eSSLState      = OpcUa_P_SSLState_Closed;
                            OpcUa_P_Socket_Delete(a_pSocket);
                        }
                        break;
                    }
                case 0: /* not successful, shut down controlled */
                default: /* < 0 fatal error */
                    {
                        uStatus = OpcUa_Socket_ProcessSslError(pInternalSocket, iResult);
                        if(OpcUa_IsBad(uStatus))
                        {
                            if(OpcUa_IsNotEqual(OpcUa_BadWouldBlock))
                            {
                                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_HandleEvent: SSL connect failed.\n");

                                if(OPCUA_P_SOCKET_ISSUBSCRIBED(pInternalSocket))
                                {
                                    pInternalSocket->pfnEventCallback(a_pSocket, OPCUA_SOCKET_EXCEPT_EVENT, pInternalSocket->pvUserData, pInternalSocket->usPort, (OpcUa_Boolean)pInternalSocket->Flags.bSSL);
                                }
                                else
                                {
                                    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_HandleEvent: pfnEventCallback is OpcUa_Null\n");
                                }

                                pInternalSocket->eSSLState = OpcUa_P_SSLState_Closed;
                                OpcUa_P_Socket_Delete(a_pSocket);
                            }
                        }
                        break;
                    }
                } /* switch */
            }
            else
#endif /* OPCUA_P_SOCKETMANAGER_SUPPORT_SSL */
			{
                if(OPCUA_P_SOCKET_ISSUBSCRIBED(pInternalSocket))
                {
                    uStatus = pInternalSocket->pfnEventCallback(a_pSocket, OPCUA_SOCKET_CONNECT_EVENT, pInternalSocket->pvUserData, pInternalSocket->usPort, (OpcUa_Boolean)pInternalSocket->Flags.bSSL);
                }
                else
                {
                    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_HandleEvent: pfnEventCallback is OpcUa_Null\n");
                }
			}
#if OPCUA_MULTITHREADED
                OpcUa_Mutex_Lock(hSocketManagerMutex);
#endif
            }

            pInternalSocket->Flags.EventMask &= (~OPCUA_SOCKET_CONNECT_EVENT);

            return OpcUa_Good;
        }
    case OPCUA_SOCKET_CLOSE_EVENT:
        {
            /* OpcUa_Trace("OpcUa_Socket_HandleEvent: OPCUA_SOCKET_CLOSE_EVENT\n"); */
            break;
        }
    case OPCUA_SOCKET_TIMEOUT_EVENT:
        {
            /*OpcUa_Trace("OpcUa_Socket_HandleEvent: OPCUA_SOCKET_TIMEOUT_EVENT\n");*/
            break;
        }
    case OPCUA_SOCKET_EXCEPT_EVENT:
        {
            OpcUa_Int32 lastE = OpcUa_RawSocket_GetLastError(pInternalSocket->rawSocket);
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_HandleEvent: OPCUA_SOCKET_EXCEPT_EVENT: %i\n", lastE);

            break;
        }
    case OPCUA_SOCKET_ACCEPT_EVENT:
        {
            /*OpcUa_Trace("OpcUa_Socket_HandleEvent: OPCUA_SOCKET_ACCEPT_EVENT\n");*/

#if OPCUA_MULTITHREADED

            if(pInternalSocket->pSocketManager->Flags.bSpawnThreadOnAccept != 0)
            {
                OpcUa_RawThread hThread = OpcUa_Null;

                OpcUa_Mutex_Lock(OpcUa_P_Socket_g_ShutdownMutex);
                OpcUa_P_Socket_g_uNuOfClientThreads++;
                OpcUa_Mutex_Unlock(OpcUa_P_Socket_g_ShutdownMutex);

                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_HandleEvent: Spawning Client Connection thread.\n");

	#ifdef _GNUC_
                uStatus = OpcUa_P_Thread_Create(&hThread);
                uStatus = OpcUa_P_Thread_Start( hThread,                                /* handle */
                                                OpcUa_SocketManager_AcceptHandlerThread,/* handler */
                                                (OpcUa_Void*)pInternalSocket);          /* argument */
	#else
		#ifdef WIN32
			hThread = CreateThread( NULL,
						0,
						(LPTHREAD_START_ROUTINE)OpcUa_SocketManager_AcceptHandlerThread,
						pInternalSocket,
						0,
						NULL);
		#endif
	#endif
	#ifdef _GNUC_
                if(OpcUa_IsGood(uStatus)){
                    if(pInternalSocket->hSemaphore != OpcUa_Null)
                    {
                        /* we must wait until the spawned thread handled the accept event */
                        OpcUa_Semaphore_TimedWait(pInternalSocket->hSemaphore,
                                                    OPCUA_INFINITE);
                    }

                    OpcUa_Mutex_Lock(OpcUa_P_Socket_g_ShutdownMutex);
                    OpcUa_P_Socket_g_uNuOfClientThreads--;
                    OpcUa_Mutex_Unlock(OpcUa_P_Socket_g_ShutdownMutex);

                    a_pSocket = OpcUa_Null; /* skip the following code */

                    OpcUa_P_Thread_Delete(&hThread);
                }
                else
                {
                    OpcUa_Mutex_Lock(OpcUa_P_Socket_g_ShutdownMutex);
                    OpcUa_P_Socket_g_uNuOfClientThreads--;
                    OpcUa_Mutex_Unlock(OpcUa_P_Socket_g_ShutdownMutex);

                    if(pInternalSocket->pSocketManager->Flags.bRejectOnThreadFail != 0)
                    {
                        OpcUa_Socket_HandleAcceptEvent(a_pSocket, pAcceptedSocket);
                        a_pSocket = OpcUa_Null; /* skip the following code */
                    }
                }


	#else

		#ifdef WIN32
                if(hThread != NULL)
                {
                    if(pInternalSocket->hSemaphore != OpcUa_Null)
                    {
                        /* we must wait until the spawned thread handled the accept event */
                        OpcUa_Semaphore_TimedWait(pInternalSocket->hSemaphore,
                                                    OPCUA_INFINITE);
                    }

                    OpcUa_Mutex_Lock(OpcUa_P_Socket_g_ShutdownMutex);
                    OpcUa_P_Socket_g_uNuOfClientThreads--;
                    OpcUa_Mutex_Unlock(OpcUa_P_Socket_g_ShutdownMutex);

                    a_pSocket = OpcUa_Null; /* skip the following code */

                    CloseHandle((HANDLE)hThread);
                }
                else
                {
                    OpcUa_Mutex_Lock(OpcUa_P_Socket_g_ShutdownMutex);
                    OpcUa_P_Socket_g_uNuOfClientThreads--;
                    OpcUa_Mutex_Unlock(OpcUa_P_Socket_g_ShutdownMutex);

                    if(pInternalSocket->pSocketManager->Flags.bRejectOnThreadFail != 0)
                    {
                        OpcUa_Socket_HandleAcceptEvent(a_pSocket, pAcceptedSocket);
                        a_pSocket = OpcUa_Null; /* skip the following code */
                    }
                }
		#endif
	#endif
            }
#endif /* OPCUA_MULTITHREADED */
            if(a_pSocket != OpcUa_Null)
            {
                pAcceptedSocket = OpcUa_SocketManager_FindFreeSocket(pInternalSocket->pSocketManager, OpcUa_False);
                OpcUa_GotoErrorIfNull(pAcceptedSocket, OpcUa_BadMaxConnectionsReached);
                OpcUa_Socket_HandleAcceptEvent(a_pSocket, pAcceptedSocket);
#if OPCUA_P_SOCKETMANAGER_SUPPORT_SSL
                if(pInternalSocket->Flags.bSSL != 0)
                {
                    uStatus = OpcUa_Socket_SetSslContext(   pAcceptedSocket,
                                                            pInternalSocket->pServerCertificate, /*a_pServerCertificate*/
                                                            pInternalSocket->pServerPrivateKey, /*a_pServerPrivateKey*/
                                                            pInternalSocket->pPKIConfig, /*a_pPKIConfig*/
                                                            OPCUA_P_SOCKETMANAGER_SERVER_SSL_VERIFICATION,
                                                            TLSv1_server_method());

                    if(OpcUa_IsBad(uStatus))
                    {
                        /* context could not be created */
                        OpcUa_P_Socket_Delete(pAcceptedSocket);
                        uStatus = OpcUa_Good;
                    }
                    else
                    {
                        OpcUa_InternalSocket* pInternalAcceptedSocket = (OpcUa_InternalSocket*)pAcceptedSocket;
                        int iResult = 0;

                        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_HandleEvent: Initiating SSL accept.\n");

                        pInternalAcceptedSocket->bClient = OpcUa_False;

                        iResult = SSL_accept(pInternalAcceptedSocket->pSSLConnection);

                        switch(iResult)
                        {
                        case 1: /* successfully completed */
                            {
                                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_HandleEvent: SSL accept completed immediately.\n");
                                pInternalAcceptedSocket->bSSLConnected  = OpcUa_True;
                                pInternalAcceptedSocket->eSSLState      = OpcUa_P_SSLState_Established;
                                if(OPCUA_P_SOCKET_ISSUBSCRIBED(pInternalSocket))
                                {
                                    uStatus = pInternalSocket->pfnEventCallback(pAcceptedSocket, OPCUA_SOCKET_ACCEPT_EVENT, pInternalSocket->pvUserData, pInternalSocket->usPort, (OpcUa_Boolean)pInternalSocket->Flags.bSSL);
                                }
                                break;
                            }
                        case 0: /* not successful, shut down controlled */
                        default: /* < 0 fatal error */
                            {
                                uStatus = OpcUa_Socket_ProcessSslError(pInternalAcceptedSocket, iResult);
                                if(OpcUa_IsBad(uStatus))
                                {
                                    if(OpcUa_IsNotEqual(OpcUa_BadWouldBlock))
                                    {
                                        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_HandleEvent: SSL accept failed.\n");
                                        pInternalAcceptedSocket->eSSLState = OpcUa_P_SSLState_Closed;
                                        OpcUa_P_Socket_Delete(pAcceptedSocket);
                                    }
                                    else
                                    {
                                        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_INFO, "OpcUa_Socket_HandleEvent: SSL_ERROR_SYSCALL was OpcUa_BadWouldBlock.\n");
                                    }
                                }
                                break;
                            }
                        } /* switch */

                        /* ignore event in further processing */
                        a_uEvent = OPCUA_SOCKET_NO_EVENT;
                    }
                }
#endif
                a_pSocket = pAcceptedSocket;
            }
            break;
        }
    default:
        {
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_HandleEvent: Unknown event!\n");
            break;
        }

    }; /* end of event dispatcher */

    /* begin dispatching of remaining events */
    if(  a_pSocket   != OpcUa_Null
		&&  a_uEvent    != OPCUA_SOCKET_NO_EVENT)
    {
        if(OPCUA_P_SOCKET_ISSUBSCRIBED(pInternalSocket))
        {
#if OPCUA_MULTITHREADED
            OpcUa_Mutex_Unlock(pInternalSocket->pSocketManager->pMutex);
#endif
            pInternalSocket->pfnEventCallback(a_pSocket, a_uEvent, pInternalSocket->pvUserData, pInternalSocket->usPort, (OpcUa_Boolean)pInternalSocket->Flags.bSSL);

#if OPCUA_MULTITHREADED
            OpcUa_Mutex_Lock(pInternalSocket->pSocketManager->pMutex);
            if(a_uEvent == OPCUA_SOCKET_EXCEPT_EVENT)
            {
                OpcUa_P_Socket_Delete(a_pSocket);
            }
#endif
        }
        else
        {
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Socket_HandleEvent: unsubscribed socket -> closing socket\n");
            OpcUa_P_Socket_Delete(a_pSocket);
        }
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * Set the event mask for this socket.
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Socket_SetEventMask( OpcUa_Socket a_pSocket,
                                            OpcUa_UInt32 a_uEventMask)
{
OpcUa_InitializeStatus(OpcUa_Module_Socket, "SetEventMask");

    OpcUa_GotoErrorIfArgumentNull(a_pSocket);
    OpcUa_GotoErrorIfArgumentNull(((OpcUa_InternalSocket*)a_pSocket)->rawSocket);

    ((OpcUa_InternalSocket*)a_pSocket)->Flags.EventMask = (OpcUa_Int)a_uEventMask;

    OpcUa_P_SocketManager_SignalEvent(  ((OpcUa_InternalSocket*)a_pSocket)->pSocketManager,
                                        OPCUA_SOCKET_RENEWLOOP_EVENT,
                                        OpcUa_False);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * Get the currently set event mask for this socket.
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Socket_GetEventMask(
    OpcUa_Socket a_pSocket,
    OpcUa_UInt32* a_pEventMask)
{
OpcUa_InitializeStatus(OpcUa_Module_Socket, "GetEventMask");

    OpcUa_GotoErrorIfArgumentNull(a_pSocket);
    OpcUa_GotoErrorIfArgumentNull(((OpcUa_InternalSocket*)a_pSocket)->rawSocket);

    *a_pEventMask = (OpcUa_UInt32)((OpcUa_InternalSocket*)a_pSocket)->Flags.EventMask;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * Find a free socket in the given list.
 *===========================================================================*/
OpcUa_Socket OpcUa_SocketManager_FindFreeSocket(    OpcUa_SocketManager     a_pSocketManager,
                                                    OpcUa_Boolean           a_bIsSignalSocket)
{
    OpcUa_UInt32                 uIndex       = 0;
    OpcUa_Boolean                bFound       = OpcUa_False;
    OpcUa_InternalSocketManager* pInternalSocketManager  = (OpcUa_InternalSocketManager*)a_pSocketManager;

#if OPCUA_USE_SYNCHRONISATION
    OpcUa_Mutex_Lock(pInternalSocketManager->pMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */

    for(uIndex = 0; uIndex < pInternalSocketManager->uintMaxSockets; uIndex++)
    {
        if(uIndex == 0 && !a_bIsSignalSocket)
        {
            continue;
        }

        if(pInternalSocketManager->pSockets[uIndex].Flags.bSocketIsInUse == OpcUa_False)
        {
#if OPCUA_USE_SYNCHRONISATION
            /*OpcUa_Mutex_Lock(pInternalSocketManager->pSockets[uIndex].pMutex);*/
#endif /* OPCUA_USE_SYNCHRONISATION */

            pInternalSocketManager->pSockets[uIndex].Flags.bSocketIsInUse   = OpcUa_True;
            OPCUA_SOCKET_INVALIDATE_S(pInternalSocketManager->pSockets[uIndex]);
            pInternalSocketManager->pSockets[uIndex].Flags.bIsListenSocket  = OpcUa_False;
            pInternalSocketManager->pSockets[uIndex].Flags.bOwnThread       = OpcUa_False;
            pInternalSocketManager->pSockets[uIndex].Flags.bSSL             = OpcUa_False;
            pInternalSocketManager->pSockets[uIndex].Flags.EventMask        = 0;
            pInternalSocketManager->pSockets[uIndex].Flags.bIsShutDown      = OpcUa_False;
            pInternalSocketManager->pSockets[uIndex].uintTimeout            = 0;
            pInternalSocketManager->pSockets[uIndex].uintLastAccess         = 0;
            pInternalSocketManager->pSockets[uIndex].pvUserData             = OpcUa_Null;
            pInternalSocketManager->pSockets[uIndex].pfnEventCallback       = OpcUa_Null;
            /*pInternalSocketManager->pSockets[uIndex].pSocketManager         = OpcUa_Null;*/ /* That should stay! */
            pInternalSocketManager->pSockets[uIndex].rawSocket              = (OpcUa_RawSocket)OPCUA_P_SOCKET_INVALID;

#ifdef OPCUA_WITH_SSL
            /*a_pSocketManager->pSockets[uintIndex].pSSL                   = OpcUa_Null;*/
#endif /* OPCUA_WITH_SSL */

            bFound = OpcUa_True;

#if OPCUA_USE_SYNCHRONISATION
            /*OpcUa_Mutex_Unlock(pInternalSocketManager->pSockets[uIndex].pMutex);*/
#endif /* OPCUA_USE_SYNCHRONISATION */

            break; /* for loop */
        }
    }

#if OPCUA_USE_SYNCHRONISATION
    OpcUa_Mutex_Unlock(pInternalSocketManager->pMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */

    if(bFound)
    {
        return &(pInternalSocketManager->pSockets[uIndex]);
    }
    else
    {
        return OpcUa_Null;
    }
}

/*============================================================================
 * Create a new socket list
 *===========================================================================*/
OpcUa_StatusCode OpcUa_P_SocketManager_NewSignalSocket(OpcUa_SocketManager a_pSocketManager)
{
    OpcUa_InternalSocket*           pIntSignalSocket = OpcUa_Null;
    OpcUa_InternalSocketManager*    pInternalSocketManager      = (OpcUa_InternalSocketManager*)a_pSocketManager;
    OpcUa_Boolean                   bIPv6                   = OpcUa_False;

OpcUa_InitializeStatus(OpcUa_Module_Socket, "NewSignalSocket");

    OpcUa_GotoErrorIfArgumentNull(a_pSocketManager);

    /* look whether signal socket is there */
    if(pInternalSocketManager->pCookie == OpcUa_Null)
    {
		/* no, create new one */
        pIntSignalSocket = (OpcUa_InternalSocket*)OpcUa_SocketManager_FindFreeSocket(a_pSocketManager, OpcUa_True);

        OpcUa_GotoErrorIfTrue((pIntSignalSocket == OpcUa_Null), OpcUa_BadResourceUnavailable);

        uStatus = OpcUa_RawSocket_Create(&pIntSignalSocket->rawSocket, OpcUa_True, OpcUa_False,&bIPv6);

        if(OpcUa_IsBad(uStatus))
        {
            OPCUA_SOCKET_INVALIDATE(pIntSignalSocket);
            pIntSignalSocket->Flags.bSocketIsInUse = OpcUa_False;
            OpcUa_GotoError;
        }

        pIntSignalSocket->Flags.EventMask =   OPCUA_SOCKET_CLOSE_EVENT
                                            | OPCUA_SOCKET_EXCEPT_EVENT
                                            | OPCUA_SOCKET_TIMEOUT_EVENT;

        OPCUA_SOCKET_SETVALID(pIntSignalSocket);

        pInternalSocketManager->pCookie = (OpcUa_Void*)pIntSignalSocket;
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
* Main socket based server loop.
*===========================================================================*/
OpcUa_StatusCode OpcUa_P_SocketManager_ServeLoopInternal(   OpcUa_SocketManager   a_pSocketManager,
                                                            OpcUa_UInt32          a_msecTimeout,
                                                            OpcUa_Socket          a_pSocket,
                                                            OpcUa_UInt32          a_uEvent,
                                                            OpcUa_UInt32*         a_puEventOccured)
{
    OpcUa_StatusCode                selectStatus            = OpcUa_Good;

#ifdef WIN32
    OpcUa_P_Socket_Array            readFdSet               = {0, {0}};
    OpcUa_P_Socket_Array            writeFdSet              = {0, {0}};
    OpcUa_P_Socket_Array            exceptFdSet             = {0, {0}};
#endif

#ifdef _GNUC_
    OpcUa_P_Socket_Array            readFdSet;
    OpcUa_P_Socket_Array            writeFdSet;
    OpcUa_P_Socket_Array            exceptFdSet;
#endif

    OpcUa_UInt32                    msecInterval            = 0;
    OpcUa_UInt32                    uintSocketEventOccured  = 0;
    OpcUa_UInt32                    uintPreviousEventMask   = 0;
    OpcUa_UInt32                    uintTimeDifference      = 0;
    OpcUa_UInt32                    uintReturnValue         = 0;

    OpcUa_Boolean                   bForcedByTimer          = OpcUa_False;
    /* more of a hack, since we always wait for shutdown, it shouldnt be given as parameter except when run once. */
    OpcUa_Boolean                   bEndloop                = (a_uEvent==OPCUA_SOCKET_SHUTDOWN_EVENT)?OpcUa_True:OpcUa_False;
    OpcUa_Boolean                   bWaitFlagSet            = OpcUa_False;

    OpcUa_TimeVal                   tmLocalTimeout;
    OpcUa_InternalSocket*           pLocalSocket            = OpcUa_Null;
    OpcUa_RawSocket                 RawSocket               = ((OpcUa_InternalSocket*)a_pSocket) ? ((OpcUa_InternalSocket*)a_pSocket)->rawSocket : OpcUa_Null;
    OpcUa_InternalSocketManager*    pInternalSocketManager  = OpcUa_Null;

#ifdef _GNUC_
    OpcUa_UInt32					rsMaxFd					= 0;
#endif


OpcUa_InitializeStatus(OpcUa_Module_Socket, "P_ServeLoop");

    /* cap */
    if(a_msecTimeout > OPCUA_SOCKET_MAXLOOPTIME)
    {
        a_msecTimeout = OPCUA_SOCKET_MAXLOOPTIME;
    }

    /* No Socket List given, use default list */
    if(a_pSocketManager == OpcUa_Null)
    {
        pInternalSocketManager = &OpcUa_Socket_g_SocketManager;
    }
    else
    {
        pInternalSocketManager = (OpcUa_InternalSocketManager*)a_pSocketManager;
    }

#if OPCUA_USE_SYNCHRONISATION
    OpcUa_Mutex_Lock(pInternalSocketManager->pMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */

    /* the serving loop */
    do
    {
        bForcedByTimer  = OpcUa_False;
        msecInterval    = a_msecTimeout;

#if OPCUA_USE_SYNCHRONISATION
        OpcUa_Mutex_Unlock(pInternalSocketManager->pMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */

        /* map msec to timeval */
        tmLocalTimeout.uintSeconds      = a_msecTimeout / 1000;
        tmLocalTimeout.uintMicroSeconds = (OpcUa_UInt32)((a_msecTimeout - tmLocalTimeout.uintSeconds * 1000) * 1000);

        OpcUa_GotoErrorIfBad(uStatus);

        if(uStatus == OpcUa_GoodNonCriticalTimeout)
        {
            /* map timeval to msec */
            msecInterval    = (OpcUa_UInt32)(tmLocalTimeout.uintSeconds * 1000 + tmLocalTimeout.uintMicroSeconds / 1000);

            bForcedByTimer  = OpcUa_True;

            /* obey cap */
            if(msecInterval > OPCUA_SOCKET_MAXLOOPTIME)
            {
                msecInterval = OPCUA_SOCKET_MAXLOOPTIME;
            }

            if(a_msecTimeout != OPCUA_P_SOCKET_INFINITE)
            {
                /* calculate and store next registered timeout */
                a_msecTimeout = a_msecTimeout - msecInterval;
            }
        }


#if OPCUA_USE_SYNCHRONISATION
        OpcUa_Mutex_Lock(pInternalSocketManager->pMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */


        /* check for signal socket */
        if(pInternalSocketManager->pCookie == OpcUa_Null)
        {
            /* we are missing the signal socket; create a new one */
            uStatus = OpcUa_P_SocketManager_NewSignalSocket(pInternalSocketManager);

            if(uStatus != OpcUa_Good)
            {
                goto Error;
            }
        }

        /* clear socket arrays */
        OPCUA_P_SOCKET_ARRAY_ZERO(&readFdSet);
        OPCUA_P_SOCKET_ARRAY_ZERO(&writeFdSet) ;
        OPCUA_P_SOCKET_ARRAY_ZERO(&exceptFdSet);


        /* fill fdsets with the sockets from the SocketManager */
        OpcUa_P_Socket_FillFdSet(pInternalSocketManager, &readFdSet,   OPCUA_SOCKET_READ_EVENT);
        OpcUa_P_Socket_FillFdSet(pInternalSocketManager, &writeFdSet, (OPCUA_SOCKET_WRITE_EVENT | OPCUA_SOCKET_CONNECT_EVENT));
        OpcUa_P_Socket_FillFdSet(pInternalSocketManager, &exceptFdSet, OPCUA_SOCKET_EXCEPT_EVENT);


        /* check for errors */
        if(     (readFdSet.uintNbSockets    == 0)
            &&  (writeFdSet.uintNbSockets   == 0)
            &&  (exceptFdSet.uintNbSockets  </*=*/ 1)) /* always contains the Signal Socket */
        {
            /* no valid socket in list -> exit list handling */
            uStatus = OpcUa_BadNotFound;

            /* in multithreading, we may have certain conditions when the loop is started before a user socket */
            /* is added to the list. It would not be ok to bail out in this case. */
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_P_SocketManager_ServeLoop: socket list is empty!\n");
            goto Error;
        }

        /* check for external events (1) */
        /* right after possible loop reentry delay */
        uStatus = OpcUa_P_Socket_HandleExternalEvent(pInternalSocketManager, a_uEvent, a_puEventOccured);
        OpcUa_GotoErrorIfBad(uStatus);

        /* leave if a shutdown event was signalled */
        if(OpcUa_IsEqual(OpcUa_GoodShutdownEvent))
        {
            break;
        }

#if OPCUA_USE_SYNCHRONISATION
        OpcUa_Mutex_Unlock(pInternalSocketManager->pMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */

        /* map msec timeout to timeval */
        /* TODO: This happens far too often; redesign this; use msec until raw select! */
        tmLocalTimeout.uintSeconds      =  (msecInterval / 1000);
        tmLocalTimeout.uintMicroSeconds = ((msecInterval % 1000) * 1000);

#ifdef _GNUC_

        if(rsMaxFd<readFdSet.maxFdInSet)
        {
        	rsMaxFd = readFdSet.maxFdInSet;
        }

        if(rsMaxFd<writeFdSet.maxFdInSet)
        {
        	rsMaxFd = writeFdSet.maxFdInSet;
        }

        if(rsMaxFd<exceptFdSet.maxFdInSet)
        {
        	rsMaxFd = exceptFdSet.maxFdInSet;
        }
#endif

        /****************************************************************/
        /* This is the only point in the whole engine, where blocking   */
        /* of the current thread is allowed. Else, processing of        */
        /* network events is slowed down!                               */
#if OPCUA_MULTITHREADED
        //TODO MEF  cast arg 1
        selectStatus = OpcUa_RawSocket_Select(
#ifndef _GNUC_
													0,
#else
													(OpcUa_RawSocket)rsMaxFd, /* 0 on win (ignore on win) */
#endif
                                                    &readFdSet,
                                                    &writeFdSet,
                                                    &exceptFdSet,
                                                    &tmLocalTimeout);
#else
        /* if we're here, the processing socketmanager should better be the global one...! */
        /* maybe test this state here */
        /* The provided ST config implements lowres timers via the global socketmanager's select timeout ... yes, it's lame ... */
        /* Thanks to Andy Griffith for the TimeredSelect and the Timer implementation in general. */
        selectStatus = OpcUa_P_Socket_TimeredSelect(
#ifndef _GNUC_
													0,
#else
													(OpcUa_RawSocket)rsMaxFd, /* 0 on win (ignore on win) */
#endif
                                                    &readFdSet,
                                                    &writeFdSet,
                                                    &exceptFdSet,
                                                    &tmLocalTimeout);
#endif
        /*                                                              */
        /****************************************************************/


#if OPCUA_USE_SYNCHRONISATION
        OpcUa_Mutex_Lock(pInternalSocketManager->pMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */

        /* check for external events (2) */
        /* right after possible select delay */
        uStatus = OpcUa_P_Socket_HandleExternalEvent(pInternalSocketManager, a_uEvent, a_puEventOccured);
        OpcUa_GotoErrorIfBad(uStatus);

        /* leave if a shutdown event was signalled */
        if(OpcUa_IsEqual(OpcUa_GoodShutdownEvent))
        {
            break;
        }

        /* handle errors in select, excluding timeout */
        /* "good" errors result from timeout and closeing the signal socket; the rest is bad... */
        if(OpcUa_IsBad(selectStatus) && (selectStatus != OpcUa_BadTimeout))
        {
            /* check for renew event set externally in list */
            if(((pInternalSocketManager->uintLastExternalEvent) & OPCUA_SOCKET_RENEWLOOP_EVENT) != OPCUA_SOCKET_NO_EVENT)
            {
                /* loop has been interrupted externally to restart with the new/changed list */
                continue;
            }

            /* no renew -> error happened in select and is unexpected, stop server */
            uStatus = OpcUa_BadCommunicationError;
            goto Error;
        }


        /* test if waiting socket returned an event we are waiting for */
        if(     a_uEvent  != OPCUA_SOCKET_NO_EVENT
            &&  a_pSocket != OpcUa_Null)
        {
            uintReturnValue = 0;

            if(         (OPCUA_P_SOCKET_ARRAY_ISSET(RawSocket, &readFdSet))
                    &&  (a_uEvent & OPCUA_SOCKET_READ_EVENT))
            {
                uintReturnValue |= OPCUA_SOCKET_READ_EVENT;
            }
            else if(    (OPCUA_P_SOCKET_ARRAY_ISSET(RawSocket, &writeFdSet))
                    &&  (a_uEvent & OPCUA_SOCKET_WRITE_EVENT))
            {
                uintReturnValue |= OPCUA_SOCKET_WRITE_EVENT;
            }
            else if(    (OPCUA_P_SOCKET_ARRAY_ISSET(RawSocket, &exceptFdSet))
                    &&  (a_uEvent & OPCUA_SOCKET_EXCEPT_EVENT))
            {
                uintReturnValue |= OPCUA_SOCKET_EXCEPT_EVENT;
            }

            pLocalSocket = (OpcUa_InternalSocket *)OpcUa_P_Socket_FindSocketEntry(pInternalSocketManager, RawSocket);
            if(pLocalSocket != OpcUa_Null)
            {
                if(uintReturnValue)
                {
                    uintSocketEventOccured = uintReturnValue;
                }

                /* test timeout waiting socket */
                if(pLocalSocket->uintTimeout !=0)
                {
                    /* check for Timeout too */
                    uintTimeDifference = (OpcUa_P_GetTickCount()/1000) - pLocalSocket->uintLastAccess;
                    if(uintTimeDifference > pLocalSocket->uintTimeout)
                    {
                        /* Socket timed out */
                        pLocalSocket->uintLastAccess = OpcUa_P_GetTickCount()/1000;

                        uintSocketEventOccured = OPCUA_SOCKET_TIMEOUT_EVENT;
                    }
                }
            }
            else
            {
                uintSocketEventOccured = OPCUA_SOCKET_EXCEPT_EVENT;
            }

            if(uintSocketEventOccured)
            {
                if (pLocalSocket)
                {
                    if(bWaitFlagSet) /* was an explicit wait (timer) */
                    {
                        OpcUa_P_Socket_ResetWaitingSocketEvent(pLocalSocket, uintPreviousEventMask);
                    }
                }

                if(a_puEventOccured != OpcUa_Null)
                {
                    *a_puEventOccured = uintSocketEventOccured;
                }

                uStatus = OpcUa_GoodCommunicationEvent;
            }
        }

        /* Handle Events by calling the registered callbacks (all sockets except the waiting socket) */
        OpcUa_P_Socket_HandleFdSet(pInternalSocketManager, &exceptFdSet,  OPCUA_SOCKET_EXCEPT_EVENT);
        OpcUa_P_Socket_HandleFdSet(pInternalSocketManager, &readFdSet,    OPCUA_SOCKET_READ_EVENT);
        OpcUa_P_Socket_HandleFdSet(pInternalSocketManager, &writeFdSet,  (OPCUA_SOCKET_WRITE_EVENT | OPCUA_SOCKET_CONNECT_EVENT));

        /* check for external events (3) */
        /* right after possible event handling delay (get the picture...) */
        uStatus = OpcUa_P_Socket_HandleExternalEvent(pInternalSocketManager, a_uEvent, a_puEventOccured);
        OpcUa_GotoErrorIfBad(uStatus);

        /* leave if a shutdown event was signalled */
        if(OpcUa_IsEqual(OpcUa_GoodShutdownEvent))
        {
            break;
        }

        /* check for timeout in select */
        if(     (selectStatus == OpcUa_BadTimeout)
            &&  (!bForcedByTimer))
        {
            if(bWaitFlagSet)
            {
                pLocalSocket = OpcUa_P_Socket_FindSocketEntry(pInternalSocketManager, RawSocket);
                if(pLocalSocket != OpcUa_Null)
                {
                    OpcUa_P_Socket_ResetWaitingSocketEvent(pLocalSocket, uintPreviousEventMask);
                }
            }

            uStatus = OpcUa_BadTimeout;
            goto Error;
        }

    } while(!bEndloop);

#if OPCUA_USE_SYNCHRONISATION
    OpcUa_Mutex_Unlock(pInternalSocketManager->pMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

#if OPCUA_USE_SYNCHRONISATION
    OpcUa_Mutex_Unlock(pInternalSocketManager->pMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */

OpcUa_FinishErrorHandling;
}

/*============================================================================
* Reset the event mask of a waiting socket.
*===========================================================================*/
OpcUa_Boolean OpcUa_P_Socket_ResetWaitingSocketEvent(OpcUa_Socket pSocket, OpcUa_UInt32 uintPreviousEventMask)
{
    if((OpcUa_InternalSocket*)pSocket)
    {
        ((OpcUa_InternalSocket*)pSocket)->Flags.bInternalWait = OpcUa_False;

        if(uintPreviousEventMask)
        {
            ((OpcUa_InternalSocket*)pSocket)->Flags.EventMask = uintPreviousEventMask;
        }

        return OpcUa_True;
    }

    return OpcUa_False;
}

/*============================================================================
* SetWaitingSocketEvent
*===========================================================================*/
OpcUa_Boolean OpcUa_P_Socket_SetWaitingSocketEvent( OpcUa_Socket pSocket,
                                                    OpcUa_UInt32 uintEvent,
                                                    OpcUa_UInt32* puintPreviousEventMask)
{
    if(     (pSocket   != OpcUa_Null)
        &&  (uintEvent != 0))
    {
        ((OpcUa_InternalSocket*)pSocket)->Flags.bInternalWait = OpcUa_True;
        if(uintEvent != ((((OpcUa_InternalSocket*)pSocket)->Flags.EventMask) & (uintEvent)))
        {
            *puintPreviousEventMask = ((OpcUa_InternalSocket*)pSocket)->Flags.EventMask;
            ((OpcUa_InternalSocket*)pSocket)->Flags.EventMask |= uintEvent;
        }

        return OpcUa_True;
    }

    return OpcUa_False;
}

/*============================================================================
* FillFdSet
*===========================================================================*/
OpcUa_StatusCode OpcUa_P_Socket_FillFdSet(  OpcUa_SocketManager     pSocketManager,
                                            OpcUa_P_Socket_Array*   pSocketArray,
                                            OpcUa_UInt32            uintEvent)
{
    OpcUa_UInt32                    uintIndex               = 0;
    OpcUa_UInt32                    uintTempEvent           = 0;
    OpcUa_StatusCode                uStatus                 = OpcUa_BadInternalError;
    OpcUa_InternalSocketManager*    pInternalSocketManager  = (OpcUa_InternalSocketManager*)pSocketManager;

    OPCUA_P_SOCKET_ARRAY_ZERO(pSocketArray);

    OpcUa_ReturnErrorIfArgumentNull(pInternalSocketManager);

    for (uintIndex = 0; uintIndex < pInternalSocketManager->uintMaxSockets; uintIndex++)
    {
        uintTempEvent = uintEvent;

        /* if socket used and valid */
        if(     (pInternalSocketManager->pSockets[uintIndex].Flags.bSocketIsInUse  != OpcUa_False)
            &&  (OPCUA_SOCKET_ISVALID_S(pInternalSocketManager->pSockets[uintIndex])))
        {
            /* is connect event wished by caller? */
            if((uintTempEvent & OPCUA_SOCKET_CONNECT_EVENT) != 0)
            {
                /* and is connect event wished by socket? */
                if(((pInternalSocketManager->pSockets[uintIndex].Flags.EventMask) & OPCUA_SOCKET_CONNECT_EVENT) != 0)
                {
                    /* then set to connect only */
                    uintTempEvent = OPCUA_SOCKET_CONNECT_EVENT;
                }
                else
                {
                    /* else remove connect event */
                    uintTempEvent &= ~ OPCUA_SOCKET_CONNECT_EVENT;
                }
            }

            /* ignore application sockets */
            if(pInternalSocketManager->pSockets[uintIndex].Flags.bFromApplication == OpcUa_False)
            {
                /* if only uintTemp is wished, set the socket in the fd_set */
                if(((pInternalSocketManager->pSockets[uintIndex].Flags.EventMask) & uintTempEvent) == uintTempEvent)
                {
                    OPCUA_P_SOCKET_ARRAY_SET(pInternalSocketManager->pSockets[uintIndex].rawSocket, pSocketArray);
                    uStatus = OpcUa_Good;
                }
            }
        }
    }

    return uStatus;
}

/*============================================================================
* FindSocketEntry
*===========================================================================*/
/* find a socket in the socket list, identified by the raw socket handle. */
OpcUa_Socket OpcUa_P_Socket_FindSocketEntry(   OpcUa_SocketManager pSocketManager,
                                               OpcUa_RawSocket     RawSocket)
{
    OpcUa_StatusCode             uStatus                = OpcUa_Good;
    OpcUa_UInt32                 uintIndex              = 0;
    OpcUa_InternalSocketManager* pInternalSocketManager = pSocketManager;

    OpcUa_GotoErrorIfArgumentNull(pSocketManager);
    OpcUa_GotoErrorIfArgumentNull(RawSocket);

    for(uintIndex = 0; uintIndex < pInternalSocketManager->uintMaxSockets; uintIndex++)
    {
        if(pInternalSocketManager->pSockets[uintIndex].Flags.bSocketIsInUse != OpcUa_False)
        {
            if(pInternalSocketManager->pSockets[uintIndex].rawSocket == RawSocket)
            {
                return &pInternalSocketManager->pSockets[uintIndex];
            }
        }
    }

Error:
    return OpcUa_Null;
}

/*============================================================================
* CreateServer
*===========================================================================*/
/* create a socket and configure it as a server socket */
OpcUa_RawSocket OpcUa_P_Socket_CreateServer(    OpcUa_Int16         Port,
                                                OpcUa_StatusCode*   Status)
{
    OpcUa_StatusCode    uStatus      = OpcUa_Good;
    OpcUa_RawSocket     RawSocket   = (OpcUa_RawSocket)OPCUA_P_SOCKET_INVALID;
    OpcUa_Boolean       bIPv6;

    OpcUa_ReferenceParameter(Port);
#if OPCUA_P_LISTEN_INET6
    bIPv6 = OpcUa_True;
#else
    bIPv6 = OpcUa_False;
#endif


    uStatus = OpcUa_RawSocket_Create(     &RawSocket,
                                            OpcUa_True,     /* Nagle off */
                                            OpcUa_False, &bIPv6);   /* No keep-alive */
    OpcUa_GotoErrorIfBad(uStatus);

    OpcUa_GotoErrorIfTrue((RawSocket == (OpcUa_RawSocket)OPCUA_P_SOCKET_INVALID), OpcUa_BadCommunicationError);

    /* set nonblocking */
    uStatus = OpcUa_RawSocket_SetBlockMode(   RawSocket,
                                                OpcUa_False);

    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = OpcUa_RawSocket_Bind(RawSocket, Port,bIPv6);
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = OpcUa_RawSocket_Listen(RawSocket);
    OpcUa_GotoErrorIfBad(uStatus);

    if(Status != OpcUa_Null)
    {
        *Status = OpcUa_RawSocket_GetLastError(RawSocket);
    }

    return RawSocket;

Error:
    if(Status != OpcUa_Null)
    {
        *Status = uStatus;
    }

    /* ignore errors which may happen, when RawSocket is invalid */
	if(RawSocket != (OpcUa_RawSocket)OPCUA_P_SOCKET_INVALID)
		OpcUa_RawSocket_Close(RawSocket);

    return (OpcUa_RawSocket)OPCUA_P_SOCKET_INVALID;
}

/*============================================================================
* HandleFdSet
*===========================================================================*/
OpcUa_Void OpcUa_P_Socket_HandleFdSet(  OpcUa_SocketManager     a_pSocketManager,
                                        OpcUa_P_Socket_Array*   a_pSocketArray,
                                        OpcUa_UInt32            a_uEvent)
{
    OpcUa_InternalSocketManager*    pInternalSocketManager         = (OpcUa_InternalSocketManager*)a_pSocketManager;
    OpcUa_UInt32                    uintIndex           = 0;
    OpcUa_UInt32                    uintLocalEvent      = 0;
    OpcUa_UInt32                    uintTimeDifference  = 0; /* seconds */


    if(pInternalSocketManager == OpcUa_Null)
    {
        return;
    }

    for(uintIndex = 0; uintIndex < pInternalSocketManager->uintMaxSockets; uintIndex++)
    {
        uintLocalEvent = a_uEvent;

        if (    (pInternalSocketManager->pSockets[uintIndex].Flags.bSocketIsInUse  != OpcUa_False)
            &&  (OPCUA_SOCKET_ISVALID_S(pInternalSocketManager->pSockets[uintIndex]))
            &&  (pInternalSocketManager->pSockets[uintIndex].Flags.bInternalWait   == OpcUa_False))
        {
            if(OPCUA_P_SOCKET_ARRAY_ISSET(pInternalSocketManager->pSockets[uintIndex].rawSocket, a_pSocketArray))
            {
                if( (uintLocalEvent == OPCUA_SOCKET_READ_EVENT) && (pInternalSocketManager->pSockets[uintIndex].Flags.bIsListenSocket != 0))
                {
                    uintLocalEvent = OPCUA_SOCKET_ACCEPT_EVENT;
                }

                if(0 != (uintLocalEvent & OPCUA_SOCKET_CONNECT_EVENT) )
                {
                    if(0 != ((pInternalSocketManager->pSockets[uintIndex].Flags.EventMask) & OPCUA_SOCKET_CONNECT_EVENT ))
                    {
                        uintLocalEvent = OPCUA_SOCKET_CONNECT_EVENT;
                    }
                    else
                    {
                        uintLocalEvent &=~ OPCUA_SOCKET_CONNECT_EVENT;
                    }
                }

                pInternalSocketManager->pSockets[uintIndex].Flags.bFromApplication = OpcUa_True;
#if 1 /* not sure about processing this further; would need to transport the information along; better access later or store in OpcUa_Socket->LastError? */
                /* the real reason for exception events is received through getsockopt with SO_ERROR */
                if(uintLocalEvent == OPCUA_SOCKET_EXCEPT_EVENT)
                {
                    OpcUa_Int apiResult = 0;
                    OpcUa_Int value     = 0;
                    OpcUa_Int size      = sizeof(value);
                    apiResult = getsockopt((SOCKET)(pInternalSocketManager->pSockets[uintIndex].rawSocket), SOL_SOCKET, SO_ERROR, (char*)&value, (socklen_t *)&size);
                    /* 10061 WSAECONNREFUSED OpcUa_BadSocketConnectionRejected */
                    apiResult = 0;
                }
#endif
                OpcUa_Socket_HandleEvent(&pInternalSocketManager->pSockets[uintIndex], uintLocalEvent);

                pInternalSocketManager->pSockets[uintIndex].Flags.bFromApplication = OpcUa_False;
            }

            if(uintLocalEvent == OPCUA_SOCKET_EXCEPT_EVENT)
            {
                /* Only check timeout, if a timeout value is set for the socket */
                if(pInternalSocketManager->pSockets[uintIndex].uintTimeout != 0)
                {
                    /* check for Timeout too */
                    uintTimeDifference = (OpcUa_P_GetTickCount()/1000) - pInternalSocketManager->pSockets[uintIndex].uintLastAccess;

                    if(uintTimeDifference > pInternalSocketManager->pSockets[uintIndex].uintTimeout)
                    {
                        /* the connection on this socket timed out */
                        pInternalSocketManager->pSockets[uintIndex].uintLastAccess         = OpcUa_P_GetTickCount()/1000;
                        pInternalSocketManager->pSockets[uintIndex].Flags.bFromApplication = OpcUa_True;

                        OpcUa_Socket_HandleEvent(&pInternalSocketManager->pSockets[uintIndex], OPCUA_SOCKET_TIMEOUT_EVENT);

                        pInternalSocketManager->pSockets[uintIndex].Flags.bFromApplication = OpcUa_False;
                    }
                }
            }
        }
    }

    return;
}

/*============================================================================
* HandleExternalEvent
*===========================================================================*/
OpcUa_StatusCode OpcUa_P_Socket_HandleExternalEvent(    OpcUa_SocketManager a_pSocketManager,
                                                        OpcUa_UInt32        a_uEvent,
                                                        OpcUa_UInt32*       a_puEventOccured)
{
    OpcUa_UInt32                 uExternalEvent         = OPCUA_SOCKET_NO_EVENT;
    OpcUa_InternalSocketManager* pInternalSocketManager = (OpcUa_InternalSocketManager*)a_pSocketManager;

OpcUa_InitializeStatus(OpcUa_Module_Socket, "P_HandleExternalEvent");
    OpcUa_GotoErrorIfArgumentNull(a_pSocketManager);

    if(pInternalSocketManager->uintLastExternalEvent != OPCUA_SOCKET_NO_EVENT)
    {
        uExternalEvent = pInternalSocketManager->uintLastExternalEvent;

        /* are we waiting on this certain event */
        if(   (a_puEventOccured             != OpcUa_Null)
           && ((a_uEvent & uExternalEvent)  != 0))
        {
            pInternalSocketManager->uintLastExternalEvent &= ~a_uEvent;
            *a_puEventOccured                        = uExternalEvent & a_uEvent;
            uStatus = OpcUa_GoodCommunicationEvent;
        }

        /* was this the Shutdown Event, raised by the system? */
        if((uExternalEvent & OPCUA_SOCKET_SHUTDOWN_EVENT) != OPCUA_SOCKET_NO_EVENT)
        {
            /* if uStatus !=  */
            uStatus = OpcUa_GoodShutdownEvent;
        }
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
*
*===========================================================================*/
OpcUa_RawSocket OpcUa_P_Socket_CreateClient(    OpcUa_UInt16        a_uPort,
                                                OpcUa_UInt16        a_uRemotePort,
                                                OpcUa_StringA       a_sRemoteAddress,
                                                OpcUa_StatusCode*   a_uStatus)
{

    OpcUa_StatusCode    uStatus      = OpcUa_Good;
    OpcUa_RawSocket     RawSocket   = (OpcUa_RawSocket)OPCUA_P_SOCKET_INVALID;
    OpcUa_Boolean       bIPv6       = OpcUa_False;

    uStatus = OpcUa_RawSocket_Create( &RawSocket,
                                      OpcUa_True,     /* Nagle off        */
                                      OpcUa_False,    /* Keep alive off   */
                                      &bIPv6);   /* IPv4 */
    OpcUa_GotoErrorIfBad(uStatus);

    if(RawSocket == (OpcUa_RawSocket)OPCUA_P_SOCKET_INVALID)
    {
        goto Error;
    }

    /* set nonblocking */
    uStatus = OpcUa_RawSocket_SetBlockMode(RawSocket,OpcUa_False);

    OpcUa_GotoErrorIfBad(uStatus);

    if(a_uPort != (OpcUa_UInt16)0)
    {
		// create a ipV4 binding to the client
        uStatus = OpcUa_RawSocket_Bind(RawSocket, a_uPort, bIPv6);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    if(a_uRemotePort != 0)
    {
        uStatus = OpcUa_RawSocket_Connect(    RawSocket,
                                                a_uRemotePort,
                                                a_sRemoteAddress);
        if(OpcUa_IsBad(uStatus))
        {
            /* we are nonblocking and would block is not an error in this mode */
            if(uStatus != OpcUa_BadWouldBlock)
            {
                goto Error;
            }
            else
            {
                uStatus = OpcUa_Good;
            }
        }
    }

    if(a_uStatus != OpcUa_Null)
    {
        *a_uStatus = uStatus;
    }

    return RawSocket;

Error:

    if(a_uStatus != OpcUa_Null)
    {
        if(OpcUa_IsBad(uStatus))
        {
            *a_uStatus = uStatus;
        }
    }
	if(RawSocket != (OpcUa_RawSocket)OPCUA_P_SOCKET_INVALID)
		OpcUa_RawSocket_Close(RawSocket); /* just in case */
    return (OpcUa_RawSocket)OPCUA_P_SOCKET_INVALID;
}

#if 0
/*============================================================================
 * Wait for a certain event to happen for a maximum of time.
 *===========================================================================*/
/* implicit call to the serveloop, but can be called without knowledge about the
   socketmanager. This stacks the serveloop. used for write all */
OpcUa_StatusCode OpcUa_Socket_WaitForEvent( OpcUa_Socket  a_pSocket,
                                            OpcUa_UInt32  a_uEvent,
                                            OpcUa_UInt32  a_msecTimeout,
                                            OpcUa_UInt32* a_pEventOccured)
{
    OpcUa_InternalSocket*   pIntSock                = (OpcUa_InternalSocket*)a_pSocket;
    OpcUa_Int               bFromApplicationSave    = OpcUa_False;

OpcUa_InitializeStatus(OpcUa_Module_Socket, "WaitForEvent");

    OpcUa_GotoErrorIfArgumentNull(a_pSocket);

    /* save old value */
    bFromApplicationSave = pIntSock->Flags.bFromApplication;

    /* we're stacked */
    pIntSock->Flags.bFromApplication = OpcUa_True;
    uStatus = OpcUa_P_SocketManager_ServeLoopInternal(  pIntSock->pSocketManager,
                                                        a_msecTimeout,
                                                        a_pSocket,
                                                        a_uEvent,
                                                        a_pEventOccured);

    /* restore old value */
    pIntSock->Flags.bFromApplication = OpcUa_False;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}
#endif
