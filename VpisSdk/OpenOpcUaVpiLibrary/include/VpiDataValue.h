//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiDataValue.h
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************
#ifndef VPIDATAVALUE
#define VPIDATAVALUE
VPI_BEGIN_EXTERN_C
VPILIBRARY_EXPORT Vpi_StatusCode VPI_DLLCALL Vpi_DataValue_Initialize(Vpi_DataValue* a_pValue);

VPILIBRARY_EXPORT 
Vpi_StatusCode VPI_DLLCALL Vpi_DataValue_Clear(Vpi_DataValue* a_pValue);

VPILIBRARY_EXPORT 
Vpi_StatusCode VPI_DLLCALL Vpi_DataValue_CopyTo(const Vpi_DataValue* pSource, Vpi_DataValue* pDestination);
VPI_END_EXTERN_C

#endif