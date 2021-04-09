//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiSemaphore.h
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************
VPI_BEGIN_EXTERN_C

VPILIBRARY_EXPORT
VpiBuiltinType::Vpi_StatusCode VPI_DLLCALL Vpi_Semaphore_Create(Vpi_Semaphore*    phNewSemaphore,
                                                       Vpi_UInt32        uInitalValue,
                                                       Vpi_UInt32        uMaxRange);

VPILIBRARY_EXPORT
VpiBuiltinType::Vpi_StatusCode VPI_DLLCALL Vpi_Semaphore_Delete(Vpi_Semaphore*    phSemaphore);

VPILIBRARY_EXPORT
VpiBuiltinType::Vpi_StatusCode VPI_DLLCALL Vpi_Semaphore_Wait(Vpi_Semaphore     hSemaphore);

VPILIBRARY_EXPORT
VpiBuiltinType::Vpi_StatusCode VPI_DLLCALL Vpi_Semaphore_TimedWait(Vpi_Semaphore     hSemaphore,
                                                         Vpi_UInt32        msecTimeout);
VPILIBRARY_EXPORT
VpiBuiltinType::Vpi_StatusCode VPI_DLLCALL Vpi_Semaphore_Post(Vpi_Semaphore     hSemaphore,
                                                    Vpi_UInt32        uReleaseCount);
VPI_END_EXTERN_C