//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiMacroDef.h
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************
// stdafx.h : fichier Include pour les fichiers Include système standard,
// ou les fichiers Include spécifiques aux projets qui sont utilisés fréquemment,
// et sont rarement modifiés
//
// 
#ifndef _Vpi_MacroDef
#define _Vpi_MacroDef 1
/*============================================================================
* The Int32 type
*===========================================================================*/

#define Vpi_Int32_Initialize(xValue) *(xValue) = (Vpi_Int32)0;

#define Vpi_Int32_Clear(xValue) *(xValue) = (Vpi_Int32)0;
/*============================================================================
* Vpi_Field_Initialize
*===========================================================================*/
#define Vpi_Field_Initialize(xType, xName) Vpi_##xType##_Initialize(&a_pValue->xName);
/*============================================================================
* Vpi_Field_Clear
*===========================================================================*/
#define Vpi_Field_Clear(xType, xName) Vpi_##xType##_Clear(&a_pValue->xName);

/*============================================================================
* Vpi_Field_CopyToScalar
*===========================================================================*/
#define Vpi_Field_CopyToScalar(xType, xName) (a_pDestination->xName = a_pSource->xName);

/*============================================================================
* Vpi_Field_CopyTo
*===========================================================================*/
#define Vpi_Field_CopyTo(xType, xName)\
 uStatus = Vpi_##xType##_CopyTo(&(a_pSource->xName), &(a_pDestination->xName));
/*============================================================================
* Vpi_Field_Copy. This one Alloc, initialize and copy
*===========================================================================*/
#define Vpi_Field_Copy(xType, xName) \
{ \
	a_pDestination->xName = (Vpi_##xType*)Vpi_Alloc(sizeof(Vpi_##xType)); \
	if (a_pDestination->xName) \
	{\
		Vpi_##xType##_Initialize(a_pDestination->xName); \
		uStatus=Vpi_Good;\
	}\
	else\
		uStatus=Vpi_BadOutOfMemory;\
	if (uStatus==Vpi_Good)\
		uStatus = Vpi_##xType##_CopyTo(a_pSource->xName, a_pDestination->xName); \
}
#endif