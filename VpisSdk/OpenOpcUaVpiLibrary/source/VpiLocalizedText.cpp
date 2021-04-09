//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiLocalizedText.cpp
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
#include "VpiLocalizedText.h"
using namespace VpiBuiltinType;
/*============================================================================
* Vpi_LocalizedText_Initialize
*===========================================================================*/
void Vpi_LocalizedText_Initialize(Vpi_LocalizedText* a_pValue)
{
	if (a_pValue != Vpi_Null)
	{
		Vpi_Field_Initialize(String, Locale);
		Vpi_Field_Initialize(String, Text);
	}
}

/*============================================================================
* Vpi_LocalizedText_Clear
*===========================================================================*/
void Vpi_LocalizedText_Clear(Vpi_LocalizedText* a_pValue)
{
	if (a_pValue != Vpi_Null)
	{
		Vpi_Field_Clear(String, Locale);
		Vpi_Field_Clear(String, Text);
	}
}

/*============================================================================
* Vpi_LocalizedText_CopyTo
*===========================================================================*/
Vpi_StatusCode Vpi_LocalizedText_CopyTo(const Vpi_LocalizedText* a_pSource, Vpi_LocalizedText* a_pDestination)
{
	Vpi_StatusCode uStatus = Vpi_Good;

	Vpi_Field_CopyTo(String, Locale);
	Vpi_Field_CopyTo(String, Text);

	if (uStatus!=Vpi_Good)
		Vpi_LocalizedText_Clear(a_pDestination);

	return uStatus;
}