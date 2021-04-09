//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiExpandedNodeId.h
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************
#ifndef VPIEXPANDEDNODEID
#define VPIEXPANDEDNODEID


VPILIBRARY_EXPORT Vpi_Void Vpi_ExpandedNodeId_Initialize(Vpi_ExpandedNodeId* pValue);

VPILIBRARY_EXPORT Vpi_Void Vpi_ExpandedNodeId_Clear(Vpi_ExpandedNodeId* pValue);

VPILIBRARY_EXPORT Vpi_StatusCode Vpi_ExpandedNodeId_CopyTo(const Vpi_ExpandedNodeId* pSource, Vpi_ExpandedNodeId* pDestination);
#endif