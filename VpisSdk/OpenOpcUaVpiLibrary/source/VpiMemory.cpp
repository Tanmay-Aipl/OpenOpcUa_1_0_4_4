//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiMemory.cpp
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
#include "VpiMemory.h"
Vpi_Void* VPI_DLLCALL Vpi_Memory_Alloc(Vpi_UInt32 nSize)
{
	size_t iSize = (size_t)nSize;
	return malloc(iSize);
}

Vpi_Void VPI_DLLCALL Vpi_Memory_Free(Vpi_Void* pBuffer)
{
	if (pBuffer != NULL)
	{
		free(pBuffer);
	}
}

Vpi_StatusCode VPI_DLLCALL Vpi_Memory_MemCpy(Vpi_Void*  pBuffer,
	Vpi_UInt32 nSizeInBytes,
	Vpi_Void*  pSource,
	Vpi_UInt32 nCount)
{
	Vpi_StatusCode uStatus = Vpi_Good;
	if (pBuffer == Vpi_Null
		|| pSource == Vpi_Null)
		uStatus= Vpi_BadInvalidArgument;
	else
	{
		if (nSizeInBytes < nCount)
			uStatus = Vpi_BadInvalidArgument;
		else
		{
			if (memcpy(pBuffer, pSource, nCount) != pBuffer)
				uStatus = Vpi_BadInvalidArgument;
		}
	}
	return uStatus;
}