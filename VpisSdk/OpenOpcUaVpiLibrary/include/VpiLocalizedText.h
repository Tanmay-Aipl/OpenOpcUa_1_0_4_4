//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiLocalizedText.h
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************
#ifndef VPILOCALIZEDTEXT
#define VPILOCALIZEDTEXT

VPILIBRARY_EXPORT Vpi_Void Vpi_LocalizedText_Initialize(Vpi_LocalizedText* pValue);

VPILIBRARY_EXPORT Vpi_Void Vpi_LocalizedText_Clear(Vpi_LocalizedText* pValue);

VPILIBRARY_EXPORT Vpi_StatusCode Vpi_LocalizedText_CopyTo(const Vpi_LocalizedText* pSource, Vpi_LocalizedText* pDestination);
#endif