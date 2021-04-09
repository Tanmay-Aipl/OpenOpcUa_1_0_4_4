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
#ifndef _Vpi_ProxyStub_H_
#define _Vpi_ProxyStub_H_ 1
VPI_BEGIN_EXTERN_C
typedef struct _Vpi_ProxyStubConfiguration
{
    /** Globally enable/disable trace output from the stack (exclude platformlayer) */
    //Vpi_Boolean   bProxyStub_Trace_Enabled;
	/* configure the output for the trace FILE, CONSOLE or NONE */
	Vpi_UInt32	uProxyStub_Trace_Output;
    /** Configure the level of messages traced. See config.h for values. */
    Vpi_UInt32    uProxyStub_Trace_Level;
	/* Trace_FileName */
	Vpi_String	StrProxyStub_Trace_FileName; // "OpenOpcUaStack.log"

	FILE*			hProxyStub_OutFile; // Handle for the trace file
	Vpi_UInt32	hOutFileNoOfEntriesMax; // Number max of entries to bufferize before write to trace file
	Vpi_UInt32	hOutFileNoOfEntries; // Current number of entries in the buffer
    ///** Security constraints for the serializer. Set this values carefully. */
    ///** The largest size for a memory block the serializer can do when deserializing a message. */
    //Vpi_Int32     iSerializer_MaxAlloc;
    ///** The largest string accepted by the serializer. */
    //Vpi_Int32     iSerializer_MaxStringLength;
    ///** The largest byte string accepted by the serializer. */
    //Vpi_Int32     iSerializer_MaxByteStringLength;
    ///** Maximum number of elements in an array accepted by the serializer. */
    //Vpi_Int32     iSerializer_MaxArrayLength;
    ///** The maximum number of bytes per message in total. */
    //Vpi_Int32     iSerializer_MaxMessageSize;

    ///** Be careful! Enabling the threadpool has severe implications on the behavior of your server! */
    ///** Controls wether the secure listener uses a thread pool to dispatch received requests. */
    //Vpi_Boolean   bSecureListener_ThreadPool_Enabled;
    ///** The minimum number of threads in the thread pool. */
    //Vpi_Int32     iSecureListener_ThreadPool_MinThreads;
    ///** The maximum number of threads in the thread pool */
    //Vpi_Int32     iSecureListener_ThreadPool_MaxThreads;
    ///** The length of the queue with jobs waiting for a free thread. */
    //Vpi_Int32     iSecureListener_ThreadPool_MaxJobs;
    ///** If MaxJobs is reached the add operation can block or return an error. */
    //Vpi_Boolean   bSecureListener_ThreadPool_BlockOnAdd;
    ///** If the add operation blocks on a full job queue, this value sets the max waiting time. */
    //Vpi_UInt32    uSecureListener_ThreadPool_Timeout;

    ///** If true, the TcpListener request a thread per client from the underlying socketmanager. Must not work with all platform layers. */
    //Vpi_Boolean   bTcpListener_ClientThreadsEnabled;
    ///** The default and maximum size for message chunks in the server. Affects network performance and memory usage. */
    //Vpi_Int32     iTcpListener_DefaultChunkSize;

    ///** The default (and requested) size for message chunks. Affects network performance and memory usage. */
    //Vpi_Int32     iTcpConnection_DefaultChunkSize;

    ///** The default and maximum size for messages in the server. Affects memory usage. */
    //Vpi_Int32     iTcpTransport_MaxMessageLength;
    ///** The default and maximum number of message chunks per message in the server. Affects memory usage. */
    //Vpi_Int32     iTcpTransport_MaxChunkCount;

    ///** The network stream should block if not all could be send in one go. Be careful and use this only with client threads. Must not work with all platform layers. */
    //Vpi_Boolean   bTcpStream_ExpectWriteToBlock;
} Vpi_ProxyStubConfiguration;
VPI_END_EXTERN_C
#endif