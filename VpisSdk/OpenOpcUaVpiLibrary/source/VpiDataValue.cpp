//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiDataValue.cpp
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************
/*
 *
 * Permission is hereby granted, for a commerciale use of this 
 * software and associated documentation files (the "Software")
 *
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 * ======================================================================*/
#include "stdafx.h"
#include "VpiDataValue.h"
using namespace VpiBuiltinType;
/*============================================================================
* Vpi_DataValue_Initialize
*===========================================================================*/
Vpi_StatusCode VPI_DLLCALL Vpi_DataValue_Initialize(Vpi_DataValue* a_pValue)
{
	Vpi_StatusCode uStatus = Vpi_Good;
	if (a_pValue == Vpi_Null)
		uStatus = Vpi_BadInvalidArgument;
	else
		Vpi_MemSet(a_pValue, 0, sizeof(Vpi_DataValue));
	return uStatus;
}

/*============================================================================
* Vpi_DataValue_Clear
*===========================================================================*/
Vpi_StatusCode VPI_DLLCALL Vpi_DataValue_Clear(Vpi_DataValue* a_pValue)
{
	Vpi_StatusCode uStatus = Vpi_Good;
	if (a_pValue == Vpi_Null)
		uStatus = Vpi_BadInvalidArgument;
	else
	{
		Vpi_Variant_Clear(&a_pValue->Value);
		Vpi_MemSet(a_pValue, 0, sizeof(Vpi_DataValue));
	}
	return uStatus;
}
///-------------------------------------------------------------------------------------------------
/// <summary>	Vpi_DataValue_CopyTo: Copy a Vpi_DataValue from source to destination. </summary>
///
/// <remarks>	Michel, 23/07/2016. </remarks>
///
/// <param name="a_pSource">	 	Vpi_DataValue Source. </param>
/// <param name="a_pDestination">	[in/out] If non-null, Vpi_DataValue destination. </param>
///
/// <returns>	A VPI_DLLCALL. </returns>
///-------------------------------------------------------------------------------------------------

Vpi_StatusCode VPI_DLLCALL Vpi_DataValue_CopyTo(const Vpi_DataValue* a_pSource, Vpi_DataValue* a_pDestination)
{
	Vpi_StatusCode uStatus = Vpi_Good;

	Vpi_Field_CopyToScalar(StatusCode, StatusCode);
	Vpi_Field_CopyToScalar(DateTime, ServerTimestamp);
#if !VPI_DATAVALUE_OMIT_PICOSECONDS
	Vpi_Field_CopyToScalar(DateTime, ServerPicoseconds);
#endif /* !VPI_DATAVALUE_OMIT_PICOSECONDS */
	Vpi_Field_CopyToScalar(DateTime, SourceTimestamp);
#if !VPI_DATAVALUE_OMIT_PICOSECONDS
	Vpi_Field_CopyToScalar(DateTime, SourcePicoseconds);
#endif /* !VPI_DATAVALUE_OMIT_PICOSECONDS */
	Vpi_Field_CopyTo(Variant, Value);

	return uStatus;
}