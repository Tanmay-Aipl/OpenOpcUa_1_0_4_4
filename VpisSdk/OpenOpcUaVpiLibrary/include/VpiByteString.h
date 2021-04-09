//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiByteString.cpp
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************
#ifndef VPIBYTESTRING
#define VPIBYTESTRING
extern "C"
{
	VPILIBRARY_EXPORT Vpi_Void VPI_DLLCALL Vpi_ByteString_Initialize(Vpi_ByteString* a_pValue);
	VPILIBRARY_EXPORT Vpi_Void VPI_DLLCALL Vpi_ByteString_Clear(Vpi_ByteString* a_pValue);
	VPILIBRARY_EXPORT Vpi_StatusCode VPI_DLLCALL Vpi_ByteString_CopyTo(const Vpi_ByteString* source, Vpi_ByteString* destination);
}
#endif