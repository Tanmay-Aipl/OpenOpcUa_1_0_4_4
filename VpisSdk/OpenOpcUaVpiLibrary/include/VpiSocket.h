//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  AddItemDlg.cpp
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************
#pragma once
/* HINT: There are currently two socket array/fd_set implementations in this module. */
/*       This is because of debugging and developing reasons.                        */
/** @brief shutdown directions */
#define VPI_SOCKET_SD_RECV  0x00
#define VPI_SOCKET_SD_SEND  0x01
#define VPI_SOCKET_SD_BOTH  0x02

/**
 * These types of events can be sent to the registered callback function from the socket.
 * The receiver can register to them has to react on this events.
 */
#define VPI_SOCKET_NO_EVENT           0x0000 /* no event happened... */
#define VPI_SOCKET_READ_EVENT         0x0001 /* socket ready for receiving */
#define VPI_SOCKET_WRITE_EVENT        0x0002 /* socket ready for writing */
#define VPI_SOCKET_CLOSE_EVENT        0x0004 /* socket has been closed */
#define VPI_SOCKET_EXCEPT_EVENT       0x0008 /* an exception ocurred on a socket */
#define VPI_SOCKET_TIMEOUT_EVENT      0x0010 /* the connection on a socket timed out */
#define VPI_SOCKET_SHUTDOWN_EVENT     0x0020 /* server shuts down */
#define VPI_SOCKET_CONNECT_EVENT      0x0040 /* the socket has connected to the remote node (client) */
#define VPI_SOCKET_ACCEPT_EVENT       0x0080 /* a remote node has connected to this socket (server) */
#define VPI_SOCKET_NEED_BUFFER_EVENT  0x0100 /* the socketmanager requests a temporary buffer */
#define VPI_SOCKET_FREE_BUFFER_EVENT  0x0200 /* the socketmanager releases the temporary buffer */

/** @brief Events which are set outside the event loop. (external events) */
#define VPI_SOCKET_RENEWLOOP_EVENT 0x0300 /* restarts loop to reinterpret socket list */
#define VPI_SOCKET_USER_EVENT      0x0400 /* user fired an event */

/** @brief SocketManager behaviour control. */
#define VPI_SOCKET_NO_FLAG                    0   /* standard behaviour */
#define VPI_SOCKET_REJECT_ON_NO_THREAD        1   /* thread pooling; reject connection if no worker thread i available */
#define VPI_SOCKET_DONT_CLOSE_ON_EXCEPT       2   /* don't close a socket if an except event occured */
#define VPI_SOCKET_SPAWN_THREAD_ON_ACCEPT     4   /* assing each accepted socket a new thread */

/**
* The internally used socket handle. Contains a raw socket in its system
* representation and a flag, indicating wether the socket is usable or not.
*/
typedef Vpi_Void* Vpi_RawSocket; /* pointer to a socket of the platform type */


/*! @brief Number of sockets in a socket array. */
#define VPI_P_SOCKET_ARRAY_SIZE               VPI_P_SOCKETMANAGER_NUMBEROFSOCKETS

/**
* @brief Internal representation for file descriptor sets.
*/
#if defined(WIN32) || defined(_WIN32_WCE)
typedef struct _Vpi_P_Socket_Array
{
    /*! @brief How many sockte entries are _set_. */
    Vpi_UInt uintNbSockets;
    /*! @brief An array of raw (platform) sockets. */
    Vpi_RawSocket SocketArray[VPI_P_SOCKET_ARRAY_SIZE];
} Vpi_P_Socket_Array;
#endif

#ifdef _GNUC_
#include <sys/select.h>
typedef struct _Vpi_P_Socket_Array
{
    /*! @brief How many sockte entries are _set_. */
    Vpi_UInt uintNbSockets;
    /*! @brief An array of raw (platform) sockets. */
    fd_set SocketArray;
    /*! @brief FD maximum on the set */
    Vpi_UInt maxFdInSet;
} Vpi_P_Socket_Array;
#endif


/*! @brief Symbol for stream oriented data transmission. */
#define VPI_P_SOCKET_STREAM                   (1u)

/*! @brief Symbol for packet oriented data transmission. */
#define VPI_P_SOCKET_DGRAM                    (2u)

/*! @brief Set all entries in a socket array to "not set". */
#ifdef WIN32
#define VPI_P_SOCKET_ARRAY_ZERO(pSA)          ((pSA)->uintNbSockets=0)
#endif

#ifdef _GNUC_
#define VPI_P_SOCKET_ARRAY_ZERO(pSA)          {\
	FD_ZERO((fd_set*)&((pSA)->SocketArray)); 	\
	(pSA)->uintNbSockets=0;						\
	(pSA)->maxFdInSet=0;						\
}
#endif

/*! @brief Checks if a specific entry in a socket array is set. */
#define VPI_P_SOCKET_ARRAY_ISSET(rawsck, pSA) Vpi_RawSocket_FD_Isset((Vpi_RawSocket)rawsck, (Vpi_P_Socket_Array*)pSA)

/*! @brief Sets a specific socket in a socket array. */
#ifdef WIN32
#define VPI_P_SOCKET_ARRAY_SET(rawsck, pSA)   { \
    if ((pSA)->uintNbSockets < VPI_P_SOCKET_ARRAY_SIZE) \
        (pSA)->SocketArray[(pSA)->uintNbSockets++]=(Vpi_RawSocket)(rawsck);}
#endif

#ifdef _GNUC_
#define VPI_P_SOCKET_ARRAY_SET(rawsck, pSA) {					\
	if ((pSA)->uintNbSockets < VPI_P_SOCKET_ARRAY_SIZE){ 		\
		FD_SET((int)rawsck,(fd_set*)&((pSA)->SocketArray));		\
		(pSA)->uintNbSockets += 1;								\
		(pSA)->maxFdInSet = (int)rawsck;						\
	}															\
}
#endif

#if 0
/*! @brief Operation would block. */
#define VPI_P_SOCKET_WOULDBLOCK 10035L /* WSAEWOULDBLOCK */

/*! @brief Operation is in progress. */
#define VPI_P_SOCKET_INPROGRESS 10036L /* WSAEINPROGRESS */
#endif

/*! @brief Value for an invalid raw socket. */
#define VPI_P_SOCKET_INVALID      (~0)

/*! @brief Wait for an infinite amount of time. */
#define VPI_P_SOCKET_INFINITE     0xffffffffL     /* wait infinite time */

/*! @brief Value returned by platform API if an error happened. */
#define VPI_P_SOCKET_SOCKETERROR  (-1)            /* platform representation of socket error */

/*============================================================================
 * Functions
 *===========================================================================*/

/*!
 * @brief Initialize the platform network interface. (interface)
 *
 * This function initializes all platform and functional resources used by the
 * p socket module. This includes static variables.
 *
 * @return Good if initialization went fine. A bad status code in any other case.
 */
VPILIBRARY_EXPORT
Vpi_StatusCode VPI_DLLCALL Vpi_RawSocket_InitializeNetwork();


/*!
 * @brief Clean up the platform network interface.
 *
 * Cleans up all data allocated by the p socket module and shuts down the
 * platforms network system as far as possible/needed.
 *
 * @return Good if initialization went fine. A bad status code in any other case.
 */
Vpi_StatusCode Vpi_RawSocket_CleanupNetwork();

/*!
 * @brief Shut a system socket down.
 *
 * @param RawSocket [in] The system socket to be shut down.
 * @param iHow      [in] Which direction (see VPI_P_SOCKET_SD_*).
 *
 * @return A "Good" status code if no error occured, a "Bad" status code otherwise.
 */
Vpi_StatusCode Vpi_RawSocket_Shutdown(Vpi_RawSocket a_RawSocket,Vpi_Int a_iHow);

/*!
 * @brief Close a system socket.
 *
 * @param RawSocket [in] The system socket to be closed.
 *
 * @return A "Good" status code if no error occured, a "Bad" status code otherwise.
 */
VPILIBRARY_EXPORT
Vpi_StatusCode Vpi_RawSocket_Close(Vpi_RawSocket RawSocket);



/*!
 * @brief Connect the given system socket to the specified address.
 *
 * @param RawSocket [in]    The socket to connect.
 * @param Port      [in]    To which port should be connected.
 * @param Host      [in]    The server IP address in string representation.
 *
 * @return A "Good" status code if no error occured, a "Bad" status code otherwise.
 *         Vpi_BadWouldBlock should not be handled as error, if non blocking sockets are used.
 */
Vpi_StatusCode Vpi_RawSocket_Connect( Vpi_RawSocket RawSocket,
											Vpi_Int16     Port,
											Vpi_StringA   Host);

/*!
 * @brief Bind the giben system socket to the specified address.
 *
 * @param RawSocket [in]    The socket to bind.
 * @param Port      [in]    Bind the socket to this port.
 *
 * @return A "Good" status code if no error occured, a "Bad" status code otherwise.
 */
Vpi_StatusCode Vpi_RawSocket_Bind(Vpi_RawSocket RawSocket,
                                        Vpi_Int16     Port,
										Vpi_Boolean a_bIPv6);

/*!
 * @brief Start listening on the given system socket.
 *
 * @param RawSocket [in]    The server socket, which should accept connect requests.
 *
 * @return A "Good" status code if no error occured, a "Bad" status code otherwise.
 */
Vpi_StatusCode Vpi_RawSocket_Listen(Vpi_RawSocket RawSocket);

/*!
 * @brief Accept connections on the given system socket.
 *
 * Accepts a connection from another network node and returns the
 * port and address as well as the newly created system socket
 * for the connection.
 *
 * @param RawSocket     [in]    Accept connection requests on this socket.
 * @param Port          [out]   Will store the port of accepted connection.
 * @param Address       [out]   Will store the address of the connected client.
 * @param NagleOff      [in]    If the new connection has Nagling activated.
 * @param KeepAliveOn   [in]    If the new connection sends keep alive pakets.
 *
 * @return The created system socket. An invalid socket (VPI_P_SOCKET_INVALID) in case of error.
 */
Vpi_RawSocket Vpi_RawSocket_Accept(   Vpi_RawSocket RawSocket,
											Vpi_UInt16*   Port,
											Vpi_UInt32*   Address,
											Vpi_Boolean   NagleOff,
											Vpi_Boolean   KeepAliveOn);

/*!
 * @brief Read data from the given system socket into a buffer.
 *
 * Tries to read data from the given system socket into the buffer.
 * The amount of the received data will fit into the given buffersize.
 *
 * @param RawSocket     [in]        The socket to read from.
 * @param Buffer        [in/out]    Will receive the read data.
 * @param BufferSize    [in]        Maximum number of bytes to receive.
 *
 * @return The number of bytes read, VPI_P_SOCKET_SOCKETERROR or 0 if the peer disconnected.
 */
VPILIBRARY_EXPORT
Vpi_Int32 VPI_DLLCALL  Vpi_RawSocket_Read( Vpi_RawSocket RawSocket,
                                    Vpi_Byte*     Buffer,
                                    Vpi_UInt32    BufferSize,
									int flags);

/*!
 * @brief Write data over the given system socket.
 *
 * Send the given data in the buffer and size over the system socket.
 *
 * @param RawSocket     [in]    Socket to send the data over.
 * @param Buffer        [in]    Buffer holding the data to be sent.
 * @param BufferSize    [in]    How much data should be sent, in bytes.
 *
 * @return The number of bytes written, a VPI_P_SOCKET_SOCKETERROR
 */
#ifdef _GNUC_
Vpi_UInt32 Vpi_RawSocket_Write(Vpi_RawSocket RawSocket,
                                    Vpi_Byte*     Buffer,
                                    Vpi_UInt32    BufferSize);
#else
Vpi_Int32 VPI_DLLCALL  Vpi_RawSocket_Write(Vpi_RawSocket RawSocket,
									Vpi_Byte*     Buffer,
									Vpi_UInt32    BufferSize);
#endif
VPILIBRARY_EXPORT
Vpi_StatusCode VPI_DLLCALL Vpi_RawSocket_SetTimeout(Vpi_RawSocket a_RawSocket, Vpi_Int32 nTimeOut);
/*!
 * @brief Set the system socket to non-blocking or mode.
 *
 * @param RawSocket [in]    The system socket descriptor.
 * @param bBlocking [in]    Socket will be set to block, if true, nonblocking else.
 *
 * @return A "Good" status code if no error occured, a "Bad" status code otherwise.
 */
VPILIBRARY_EXPORT
Vpi_StatusCode VPI_DLLCALL  VPI_DLLCALL Vpi_RawSocket_SetBlockMode(    Vpi_RawSocket RawSocket,
													Vpi_Boolean   bBlocking);

/*!
 * @brief Convert the given value from network into host byte order.
 *
 * @param netLong   [in]    Value to be converted.
 *
 * @return The converted value.
 */
Vpi_UInt32 Vpi_RawSocket_NToHL(Vpi_UInt32 netLong);

/*!
 * @brief Convert the given value from network into host byte order.
 *
 * @param netShort  [in]    Value to be converted.
 *
 * @return The converted value.
 */
Vpi_UInt16 Vpi_RawSocket_NToHS(Vpi_UInt16 netShort);

/*!
 * @brief Convert the given value from host into network byte order.
 *
 * @param hstLong   [in]    Value to be converted.
 *
 * @return The converted value.
 */
Vpi_UInt32 Vpi_RawSocket_HToNL(Vpi_UInt32 hstLong);

/*!
 * @brief Convert the given value from host into network byte order.
 *
 * @param hstShort  [in]    Value to be converted.
 *
 * @return The converted value.
 */
Vpi_UInt16 Vpi_RawSocket_HToNS(Vpi_UInt16 hstShort);




/*!
 * @brief Set sockets in given file descriptor sets if a certain event occured.
 *
 * @param RawSocket         [in]        The maximum file descriptor. Ignored in win32.
 * @param FdSetRead         [in/out]    Vpi_Socket_FdSet with all sockets set, that wait on read events. Holds all sockets where this event occured afterwards.
 * @param FdSetWrite        [in/out]    Vpi_Socket_FdSet with all sockets set, that wait on write events. Holds all sockets where this event occured afterwards.
 * @param FdSetException    [in/out]    Vpi_Socket_FdSet with all sockets set, that wait on exception events. Holds all sockets where this event occured afterwards.
 * @param Timeout           [in]        The maximume time to block at this call.
 *
 * @return A "Good" status code if no error occured, a "Bad" status code otherwise.
 */
VPILIBRARY_EXPORT
Vpi_StatusCode Vpi_RawSocket_Select(  Vpi_RawSocket         RawSocket,
											Vpi_P_Socket_Array*   FdSetRead,
											Vpi_P_Socket_Array*   FdSetWrite,
											Vpi_P_Socket_Array*   FdSetException,
											Vpi_TimeVal*          Timeout);


#if VPI_P_SOCKETGETPEERINFO_V2
/*!
 * @brief Get address information for the peer connected to the given socket socket handle.
 *
 * @param RawSocket             [in] Identifier for the connection.
 * @param achPeerInfoBuffer     [in] Where the address information gets stored.
 * @param uiPeerInfoBufferSize  [in] The size of the given buffer.
 *
 * @return A "Good" status code if no error occured, a "Bad" status code otherwise.
 */
Vpi_StatusCode Vpi_RawSocket_GetPeerInfo( Vpi_Socket    RawSocket,
												Vpi_CharA*    achPeerInfoBuffer,
												Vpi_UInt32    uiPeerInfoBufferSize);
#else

/*!
 * @brief Get IP address and port number of the connected network node.
 *
 * @param RawSocket [in]    Identifier for the connection.
 * @param IP        [out]   Where the IP gets stored.
 * @param Port      [out]   Where the port gets stored.
 *
 * @return A "Good" status code if no error occured, a "Bad" status code otherwise.
 */
VPILIBRARY_EXPORT
Vpi_StatusCode Vpi_RawSocket_GetPeerInfo( Vpi_RawSocket RawSocket,
												Vpi_UInt32*   IP,
												Vpi_UInt16*   Port);
#endif

/*!
 * @brief Get IP address and port number of the local network node.
 *
 * @param RawSocket [in]    Identifier for the connection.
 * @param IP        [out]   Where the IP gets stored.
 * @param Port      [out]   Where the port gets stored.
 *
 * @return A "Good" status code if no error occured, a "Bad" status code otherwise.
 */
Vpi_StatusCode Vpi_RawSocket_GetLocalInfo(Vpi_RawSocket RawSocket,
												Vpi_UInt32*   IP,
												Vpi_UInt16*   Port);

/*!
 * @brief Get last socket error that happened.
 *
 * This function is currently mostly for debugging reasons, since no real error
 * mapping exists, yet (and probably won't).
 *
 * @param RawSocket [in]    Identifier for the connection, which error code is requested.
 *
 * @return The raw error value.
 */
Vpi_Int32 Vpi_RawSocket_GetLastError(Vpi_RawSocket RawSocket);

/*!
 * @brief Create a system socket.
 *
 * @param pRawSocket    [out]   Pointer to a system socket to store the new one.
 * @param NagleOff      [in]    Switch for Nagle algorithm.
 * @param KeepAliveOn   [in]    Switch for TCP keep alive packet.
 *
 * @return A "Good" status code if no error occured, a "Bad" status code otherwise.
 */
Vpi_StatusCode Vpi_RawSocket_Create(  Vpi_RawSocket* pRawSocket,
											Vpi_Boolean    NagleOff,
											Vpi_Boolean    KeepAliveOn,
                                            Vpi_Boolean*   pbIPv6);


Vpi_Boolean Vpi_RawSocket_FD_Isset(   Vpi_RawSocket         a_RawSocket,
											Vpi_P_Socket_Array*   a_pFdSet);


Vpi_UInt32 Vpi_RawSocket_InetAddr(    Vpi_StringA   sRemoteAddress);
