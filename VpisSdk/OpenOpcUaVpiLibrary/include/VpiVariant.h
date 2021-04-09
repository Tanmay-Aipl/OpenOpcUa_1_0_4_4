//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************

#ifndef _Vpi_Variant_H_
#define _Vpi_Variant_H_ 1

VPI_BEGIN_EXTERN_C


VPILIBRARY_EXPORT
Vpi_StatusCode VPI_DLLCALL Vpi_Variant_Initialize( Vpi_Variant* pVariant);

VPILIBRARY_EXPORT
Vpi_StatusCode VPI_DLLCALL Vpi_Variant_Clear(Vpi_Variant* pVariant);

VPILIBRARY_EXPORT 
Vpi_StatusCode VPI_DLLCALL Vpi_Variant_CopyTo(const Vpi_Variant* pSource, Vpi_Variant* pDestination);
VPI_END_EXTERN_C
// Not exported
Vpi_Void Vpi_VariantUnion_Clear(Vpi_UInt16 datatype, Vpi_VariantUnion* a_pValue);
Vpi_Void Vpi_VariantArrayValue_Clear(Vpi_UInt16 uDatatype, Vpi_Int32 iLength, Vpi_VariantArrayUnion* pValue);
Vpi_StatusCode Vpi_VariantArrayValue_CopyTo(Vpi_UInt16 a_Datatype,
												Vpi_Int32                     a_iLength,
												const Vpi_VariantArrayUnion*  a_pSource,
												const Vpi_VariantArrayUnion*  a_pDestination);
Vpi_StatusCode Vpi_VariantArrayValue_CreateArray(Vpi_UInt16    a_Datatype,
													Vpi_Int32     a_iNumberOfElements,
													Vpi_Void**    a_ppArray);
Vpi_StatusCode Vpi_VariantUnion_CopyTo(Vpi_UInt16 a_Datatype, const Vpi_VariantUnion* a_pSource, Vpi_VariantUnion* a_pDestination);
#endif /* _Vpi_Trace_H_ */
