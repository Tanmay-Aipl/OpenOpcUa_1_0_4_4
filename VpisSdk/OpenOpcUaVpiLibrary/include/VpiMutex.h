//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiMutex.h
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************

/* mutex */
VPI_BEGIN_EXTERN_C
VPILIBRARY_EXPORT 
Vpi_StatusCode VPI_DLLCALL Vpi_Mutex_Create (      Vpi_Mutex* phNewMutex);   
VPILIBRARY_EXPORT 
Vpi_Void			VPI_DLLCALL Vpi_Mutex_Delete(      Vpi_Mutex* phMutex);
VPILIBRARY_EXPORT 
Vpi_Void			VPI_DLLCALL Vpi_Mutex_Lock(      Vpi_Mutex  hMutex);   
VPILIBRARY_EXPORT 
Vpi_Void			VPI_DLLCALL Vpi_Mutex_Unlock(      Vpi_Mutex  hMutex);  

VPILIBRARY_EXPORT
Vpi_Int32 VPI_DLLCALL Vpi_InterlockExchange(Vpi_Int32* volatile pTarget, Vpi_Int32 value);

VPI_END_EXTERN_C