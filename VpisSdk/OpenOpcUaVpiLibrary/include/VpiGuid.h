//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiQualifiedName.h
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************
#ifndef VPIGUID
#define VPIGUID

VPI_BEGIN_EXTERN_C
VPILIBRARY_EXPORT Vpi_StatusCode Vpi_Guid_CopyTo(
	Vpi_Guid*  pSource,
	Vpi_Guid** ppDestination);
VPILIBRARY_EXPORT
Vpi_Guid* Vpi_Guid_Create(Vpi_Guid* pGuid);

VPI_END_EXTERN_C
#endif