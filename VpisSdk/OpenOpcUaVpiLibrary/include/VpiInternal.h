//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiInternal.h
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************

/* UA platform definitions, base types and configuration */
#include "VpiPlatformdefs.h"

/* platform layer does not support error macro tracing */
#if VPI_TRACE_ERROR_MACROS
#undef VPI_TRACE_ERROR_MACROS
#endif


//#include <opcua_statuscodes.h>
//#include <opcua_stackstatuscodes.h>
//#include <opcua_errorhandling.h>


/* import types for crypto and pki */
//#include <opcua_types.h>
//#include <opcua_crypto.h>
//#include <opcua_pki.h>
/* own */
//#include "opcua_p_interface.h"

/**********************************************************************************/
/*/  Configuration section.                                                      /*/
/**********************************************************************************/

/* @brief Enable if timestamps should be printed in trace outputs. */
#define VPI_P_TRACE_ENABLE_TIME                               VPI_CONFIG_YES

/* @brief Enable file trace outputs. */
#define VPI_P_TRACE_TO_FILE                                   VPI_CONFIG_YES

#if VPI_P_TRACE_TO_FILE
/* @brief Flush file buffer content immediately after each write. Negative impact on performance! */
#define VPI_P_TRACE_FFLUSH_IMMEDIATELY                        VPI_CONFIG_YES
#endif

/** @brief Primary file name and path. */
#define VPI_P_TRACE_G_OUTFILE                                 "OpenOpcUaStack.log"

/** @brief Secondary file name and path. Primary file gets copied to this location
           every VPI_P_TRACE_G_MAX_FILE_ENTRIES. */
#define VPI_P_TRACE_G_OUTFILE_BACKUP                          "OpenOpcUaStackOld.log"

/** @brief Maximum number of trace lines per file. */
#define VPI_P_TRACE_G_MAX_FILE_ENTRIES                        2000

/** @brief Explicitly accept selfsigned certificates if set to VPI_CONFIG_YES. */
#define VPI_P_PKI_ACCEPT_SELFSIGNED_CERTIFICATES              VPI_CONFIG_NO

/** @brief The maximum number of socket managers in multithreading config, supported by the socket module. */
#ifndef VPI_P_SOCKETMANAGER_STATICMAX
# define VPI_P_SOCKETMANAGER_STATICMAX                        60
#endif /* VPI_P_SOCKETMANAGER_STATICMAX */

/** @brief The maximum number of sockets supported by a socket manager. */
#ifndef VPI_P_SOCKETMANAGER_NUMBEROFSOCKETS
# define VPI_P_SOCKETMANAGER_NUMBEROFSOCKETS                  60
#endif /* VPI_P_SOCKETMANAGER_NUMBEROFSOCKETS */

/** @brief Toggle SSL support in the socket manager class. */
#ifndef VPI_P_SOCKETMANAGER_SUPPORT_SSL
# define VPI_P_SOCKETMANAGER_SUPPORT_SSL                      VPI_CONFIG_YES
#endif /* VPI_P_SOCKETMANAGER_SUPPORT_SSL */

/* @brief if SSL_VERIFY_NONE all server certificates will be accepted; else use SSL_VERIFY_PEER */
#define VPI_P_SOCKETMANAGER_CLIENT_SSL_VERIFICATION           SSL_VERIFY_PEER

/** @brief if SSL_VERIFY_NONE, server won't request client certificates; else use SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT */
#define VPI_P_SOCKETMANAGER_SERVER_SSL_VERIFICATION           SSL_VERIFY_NONE

/** @brief Override OpenSSL builtin verification function - not recommended! */
#define VPI_P_SOCKET_SSL_USE_OWN_CERT_VERIFY                  VPI_CONFIG_NO

/** @brief Invoke postverification callback. */
#define VPI_P_SOCKET_USE_SSL_VERIFY_CALLBACK                  VPI_CONFIG_YES

/** @brief Removes the global pointer array OpcUa_P_Socket_g_pSocketManagers which is no longer necessary */
#define VPI_P_SOCKETMANAGER_STATICARRAY                       VPI_CONFIG_NO

/** @brief Endpoints should listen on IPv6 (includes IPv4 on dual stacks) */
#define VPI_P_LISTEN_INET6                                    VPI_CONFIG_NO

/* Create the configuration string for OpcUa_P_GetConfigString(). */
#define VPI_P_CONFIGSTRING    "TraceEnableTime:"VPI_TOSTRING(VPI_P_TRACE_ENABLE_TIME)"\\"\
                                "TraceToFile:"VPI_TOSTRING(VPI_P_TRACE_TO_FILE)"\\"\
                                "TraceMaxFileEntries:"VPI_TOSTRING(VPI_P_TRACE_G_MAX_FILE_ENTRIES)"\\"\
                                "SocketListsStatic:"VPI_TOSTRING(VPI_P_SOCKETMANAGER_STATICARRAY)"\\"\
                                "MaxSocketLists:"VPI_TOSTRING(VPI_P_SOCKETMANAGER_STATICMAX)"\\"\
                                "MaxSocketsPerList:"VPI_TOSTRING(VPI_P_SOCKETMANAGER_NUMBEROFSOCKETS)"\\"\
                                "SupportSSL:"VPI_TOSTRING(VPI_P_SOCKETMANAGER_SUPPORT_SSL)"\\"\
                                "SupportPolicyBasic128Rsa5:"VPI_TOSTRING(VPI_SUPPORT_SECURITYPOLICY_BASIC128RSA15)"\\"\
                                "SupportPolicyBasic256:"VPI_TOSTRING(VPI_SUPPORT_SECURITYPOLICY_BASIC256)"\\"\
                                "SupportPolicyNone:"VPI_TOSTRING(VPI_SUPPORT_SECURITYPOLICY_NONE)"\\"\
                                "ListenIPv6:"VPI_TOSTRING(VPI_P_LISTEN_INET6)"\\"\
                                "SupportPki:"VPI_TOSTRING(VPI_SUPPORT_PKI)

#define VPI_P_SET_BITS(xField, xBitMask)  xField |= xBitMask
#define VPI_P_CLR_BITS(xField, xBitMask)  xField &= ~xBitMask
/** 
 * @brief   Appends information to the version string. 
 * 
 * @param   strVersionType  Optional version type or prefix.
 * @param   strVersionInfo  The string to add to the end of the version string.
 *
 */
//OpcUa_Void OpcUa_P_VersionStringAppend(const OpcUa_CharA* strVersionType,
//                                       const OpcUa_CharA* strVersionInfo);

/**********************************************************************************/
/*/  Neutralize some sideeffects from stack headers.                             /*/
/**********************************************************************************/

/** @brief Maximum wait time for socket module (in Milli sec) at the blocking point. */
#define VPI_SOCKET_MAXLOOPTIME (OpcUa_UInt32)1000 /* reloop after 1 second to be secure against hangs */

/** @brief  errortracing macroes are currently not supported */
#if VPI_TRACE_ERROR_MACROS
    #undef VPI_TRACE_ERROR_MACROS
#endif

/** @brief Imported trace levels from opcua_trace.h */
#define VPI_TRACE_LEVEL_ERROR         0x00000020 /* in-system errors, which require bugfixing        */
#define VPI_TRACE_LEVEL_WARNING       0x00000010 /* in-system warnings and extern errors             */
#define VPI_TRACE_LEVEL_SYSTEM        0x00000008 /* rare system messages (start, stop, connect)      */
#define VPI_TRACE_LEVEL_INFO          0x00000004 /* more detailed information about system events    */
#define VPI_TRACE_LEVEL_DEBUG         0x00000002 /* information needed for debug reasons             */
#define VPI_TRACE_LEVEL_CONTENT       0x00000001 /* all message content                              */

//#ifndef UAANSICWIN32PLATFORMLAYERDLL
//    /* if the platform layer is not linked dynamically, it may use the stacks trace functionality */
//    OpcUa_Void OpcUa_Trace_Imp( OpcUa_UInt32 uTraceLevel, OpcUa_CharA* sFormat, ...);
//    #if VPI_TRACE_ENABLE
//        #define OpcUa_Trace OpcUa_Trace_Imp
//    #else /* VPI_TRACE_ENABLE */
//        #define OpcUa_Trace(xLevel, xFormat, ...)
//    #endif /* VPI_TRACE_ENABLE */
//#else
//    #define OpcUa_Trace(xLevel, xFormat, ...)
//#endif

/**********************************************************************************/
/*/  Trace Modules.                                                              /*/
/**********************************************************************************/
#define OpcUa_Module_P_OpenSSL 0
#define OpcUa_Module_P_CryptoFactory 1
#define OpcUa_Module_P_PKIFactory 2
#define OpcUa_Module_P_WinCrypt 3
#define OpcUa_Module_P_Win32 4
#define OpcUa_Module_P_Libxml2 5

/**********************************************************************************/
/*/  Evaluate Security Config.                                                   /*/
/**********************************************************************************/
/* determine wether OpenSSL is required and set the compiler switch appropriately */
/* DON'T CHANGE THIS MANUALLY, just add new supported policies! */

#if (VPI_SUPPORT_SECURITYPOLICY_BASIC128RSA15 || VPI_SUPPORT_SECURITYPOLICY_BASIC256) && !VPI_REQUIRE_OPENSSL
#  define VPI_REQUIRE_OPENSSL VPI_CONFIG_YES
#endif /* VPI_SUPPORT_SECURITYPOLICY_BASIC128RSA15 || VPI_SUPPORT_SECURITYPOLICY_BASIC256 */

#if VPI_SUPPORT_PKI && !VPI_REQUIRE_OPENSSL
#  define VPI_REQUIRE_OPENSSL VPI_CONFIG_YES
#endif /* VPI_SUPPORT_PKI */

/* if at all, VPI_REQUIRE_OPENSSL is set to VPI_CONFIG_YES before this point. */
#ifndef VPI_REQUIRE_OPENSSL
#define VPI_REQUIRE_OPENSSL VPI_CONFIG_NO
#endif /* VPI_REQUIRE_OPENSSL */

#if VPI_REQUIRE_OPENSSL && !VPI_HAVE_OPENSSL
  # error OpenSSL required; globally #define VPI_HAVE_OPENSSL if OpenSSL is available or disable security!
#endif

/* provide optional SHA2 functionality? not required by UA */
#define VPI_P_SUPPORT_OPENSSL_SHA2 VPI_CONFIG_NO
/**********************************************************************************/
/*/  Internally used function prototypes.                                        /*/
/**********************************************************************************/


/**********************************************************************************/
/*/                                 End Of File.                                 /*/
/**********************************************************************************/
