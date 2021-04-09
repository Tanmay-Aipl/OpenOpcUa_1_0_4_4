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

/* UA platform definitions, base types and configuration */
#include "opcua_platformdefs.h"

/* platform layer does not support error macro tracing */
#if OPCUA_TRACE_ERROR_MACROS
#undef OPCUA_TRACE_ERROR_MACROS
#endif


#include <opcua_statuscodes.h>
#include <opcua_stackstatuscodes.h>
#include <opcua_errorhandling.h>


/* import types for crypto and pki */
#include <opcua_types.h>
#include <opcua_crypto.h>
#include <opcua_pki.h>
/* own */
#include "opcua_p_interface.h"

/**********************************************************************************/
/*/  Configuration section.                                                      /*/
/**********************************************************************************/

/* @brief Enable if timestamps should be printed in trace outputs. */
#define OPCUA_P_TRACE_ENABLE_TIME                               OPCUA_CONFIG_YES

/* @brief Enable file trace outputs. */
#define OPCUA_P_TRACE_TO_FILE                                   OPCUA_CONFIG_YES

#if OPCUA_P_TRACE_TO_FILE
/* @brief Flush file buffer content immediately after each write. Negative impact on performance! */
#define OPCUA_P_TRACE_FFLUSH_IMMEDIATELY                        OPCUA_CONFIG_YES
#endif

/** @brief Primary file name and path. */
#define OPCUA_P_TRACE_G_OUTFILE                                 "OpenOpcUaStack.log"

/** @brief Secondary file name and path. Primary file gets copied to this location
           every OPCUA_P_TRACE_G_MAX_FILE_ENTRIES. */
#define OPCUA_P_TRACE_G_OUTFILE_BACKUP                          "OpenOpcUaStackOld.log"

/** @brief Maximum number of trace lines per file. */
#define OPCUA_P_TRACE_G_MAX_FILE_ENTRIES                        2000

/** @brief Explicitly accept selfsigned certificates if set to OPCUA_CONFIG_YES. */
#define OPCUA_P_PKI_ACCEPT_SELFSIGNED_CERTIFICATES              OPCUA_CONFIG_NO

/** @brief The maximum number of socket managers in multithreading config, supported by the socket module. */
#ifndef OPCUA_P_SOCKETMANAGER_STATICMAX
# define OPCUA_P_SOCKETMANAGER_STATICMAX                        60
#endif /* OPCUA_P_SOCKETMANAGER_STATICMAX */

/** @brief The maximum number of sockets supported by a socket manager. */
#ifndef OPCUA_P_SOCKETMANAGER_NUMBEROFSOCKETS
# define OPCUA_P_SOCKETMANAGER_NUMBEROFSOCKETS                  60
#endif /* OPCUA_P_SOCKETMANAGER_NUMBEROFSOCKETS */

/** @brief Toggle SSL support in the socket manager class. */
#ifndef OPCUA_P_SOCKETMANAGER_SUPPORT_SSL
# define OPCUA_P_SOCKETMANAGER_SUPPORT_SSL                      OPCUA_CONFIG_YES
#endif /* OPCUA_P_SOCKETMANAGER_SUPPORT_SSL */

/* @brief if SSL_VERIFY_NONE all server certificates will be accepted; else use SSL_VERIFY_PEER */
#define OPCUA_P_SOCKETMANAGER_CLIENT_SSL_VERIFICATION           SSL_VERIFY_PEER

/** @brief if SSL_VERIFY_NONE, server won't request client certificates; else use SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT */
#define OPCUA_P_SOCKETMANAGER_SERVER_SSL_VERIFICATION           SSL_VERIFY_NONE

/** @brief Override OpenSSL builtin verification function - not recommended! */
#define OPCUA_P_SOCKET_SSL_USE_OWN_CERT_VERIFY                  OPCUA_CONFIG_NO

/** @brief Invoke postverification callback. */
#define OPCUA_P_SOCKET_USE_SSL_VERIFY_CALLBACK                  OPCUA_CONFIG_YES

/** @brief Removes the global pointer array OpcUa_P_Socket_g_pSocketManagers which is no longer necessary */
#define OPCUA_P_SOCKETMANAGER_STATICARRAY                       OPCUA_CONFIG_NO

/** @brief Endpoints should listen on IPv6 (includes IPv4 on dual stacks) */
#define OPCUA_P_LISTEN_INET6                                    OPCUA_CONFIG_NO

/* Create the configuration string for OpcUa_P_GetConfigString(). */
#define OPCUA_P_CONFIGSTRING    "TraceEnableTime:"OPCUA_TOSTRING(OPCUA_P_TRACE_ENABLE_TIME)"\\"\
                                "TraceToFile:"OPCUA_TOSTRING(OPCUA_P_TRACE_TO_FILE)"\\"\
                                "TraceMaxFileEntries:"OPCUA_TOSTRING(OPCUA_P_TRACE_G_MAX_FILE_ENTRIES)"\\"\
                                "SocketListsStatic:"OPCUA_TOSTRING(OPCUA_P_SOCKETMANAGER_STATICARRAY)"\\"\
                                "MaxSocketLists:"OPCUA_TOSTRING(OPCUA_P_SOCKETMANAGER_STATICMAX)"\\"\
                                "MaxSocketsPerList:"OPCUA_TOSTRING(OPCUA_P_SOCKETMANAGER_NUMBEROFSOCKETS)"\\"\
                                "SupportSSL:"OPCUA_TOSTRING(OPCUA_P_SOCKETMANAGER_SUPPORT_SSL)"\\"\
                                "SupportPolicyBasic128Rsa5:"OPCUA_TOSTRING(OPCUA_SUPPORT_SECURITYPOLICY_BASIC128RSA15)"\\"\
                                "SupportPolicyBasic256:"OPCUA_TOSTRING(OPCUA_SUPPORT_SECURITYPOLICY_BASIC256)"\\"\
                                "SupportPolicyNone:"OPCUA_TOSTRING(OPCUA_SUPPORT_SECURITYPOLICY_NONE)"\\"\
                                "ListenIPv6:"OPCUA_TOSTRING(OPCUA_P_LISTEN_INET6)"\\"\
                                "SupportPki:"OPCUA_TOSTRING(OPCUA_SUPPORT_PKI)

#define OPCUA_P_SET_BITS(xField, xBitMask)  xField |= xBitMask
#define OPCUA_P_CLR_BITS(xField, xBitMask)  xField &= ~xBitMask
/** 
 * @brief   Appends information to the version string. 
 * 
 * @param   strVersionType  Optional version type or prefix.
 * @param   strVersionInfo  The string to add to the end of the version string.
 *
 */
OpcUa_Void OpcUa_P_VersionStringAppend(const OpcUa_CharA* strVersionType,
                                       const OpcUa_CharA* strVersionInfo);

/**********************************************************************************/
/*/  Neutralize some sideeffects from stack headers.                             /*/
/**********************************************************************************/

/** @brief Maximum wait time for socket module (in Milli sec) at the blocking point. */
#define OPCUA_SOCKET_MAXLOOPTIME (OpcUa_UInt32)1000 /* reloop after 1 second to be secure against hangs */

/** @brief  errortracing macroes are currently not supported */
#if OPCUA_TRACE_ERROR_MACROS
    #undef OPCUA_TRACE_ERROR_MACROS
#endif


//#ifndef UAANSICWIN32PLATFORMLAYERDLL
//    /* if the platform layer is not linked dynamically, it may use the stacks trace functionality */
//    OpcUa_Void OpcUa_Trace_Imp( OpcUa_UInt32 uTraceLevel, OpcUa_CharA* sFormat, ...);
//    #if OPCUA_TRACE_ENABLE
//        #define OpcUa_Trace OpcUa_Trace_Imp
//    #else /* OPCUA_TRACE_ENABLE */
//        #define OpcUa_Trace(xLevel, xFormat, ...)
//    #endif /* OPCUA_TRACE_ENABLE */
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

#if (OPCUA_SUPPORT_SECURITYPOLICY_BASIC128RSA15 || OPCUA_SUPPORT_SECURITYPOLICY_BASIC256) && !OPCUA_REQUIRE_OPENSSL
#  define OPCUA_REQUIRE_OPENSSL OPCUA_CONFIG_YES
#endif /* OPCUA_SUPPORT_SECURITYPOLICY_BASIC128RSA15 || OPCUA_SUPPORT_SECURITYPOLICY_BASIC256 */

#if OPCUA_SUPPORT_PKI && !OPCUA_REQUIRE_OPENSSL
#  define OPCUA_REQUIRE_OPENSSL OPCUA_CONFIG_YES
#endif /* OPCUA_SUPPORT_PKI */

/* if at all, OPCUA_REQUIRE_OPENSSL is set to OPCUA_CONFIG_YES before this point. */
#ifndef OPCUA_REQUIRE_OPENSSL
#define OPCUA_REQUIRE_OPENSSL OPCUA_CONFIG_NO
#endif /* OPCUA_REQUIRE_OPENSSL */

#if OPCUA_REQUIRE_OPENSSL && !OPCUA_HAVE_OPENSSL
  # error OpenSSL required; globally #define OPCUA_HAVE_OPENSSL if OpenSSL is available or disable security!
#endif

/* provide optional SHA2 functionality? not required by UA */
#define OPCUA_P_SUPPORT_OPENSSL_SHA2 OPCUA_CONFIG_NO
/**********************************************************************************/
/*/  Internally used function prototypes.                                        /*/
/**********************************************************************************/


/**********************************************************************************/
/*/                                 End Of File.                                 /*/
/**********************************************************************************/
