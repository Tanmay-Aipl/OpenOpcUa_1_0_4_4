//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiNodeId.h
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************
#ifndef VPIMEMORY
#define VPIMEMORY

VPILIBRARY_EXPORT Vpi_Void* VPI_DLLCALL Vpi_Memory_Alloc(Vpi_UInt32 nSize);

VPILIBRARY_EXPORT Vpi_Void VPI_DLLCALL Vpi_Memory_Free(Vpi_Void* a_pBuffer);

VPILIBRARY_EXPORT
Vpi_StatusCode VPI_DLLCALL Vpi_Memory_MemCpy(Vpi_Void*  pBuffer,
	Vpi_UInt32 nSizeInBytes,
	Vpi_Void*  pSource,
	Vpi_UInt32 nCount);

#endif