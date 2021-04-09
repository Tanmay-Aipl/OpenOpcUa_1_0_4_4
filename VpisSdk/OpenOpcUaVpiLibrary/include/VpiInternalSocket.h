//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiInternalSocket.h
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************

/******************************************************************************************************/
/** @file Internally used definitions and types for the platform layer network implementation         */
/******************************************************************************************************/
#ifndef _Vpi_Socket_Internal_H_
#define _Vpi_Socket_Internal_H_ 1

#if VPI_P_SOCKETMANAGER_SUPPORT_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif /* VPI_P_SOCKETMANAGER_SUPPORT_SSL */

VPI_BEGIN_EXTERN_C
/*============================================================================
 * The Socket Event Callback
 *===========================================================================*/
/** @brief Function prototype for receiving event callbacks from the socket module. */
typedef Vpi_StatusCode (*Vpi_Socket_EventCallback)( Vpi_Socket   hSocket,
                                                        Vpi_UInt32   uintSocketEvent,
                                                        Vpi_Void*    pUserData,
                                                        Vpi_UInt16   usPortNumber,
                                                        Vpi_Boolean  bIsSSL);
/*============================================================================
 * The Socket Type
 *===========================================================================*/

/* forward definition of the Vpi_Socket structure */
typedef struct _Vpi_InternalSocket Vpi_InternalSocket;

/* forward definition of the Vpi_SocketManager structure */
typedef struct _Vpi_InternalSocketManager Vpi_InternalSocketManager;

/* different connection states for SSL connections */
#if VPI_P_SOCKETMANAGER_SUPPORT_SSL
typedef enum _Vpi_P_SSLState
{
    Vpi_P_SSLState_Invalid            = 0,
    Vpi_P_SSLState_WaitForTransport   = 2,
    Vpi_P_SSLState_WantAccept         = 1,
    Vpi_P_SSLState_WantConnect        = 2,
    Vpi_P_SSLState_Established        = 3,
    Vpi_P_SSLState_Shutdown           = 4,
    Vpi_P_SSLState_WantRead           = 5,
    Vpi_P_SSLState_WantWrite          = 6,
    Vpi_P_SSLState_Closed             = 7
} Vpi_P_SSLState;
#endif /* VPI_P_SOCKETMANAGER_SUPPORT_SSL */

/// <summary>
/// Internal representation for a logical socket (client and server). Includes
/// beside the system socket additional information for handling.
/// </summary>
struct _Vpi_InternalSocket
{
    Vpi_RawSocket                  rawSocket;          /* system socket */
    Vpi_Boolean                    bClient;
#if VPI_P_SOCKETMANAGER_SUPPORT_SSL
    Vpi_P_SSLState                 eSSLState;
    SSL*                             pSSLConnection;
    SSL_CTX*                         pSSLContext;
    Vpi_Boolean                    bSSLConnected;
    Vpi_ByteString*                pServerCertificate;
    Vpi_Key*                       pServerPrivateKey;
    Vpi_Void*                      pPKIConfig;
    Vpi_Socket_CertificateCallback pfnCertificateValidation;
#endif /* VPI_P_SOCKETMANAGER_SUPPORT_SSL */
    Vpi_Socket_EventCallback      pfnEventCallback;   /* function to call on event */
    Vpi_Void*                     pvUserData;         /* data for callback */
    Vpi_InternalSocketManager*    pSocketManager;     /* the socket manager, this socket belongs to */
    Vpi_UInt16                    usPort;             /* the socket port of this socket */
    struct _Flags
    {
        Vpi_Int                   EventMask:11;       /* mask and unmask eventhandling */
        Vpi_Int                   bIsListenSocket:1;  /* to distinguish between accept and read events */
        Vpi_Int                   bInvalidSocket:1;   /* is the socket usable */
        Vpi_Int                   bOwnThread:1;       /* if this socket is handled by an own thread */
        Vpi_Int                   bFromApplication:1; /* Application is explicitely waiting for an event on this socket. */
        Vpi_Int                   bSocketIsInUse:1;   /* true if this list member is currently connected or listening */
        Vpi_Int                   bInternalWait:1;    /* shows if this socket in an internal wait */
        Vpi_Int                   bSSL:1;             /* SSL used? */
        Vpi_Int                   bIsShutDown:1;
    } Flags;
    Vpi_UInt32                    uintTimeout;        /* interval until connection is considered timed out */
    Vpi_UInt32                    uintLastAccess;     /* system tick count in seconds when last action on this socket took place */
#if VPI_USE_SYNCHRONISATION
    Vpi_Semaphore                 hSemaphore;         /* synchronize with accept handler thread */
    Vpi_Mutex                     pMutex;             /* critical section; synchronize object access */
#endif /* VPI_USE_SYNCHRONISATION */
};

/**
* List of sockets for one listening socket (included).
*/
struct _Vpi_InternalSocketManager
{
    Vpi_InternalSocket*   pSockets;                 /* the sockets */
    Vpi_UInt32            uintMaxSockets;           /* how many socket entries can this list hold at maximum. Mind the signal socket!  */
    Vpi_Void*             pCookie;                  /* pointer to internal data */
    Vpi_UInt32            uintLastExternalEvent;    /* the last occured event */
#if VPI_MULTITHREADED
    Vpi_RawThread         pThread;                  /* each socket list has its own thread... */
#endif /* VPI_MULTITHREADED */
#if VPI_USE_SYNCHRONISATION
    Vpi_Mutex             pMutex;                   /* ... and therefore its own mutex! */
    Vpi_Semaphore         pShutdownEvent;           /* wait on this semaphore to synchronize on shutdown */
#endif /* VPI_USE_SYNCHRONISATION */
    struct _SocketManagerFlags
    {
        Vpi_Int   bStopServerLoop     :1;         /* set to true to end mainloop thread */
        Vpi_Int   bSpawnThreadOnAccept:1;         /* is a new thread spawned on a new connection accept? */
        Vpi_Int   bRejectOnThreadFail :1;         /* reject an accept when there is no free thread? */
        Vpi_Int   bDontCloseOnExcept  :1;         /* override default closing of a socket on except event */
    } Flags;
};

/*
* Sets a socket to invalid.
*/
#define VPI_SOCKET_INVALIDATE(a)      a->Flags.bInvalidSocket = -1
#define VPI_SOCKET_INVALIDATE_S(a)    a.Flags.bInvalidSocket = -1
#define VPI_SOCKET_SETVALID(a)        a->Flags.bInvalidSocket = 0
#define VPI_SOCKET_SETVALID_S(a)      a.Flags.bInvalidSocket = 0

/**
* Checks wether a socket is valid.
*/
#define VPI_SOCKET_ISVALID(a)         a->Flags.bInvalidSocket == 0
#define VPI_SOCKET_ISVALID_S(a)       a.Flags.bInvalidSocket == 0


/*============================================================================
 * Initialize Socket Type
 *===========================================================================*/
Vpi_Void          Vpi_Socket_Initialize(    Vpi_Socket pSocket);

/*============================================================================
 * Clear Socket Type
 *===========================================================================*/
Vpi_Void          Vpi_Socket_Clear(         Vpi_Socket pSocket);

/*============================================================================
 * Initialize Socket Type
 *===========================================================================*/
Vpi_Void          Vpi_Socket_Delete(        Vpi_Socket* ppSocket);

/*============================================================================
 * Allocate Socket Type
 *===========================================================================*/
Vpi_Socket       Vpi_Socket_Alloc();


/**************************** The SocketManager Type ****************************/

/*============================================================================
 * Allocate SocketManager Type
 *===========================================================================*/
Vpi_SocketManager Vpi_SocketManager_Alloc();

/*============================================================================
 * Initialize SocketManager Type
 *===========================================================================*/
Vpi_Void          Vpi_SocketManager_Initialize(Vpi_SocketManager pSocketManager);



/*============================================================================
 * Create the Sockets in the given list
 *===========================================================================*/
Vpi_StatusCode    Vpi_SocketManager_CreateSockets(  Vpi_SocketManager     pSocketManager,
                                                        Vpi_UInt32            uintMaxSockets);


/*============================================================================
 * Wait for a certain event to happen for a maximum of time.
 *===========================================================================*/
Vpi_StatusCode    Vpi_Socket_WaitForEvent(  Vpi_Socket                pSocket,
                                                Vpi_UInt32                uintEvent,
                                                Vpi_UInt32                msecInterval,
                                                Vpi_UInt32*               puintEventOccured);




/*============================================================================
 * Set the event mask for this socket.
 *===========================================================================*/
Vpi_StatusCode    Vpi_Socket_SetEventMask(  Vpi_Socket                pSocket,
                                                Vpi_UInt32                uintEventMask);

/*============================================================================
 * Get the currently set event mask for this socket.
 *===========================================================================*/
Vpi_StatusCode    Vpi_Socket_GetEventMask(  Vpi_Socket                pSocket,
                                                Vpi_UInt32*               puintEventMask);

/*============================================================================
 * Network Byte Order Conversion Helper Functions
 *===========================================================================*/
Vpi_UInt32        Vpi_Socket_NToHL(         Vpi_UInt32 netLong);
Vpi_UInt16        Vpi_Socket_NToHS(         Vpi_UInt16 netShort);

Vpi_UInt32        Vpi_Socket_HToNL(         Vpi_UInt32 hstLong);
Vpi_UInt16        Vpi_Socket_HToNS(         Vpi_UInt16 hstShort);


/*============================================================================
 * Find a free socket.
 *===========================================================================*/
Vpi_Socket        Vpi_SocketManager_FindFreeSocket( Vpi_SocketManager pSocketManager,
                                                        Vpi_Boolean       bIsSignalSocket);

/*============================================================================
 * Take action based on socket and event.
 *===========================================================================*/
Vpi_StatusCode    Vpi_Socket_HandleEvent(   Vpi_Socket        pSocket,
                                                Vpi_UInt32        uintEvent);

/*!
 * @brief Fill the socket array with sockets from the given socket list and selected based on the given event.
 *
 * @param pSocketManager   [in]    The source of the sockets.
 * @param pSocketArray  [out]   The sockets in this array get set based on the socket list.
 * @param Event         [in]    Only set sockets with this event set.
 *
 * @return A "Good" status code if no error occured, a "Bad" status code otherwise.
 */
//Vpi_StatusCode Vpi_P_Socket_FillFdSet(Vpi_SocketManager   SocketManager,
//                                          Vpi_P_Socket_Array* pSocketArray,
//                                          Vpi_UInt32          Event);


/*!
 * @brief Sets the given Vpi_Socket to waiting.
 *
 * The given event is assigned and the previous event mask stored at the given position.
 *
 * @param pSocket               [in/out]    The target socket for the operation.
 * @param Event                 [in]        Which event should be waited on.
 * @param pPreviousEventMask    [out]       Receives the mask which was valid until now.
 *
 * @return Vpi_True on success, Vpi_False else.
 */
//Vpi_Boolean Vpi_P_Socket_SetWaitingSocketEvent(Vpi_Socket  pSocket,
//                                                   Vpi_UInt32  Event,
//                                                   Vpi_UInt32* pPreviousEventMask);



/*!
 * @brief Handle all signaled events in the socket array.
 *
 * @param pSocketManager   [in]    The list with the Vpi_Sockets which store the handler routines.
 * @param pSocketArray  [in]    The array with the system sockets to be checked.
 * @param Event         [in]    Handle all sockets waiting for this event.
 */
//Vpi_Void Vpi_P_Socket_HandleFdSet(Vpi_SocketManager   SocketManager,
//                                      Vpi_P_Socket_Array* SocketArray,
//                                      Vpi_UInt32          Event);

/*!
 * @brief Handle an externally triggered event.
 *
 * @param pSocketManager   [in]    The current socket list.
 * @param Event         [in]    The awaited event.
 * @param pEventOccured [in]    The event, that occured.
 *
 * @return A "Good" status code if no error occured, a "Bad" status code otherwise.
 */
//Vpi_StatusCode Vpi_P_Socket_HandleExternalEvent(Vpi_SocketManager SocketManager,
//                                                    Vpi_UInt32        Event,
//                                                    Vpi_UInt32*       pEventOccured);

/*!
 * @brief Find the corresponding socket entry in the given socket list.
 *
 * @param pSocketManager [in]    The list to be searched in.
 * @param RawSocket      [in]    The target system socket to search for.
 *
 * @return A pointer to the found Vpi_Socket, or Vpi_Null if the search ended without success.
 */
//Vpi_Socket  Vpi_P_Socket_FindSocketEntry(Vpi_SocketManager SocketManager,
//                                             Vpi_RawSocket     RawSocket);

/*!
 * @brief Is the given system socket marked as set in the specified socket array?
 *
 * @param RawSocket     [in]    The system socket which gets checked.
 * @param pSocketArray  [in]    The file descriptor array to search in.
 *
 * @return Zero if not set, non zero otherwise.
 */
//Vpi_Int32 VPI_P_SOCKET_ARRAY_ISSET_F(Vpi_RawSocket       RawSocket,
//                                         Vpi_P_Socket_Array* pSocketArray);

/*!
 * @brief Sets the given Vpi_Socket to non-waiting and restores the event mask.
 *
 * @param pSocket           [in/out]    The target socket for the operation.
 * @param PreviousEventMask [in]        The mask to restore.
 *
 * @return Vpi_True on success, Vpi_False else.
 */
//Vpi_Boolean Vpi_P_Socket_ResetWaitingSocketEvent(Vpi_Socket  pSocket,
//                                                     Vpi_UInt32  PreviousEventMask);

/*!
 * @brief Create and initialize a listening Vpi_Socket.
 *
 * @param Port      [in]    The port to listen on.
 * @param Status    [out]   How the operation went.
 *
 * @return The created system socket. An invalid socket in case of error.
 */
//Vpi_RawSocket Vpi_P_Socket_CreateServer(Vpi_Int16       Port,
//                                            Vpi_StatusCode* Status);

/*!
 * @brief Create a Vpi_Socket and connect to specified network node.
 *
 * @param Port          [in]    Non zero to bind the socket locally.
 * @param RemotePort    [in]    The port on the server side.
 * @param RemoteAdress  [in]    The IP address of the server as string (ascii).
 * @param Status        [out]   Status how the operation finished.
 *
 * @return The created system socket. An invalid socket in case of error.
 */
//Vpi_RawSocket Vpi_P_Socket_CreateClient(Vpi_UInt16                    Port,
//                                            Vpi_UInt16                    RemotePort,
//                                            Vpi_StringA                   RemoteAddress,
//                                            Vpi_StatusCode*               Status);

/*!
 * @brief Check the socket list for events and handle them.
 *
 * @param pSocketManager [in]    The socket list holding the sockets for the select call.
 * @param msecTimeout    [in]    The maximum number of milliseconds, this function blocks the calling thread.
 * @param Socket         [in]    A specific socket, which waits for a event.
 * @param Event          [in]    The event which the socket is waiting for.
 * @param pEventOccured  [out]   Holds the events that occured during the call.
 *
 * @return A "Good" status code if no error occured, a "Bad" status code otherwise.
 */
//Vpi_StatusCode Vpi_P_SocketManager_ServeLoopInternal(   Vpi_SocketManager   SocketManager,
//                                                            Vpi_UInt32          msecTimeout,
//                                                            Vpi_Socket          Socket,
//                                                            Vpi_UInt32          Event,
//                                                            Vpi_UInt32*         pEventOccured);

/*!
 * @brief Interrupt server loop and signal an event.
 *
 * Unblocks the select function if called from another thread and sets the given event as
 * fired. The event counts as an external event and gets handled through handle external
 * event.
 *
 * @param pSocketManager    [in]    Pointer to the socket list, that should get interrupted.
 * @param Event             [in]    Signal this event with this operation.
 * @param AllManagers       [in]    Should all active managers be interrupted? Ignores first parameter if Vpi_True.
 *
 * @return A "Good" status code if no error occured, a "Bad" status code otherwise.
 */
//Vpi_StatusCode Vpi_P_SocketManager_InterruptLoopInternal(   Vpi_SocketManager     SocketManager,
//                                                                Vpi_UInt32            Event,
//                                                                Vpi_Boolean           bAllManagers);


/*============================================================================
 * Network Byte Order Conversion Helper Functions
 *===========================================================================*/
Vpi_UInt32 Vpi_Socket_NToHL(Vpi_UInt32 a_netLong);

Vpi_UInt16 Vpi_Socket_NToHS(Vpi_UInt16 a_netShort);

Vpi_UInt32 Vpi_Socket_HToNL(Vpi_UInt32 a_hstLong);

Vpi_UInt16 Vpi_Socket_HToNS(Vpi_UInt16 a_hstShort);

/*============================================================================
 * Set socket to nonblocking mode
 *===========================================================================*/
Vpi_StatusCode Vpi_SocketManager_InternalCreateServer(
    Vpi_InternalSocketManager* a_pSocketManager,
    Vpi_UInt16                a_uPort,
    Vpi_ByteString*           a_pServerCertificate,
    Vpi_Key*                  a_pServerPrivateKey,
    Vpi_Void*                 a_pPKIConfig,
    Vpi_Socket_EventCallback  a_pfnSocketCallBack,
    Vpi_Void*                 a_pCallbackData,
    Vpi_Socket*               a_ppSocket,
    Vpi_Boolean               a_bUseSSL);

#if VPI_P_SOCKETMANAGER_SUPPORT_SSL
/*============================================================================
 * Create SSL context for given socket
 *===========================================================================*/
Vpi_StatusCode Vpi_Socket_SetSslContext(                Vpi_InternalSocket*       a_pSocket,
                                                            Vpi_ByteString*           a_pServerCertificate,
                                                            Vpi_Key*                  a_pServerPrivateKey,
                                                            Vpi_Void*                 a_pPKIConfig,
                                                            Vpi_Int                   a_iMode,
                                                            const SSL_METHOD*           a_pSSLMethod);

Vpi_StatusCode Vpi_Socket_ProcessSslError(              Vpi_InternalSocket*       a_pSocket,
                                                            Vpi_Int                   a_iSslError);

#endif /* VPI_P_SOCKETMANAGER_SUPPORT_SSL */

VPI_END_EXTERN_C

#endif /* _Vpi_Socket_Internal_H_ */
