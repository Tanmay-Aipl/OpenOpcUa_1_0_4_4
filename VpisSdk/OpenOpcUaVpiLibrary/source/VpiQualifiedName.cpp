//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiQualifiedName.cpp
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
#include "VpiQualifiedName.h"
using namespace VpiBuiltinType;
/*============================================================================
* Vpi_QualifiedName_Initialize
*===========================================================================*/
void Vpi_QualifiedName_Initialize(Vpi_QualifiedName* a_pValue)
{
	if (a_pValue != Vpi_Null)
	{
		Vpi_Field_Initialize(Int32, NamespaceIndex);
		Vpi_Field_Initialize(String, Name);
	}
}

/*============================================================================
* Vpi_QualifiedName_Clear
*===========================================================================*/
void Vpi_QualifiedName_Clear(Vpi_QualifiedName* a_pValue)
{
	if (a_pValue != Vpi_Null)
	{
		Vpi_Field_Clear(Int32, NamespaceIndex);
		Vpi_Field_Clear(String, Name);
	}
}

/*============================================================================
* Vpi_QualifiedName_CopyTo
*===========================================================================*/
Vpi_StatusCode Vpi_QualifiedName_CopyTo(const Vpi_QualifiedName* a_pSource, Vpi_QualifiedName* a_pDestination)
{
	Vpi_StatusCode uStatus = Vpi_Good;
	if (a_pDestination)
	{
		Vpi_Field_CopyTo(String, Name);
		Vpi_Field_CopyToScalar(UInt16, NamespaceIndex);
		if (uStatus!=Vpi_Good)
			Vpi_QualifiedName_Clear(a_pDestination);
	}
	return uStatus;
}