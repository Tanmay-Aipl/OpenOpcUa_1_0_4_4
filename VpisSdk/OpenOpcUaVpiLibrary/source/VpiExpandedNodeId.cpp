//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiExpandedNodeId.cpp
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
#include "VpiExpandedNodeId.h"
using namespace VpiBuiltinType;
/*============================================================================
* Vpi_ExpandedNodeId_Initialize
*===========================================================================*/
Vpi_Void Vpi_ExpandedNodeId_Initialize(Vpi_ExpandedNodeId* a_pValue)
{
	if (a_pValue == Vpi_Null)
	{
		return;
	}

	Vpi_MemSet(a_pValue, 0, sizeof(Vpi_ExpandedNodeId));
	Vpi_NodeId_Initialize(&a_pValue->NodeId);
}

/*============================================================================
* Vpi_ExpandedNodeId_Clear
*===========================================================================*/
Vpi_Void Vpi_ExpandedNodeId_Clear(Vpi_ExpandedNodeId* a_pValue)
{
	if (a_pValue == Vpi_Null)
	{
		return;
	}

	Vpi_NodeId_Clear(&a_pValue->NodeId);
	Vpi_String_Clear(&(a_pValue->NamespaceUri));
	Vpi_MemSet(a_pValue, 0, sizeof(Vpi_ExpandedNodeId));
}

/*============================================================================
* Vpi_ExpandedNodeId_CopyTo
*===========================================================================*/
Vpi_StatusCode Vpi_ExpandedNodeId_CopyTo(const Vpi_ExpandedNodeId* a_pSource, Vpi_ExpandedNodeId* a_pDestination)
{
	Vpi_StatusCode uStatus=Vpi_Good;
	if (a_pDestination)
	{
		Vpi_Field_CopyToScalar(UInt32, ServerIndex);
		Vpi_Field_CopyTo(String, NamespaceUri);
		Vpi_Field_CopyTo(NodeId, NodeId);

		if (uStatus!=Vpi_Good)
			Vpi_ExpandedNodeId_Clear(a_pDestination);
	}

	return uStatus;
}