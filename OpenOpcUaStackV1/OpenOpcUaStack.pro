TEMPLATE = lib
#CONFIG += staticlib
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
TARGET = ../lib/linux/OpenOpcUaStack

INCLUDEPATH +=include/shared
##    mettre le path qui va bien a la place ou en plus de /usr/lib ex : -L/home/pascal/OpenOpcUa_V1/OpenOpcUaStackV1
##  j'ai mis /usr/lib car je suis en 32 bits et a titre d'exemple
LIBS *= -L/usr/lib \
               -lpthread \
               -lcrypto \
               -lssl


DEFINES +=\
       _USE_UNIX98 \
       _USE_POSIX199309 \
       _OPCUA_USE_POSIX\
       _GNUC_\
       _DEBUG \
       _BIG_ENDIAN


SOURCES += \
    ./source/opcua_xmlwriter.c \
    ./source/opcua_xmlreader.c \
    ./source/opcua_xmlencoder.c \
    ./source/opcua_xmldecoder.c \
    ./source/opcua_utilities.c \
    ./source/opcua_types.c \
    ./source/opcua_trace.c \
    ./source/opcua_timer.c \
    ./source/opcua_threadpool.c \
    ./source/opcua_thread.c \
    ./source/opcua_tcpstream.c \
    ./source/opcua_tcpsecurechannel.c \
    ./source/opcua_tcplistener_connectionmanager.c \
    ./source/opcua_tcplistener.c \
    ./source/opcua_tcpconnection.c \
    ./source/opcua_stringtable.c \
    ./source/opcua_string.c \
    ./source/opcua_stream.c \
    ./source/opcua_servicetable.c \
    ./source/opcua_serverapi.c \
    ./source/opcua_securestream.c \
    ./source/opcua_securelistener_policymanager.c \
    ./source/opcua_securelistener_channelmanager.c \
    ./source/opcua_securelistener.c \
    ./source/opcua_secureconnection.c \
    ./source/opcua_securechannel.c \
    ./source/opcua_p_wincrypt_random.c \
    ./source/opcua_p_win32_pki.c \
    ./source/opcua_p_utilities.c \
    ./source/opcua_p_trace.c \
    ./source/opcua_p_timer.c \
    ./source/opcua_p_thread.c \
    ./source/opcua_p_string.c \
    ./source/opcua_p_socket_internal.c \
    ./source/opcua_p_socket_interface.c \
    ./source/opcua_p_socket.c \
    ./source/opcua_p_semaphore.c \
    ./source/opcua_p_securitypolicy_none.c \
    ./source/opcua_proxystub.c \
    ./source/opcua_p_pki_nosecurity.c \
    ./source/opcua_p_pkifactory.c \
    ./source/opcua_p_openssl_x509.c \
    ./source/opcua_p_openssl_sha.c \
    ./source/opcua_p_openssl_rsa.c \
    ./source/opcua_p_openssl_random.c \
    ./source/opcua_p_openssl_pki.c \
    ./source/opcua_p_openssl_hmac_sha.c \
    ./source/opcua_p_openssl_ecdsa.c \
    ./source/opcua_p_openssl_aes.c \
    ./source/opcua_p_openssl_3des.c \
    ./source/opcua_p_openssl.c \
    ./source/opcua_p_mutex.c \
    ./source/opcua_p_memory.c \
    ./source/opcua_p_libxml2.c \
    ./source/opcua_p_internal.c \
    ./source/opcua_p_interface.c \
    ./source/opcua_p_guid.c \
    ./source/opcua_p_datetime.c \
    ./source/opcua_p_cryptofactory.c \
    ./source/opcua_p_binary.c \
    ./source/opcua_messagecontext.c \
    ./source/opcua_memorystream.c \
    ./source/opcua_memory.c \
    ./source/opcua_listener.c \
    ./source/opcua_list.c \
    ./source/opcua_httpstream.c \
    ./source/opcua_https_secureconnection.c \
    ./source/opcua_https_connection.c \
    ./source/opcua_httplistener_securitystub.c \
    ./source/opcua_httplistener_connectionmanager.c \
    ./source/opcua_httplistener.c \
    ./source/opcua_http_internal.c \
    ./source/opcua_httpconnection_securityproxy.c \
    ./source/opcua_httpconnection.c \
    ./source/opcua_guid.c \
    ./source/opcua_extensionobject.c \
    ./source/opcua_enumeratedtype.c \
    ./source/opcua_endpoint_ex.c \
    ./source/opcua_endpoint.c \
    ./source/opcua_encoder.c \
    ./source/opcua_encodeableobject.c \
    ./source/opcua_decoder.c \
    ./source/opcua_datetime.c \
    ./source/opcua_crypto.c \
    ./source/opcua_core.c \
    ./source/opcua_connection.c \
    ./source/opcua_clientapi.c \
    ./source/opcua_channel.c \
    ./source/opcua_builtintypes.c \
    ./source/opcua_buffer.c \
    ./source/opcua_binaryencoder.c \
    ./source/opcua_binarydecoder.c \
    ./source/opcua_base64.c \
    ./source/opcua_asynccallstate.c

HEADERS += \
    ./include/shared/resource.h \
    ./include/shared/opcua_xmlwriter.h \
    ./include/shared/opcua_xmlreader.h \
    ./include/shared/opcua_xmlencoderinternal.h \
    ./include/shared/opcua_xmlencoder.h \
    ./include/shared/opcua_xmldefs.h \
    ./include/shared/opcua_utilities.h \
    ./include/shared/opcua_types.h \
    ./include/shared/opcua_trace.h \
    ./include/shared/opcua_timer.h \
    ./include/shared/opcua_threadpool.h \
    ./include/shared/opcua_thread.h \
    ./include/shared/opcua_tcpstream.h \
    ./include/shared/opcua_tcpsecurechannel.h \
    ./include/shared/opcua_tcplistener_connectionmanager.h \
    ./include/shared/opcua_tcplistener.h \
    ./include/shared/opcua_tcpconnection.h \
    ./include/shared/opcua_stringtable.h \
    ./include/shared/opcua_string.h \
    ./include/shared/opcua_stream.h \
    ./include/shared/opcua_statuscodes.h \
    ./include/shared/opcua_stackstatuscodes.h \
    ./include/shared/opcua_socket.h \
    ./include/shared/opcua_soapsecurechannel.h \
    ./include/shared/opcua_servicetable.h \
    ./include/shared/opcua_serverstub.h \
    ./include/shared/opcua_serverapi.h \
    ./include/shared/opcua_semaphore.h \
    ./include/shared/opcua_securestream.h \
    ./include/shared/opcua_securelistener_policymanager.h \
    ./include/shared/opcua_securelistener_channelmanager.h \
    ./include/shared/opcua_securelistener.h \
    ./include/shared/opcua_secureconnection.h \
    ./include/shared/opcua_securechannel_types.h \
    ./include/shared/opcua_securechannel.h \
    ./include/shared/opcua_p_xml.h \
    ./include/shared/opcua_p_wincrypt.h \
    ./include/shared/opcua_p_win32_pki.h \
    ./include/shared/opcua_p_utilities.h \
    ./include/shared/opcua_p_types.h \
    ./include/shared/opcua_p_trace.h \
    ./include/shared/opcua_p_timer.h \
    ./include/shared/opcua_p_thread.h \
    ./include/shared/opcua_p_string.h \
    ./include/shared/opcua_p_socket_internal.h \
    ./include/shared/opcua_p_socket_interface.h \
    ./include/shared/opcua_p_socket.h \
    ./include/shared/opcua_p_semaphore.h \
    ./include/shared/opcua_p_securitypolicy_none.h \
    ./include/shared/opcua_proxystub.h \
    ./include/shared/opcua_p_pki_nosecurity.h \
    ./include/shared/opcua_p_pkifactory.h \
    ./include/shared/opcua_p_pki.h \
    ./include/shared/opcua_p_os.h \
    ./include/shared/opcua_p_openssl_pki.h \
    ./include/shared/opcua_p_openssl.h \
    ./include/shared/opcua_p_mutex.h \
    ./include/shared/opcua_p_memory.h \
    ./include/shared/opcua_p_libxml2.h \
    ./include/shared/opcua_platformdefs.h \
    ./include/shared/opcua_pkifactory.h \
    ./include/shared/opcua_pki.h \
    ./include/shared/opcua_p_internal.h \
    ./include/shared/opcua_p_interface.h \
    ./include/shared/opcua_p_guid.h \
    ./include/shared/opcua_p_datetime.h \
    ./include/shared/opcua_p_cryptofactory.h \
    ./include/shared/opcua_p_crypto.h \
    ./include/shared/opcua_p_compilerinfo.h \
    ./include/shared/opcua_p_binary.h \
    ./include/shared/opcua_mutex.h \
    ./include/shared/opcua_messagecontext.h \
    ./include/shared/opcua_memorystream.h \
    ./include/shared/opcua_memory.h \
    ./include/shared/opcua_listener.h \
    ./include/shared/opcua_list.h \
    ./include/shared/opcua_identifiers.h \
    ./include/shared/opcua_httpstream.h \
    ./include/shared/opcua_https_secureconnection.h \
    ./include/shared/opcua_https_connection.h \
    ./include/shared/opcua_httplistener_securitystub.h \
    ./include/shared/opcua_httplistener_connectionmanager.h \
    ./include/shared/opcua_httplistener.h \
    ./include/shared/opcua_http_internal.h \
    ./include/shared/opcua_httpconnection_securityproxy.h \
    ./include/shared/opcua_httpconnection.h \
    ./include/shared/opcua_guid.h \
    ./include/shared/opcua_extensionobject.h \
    ./include/shared/opcua_exclusions.h \
    ./include/shared/opcua_errorhandling.h \
    ./include/shared/opcua_enumeratedtype.h \
    ./include/shared/opcua_endpoint_internal.h \
    ./include/shared/opcua_endpoint_ex.h \
    ./include/shared/opcua_endpoint.h \
    ./include/shared/opcua_encoder.h \
    ./include/shared/opcua_encodeableobject.h \
    ./include/shared/opcua_decoder.h \
    ./include/shared/opcua_datetime.h \
    ./include/shared/opcua_cryptofactory.h \
    ./include/shared/opcua_crypto.h \
    ./include/shared/opcua_credentials.h \
    ./include/shared/opcua_core.h \
    ./include/shared/opcua_connection.h \
    ./include/shared/opcua_config.h \
    ./include/shared/opcua_clientproxy.h \
    ./include/shared/opcua_clientapi.h \
    ./include/shared/opcua_channel_internal.h \
    ./include/shared/opcua_channel.h \
    ./include/shared/opcua_builtintypes.h \
    ./include/shared/opcua_buffer.h \
    ./include/shared/opcua_browsenames.h \
    ./include/shared/opcua_binaryencoderinternal.h \
    ./include/shared/opcua_binaryencoder.h \
    ./include/shared/opcua_base64.h \
    ./include/shared/opcua_attributes.h \
    ./include/shared/opcua_asynccallstate.h \
    ./include/shared/opcua.h

