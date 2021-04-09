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
#ifndef VPINODEID
#define VPINODEID
VPI_BEGIN_EXTERN_C
VPILIBRARY_EXPORT Vpi_Void VPI_DLLCALL Vpi_NodeId_Initialize(Vpi_NodeId* pValue);

VPILIBRARY_EXPORT Vpi_Void VPI_DLLCALL Vpi_NodeId_Clear(Vpi_NodeId* pValue);

VPILIBRARY_EXPORT Vpi_StatusCode VPI_DLLCALL Vpi_NodeId_CopyTo(const Vpi_NodeId* pSource, Vpi_NodeId* pDestination);

VPI_END_EXTERN_C
#endif