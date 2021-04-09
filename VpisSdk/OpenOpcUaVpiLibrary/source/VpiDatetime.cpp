//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiDateTime.cpp
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
#include "VpiDatetime.h"
using namespace VpiBuiltinType;
Vpi_StatusCode VPI_DLLCALL Vpi_DateTime_GetStringFromDateTime(  Vpi_DateTime  a_dateTime, 
                                                        Vpi_StringA   a_pBuffer, 
                                                        Vpi_UInt32    a_uLength)
{
    Vpi_StatusCode uStatus=Vpi_Good;
	Vpi_Int           apiResult    = 0;
	const char*         formatString = "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ";
	FILETIME            FileTime;
	SYSTEMTIME          SystemTime;

    if (a_pBuffer)
	{
		if(a_uLength < 25)
			uStatus = Vpi_BadInvalidArgument;
		else
		{

			FileTime.dwLowDateTime  = (DWORD)(a_dateTime.dwLowDateTime);
			FileTime.dwHighDateTime = (DWORD)(a_dateTime.dwHighDateTime);
			if (FileTimeToSystemTime(&FileTime, &SystemTime))
			{
#if VPI_USE_SAFE_FUNCTIONS
#ifdef WIN32
				apiResult = Vpi_SPrintfA(a_pBuffer, a_uLength, formatString, SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds);
#endif

#ifdef _GNUC_
				apiResult = Vpi_SPrintfA(a_pBuffer, formatString, SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds); 
#endif
#else /* VPI_USE_SAFE_FUNCTIONS */
				apiResult = sprintf(a_pBuffer,
					formatString,
					SystemTime.wYear,
					SystemTime.wMonth,
					SystemTime.wDay,
					SystemTime.wHour,
					SystemTime.wMinute,
					SystemTime.wSecond,
					SystemTime.wMilliseconds);
#endif /* VPI_USE_SAFE_FUNCTIONS */

				if (apiResult < 20)
				{
					uStatus = Vpi_Bad;
				}
			}
			else
				uStatus = Vpi_BadUnexpectedError;
		}
	}
	else
		uStatus = Vpi_BadInvalidArgument;
    return uStatus;
}

Vpi_DateTime VPI_DLLCALL Vpi_DateTime_UtcNow()
{
	FILETIME ftTime;
    Vpi_DateTime tmpDateTime;

    GetSystemTimeAsFileTime(&ftTime);

    tmpDateTime.dwHighDateTime = (Vpi_UInt32)ftTime.dwHighDateTime;
    tmpDateTime.dwLowDateTime  = (Vpi_UInt32)ftTime.dwLowDateTime;

    return tmpDateTime;
}