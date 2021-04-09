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
#ifndef VPIQUALIFIEDNAME
#define VPIQUALIFIEDNAME

VPILIBRARY_EXPORT Vpi_Void Vpi_QualifiedName_Initialize(Vpi_QualifiedName* pValue);

VPILIBRARY_EXPORT Vpi_Void Vpi_QualifiedName_Clear(Vpi_QualifiedName* pValue);

VPILIBRARY_EXPORT Vpi_StatusCode Vpi_QualifiedName_CopyTo(const Vpi_QualifiedName* pSource, Vpi_QualifiedName* pDestination);
#endif