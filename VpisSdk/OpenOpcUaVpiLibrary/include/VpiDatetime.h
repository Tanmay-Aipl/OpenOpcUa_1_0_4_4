//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiDatetime.h
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************
#ifndef VPI_DATETIME
#define VPI_DATETIME
extern "C"
{
	#define Vpi_DateTime_Clear(xValue) Vpi_MemSet(xValue, 0, sizeof(Vpi_DateTime));
	VPILIBRARY_EXPORT Vpi_StatusCode VPI_DLLCALL Vpi_DateTime_GetStringFromDateTime(Vpi_DateTime  DateTime,
		Vpi_CharA*    pchBuffer,
		Vpi_UInt32    uLength);
	VPILIBRARY_EXPORT Vpi_DateTime VPI_DLLCALL Vpi_DateTime_UtcNow();
}
#endif