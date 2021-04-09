//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiTypes.h
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************
#pragma once
/**
* @brief Holds a time value with a maximum resolution of micro seconds.
*/
using namespace VpiBuiltinType;
typedef struct _Vpi_TimeVal Vpi_TimeVal;

struct _Vpi_TimeVal
{
	/** @brief The number of full seconds since 1970. */
	Vpi_UInt32 uintSeconds;
	/** @brief The fraction of the last second. */
	Vpi_UInt32 uintMicroSeconds;
};

/** 
  @brief The CryptoKey.
  */
struct Vpi_Crypto_Key_;

//Vpi_BEGIN_EXTERN_C

typedef Vpi_Void (*Vpi_Key_ClearHandle)(struct Vpi_Crypto_Key_* pKey);

struct Vpi_Crypto_Key_
{
	Vpi_UInt              Type;
	Vpi_ByteString        Key;
	Vpi_Key_ClearHandle   fpClearHandle;
};
typedef struct Vpi_Crypto_Key_ Vpi_Key;

/**
 * @brief Vpi_SocketManager Type
 */
typedef Vpi_Void*     Vpi_SocketManager;

/**
 * @brief Vpi_Socket Type
 */
typedef Vpi_Void*     Vpi_Socket;

/**
 * @brief Vpi_Thread Type
 */
typedef Vpi_Void*     Vpi_Thread;

/**
 * @brief The handle for the mutex.
 */
//typedef Vpi_Void*     Vpi_Mutex;

/**
 * @brief The handle for the semaphore.
 */
//typedef Vpi_Void*     Vpi_Semaphore;

/**
 * @brief Internally used thread main entry function.
 */
typedef Vpi_Void      (Vpi_PfnInternalThreadMain)(Vpi_Void* pArguments);
//Vpi_END_EXTERN_C