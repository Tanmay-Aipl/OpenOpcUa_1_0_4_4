/*****************************************************************************
Author
©. Michel Condemine, 4CE Industry (2010-2012)

Contributors


This software is a computer program whose purpose is to
implement behavior describe in the OPC UA specification.
see wwww.opcfoundation.org for more details about OPC.
This software is governed by the CeCILL-C license under French law and
abiding by the rules of distribution of free software.  You can  use,
modify and/ or redistribute the software under the terms of the CeCILL-C
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info".

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability.

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or
data to be ensured and,  more generally, to use and operate it in the
same conditions as regards security.

The fact that you are presently reading this means that you have had
knowledge of the CeCILL-C license and that you accept its terms.

*****************************************************************************/
// stdafx.h : fichier Include pour les fichiers Include système standard,
// ou les fichiers Include spécifiques aux projets qui sont utilisés fréquemment,
// et sont rarement modifiés
//
#pragma once#ifdef _GNUC_#include <stdio.h>#include <stdlib.h>#include <string.h>#include <pthread.h>#ifndef __cdecl#define __cdecl#endif
#ifndef __stdcall
#define __stdcall
#endif
#endif

#ifdef WIN32
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN  
#include <windows.h>
#include <string>
#include <vector>
#endif

#include "Vfi.h"
#include "VfiXxx.h"
#define OpcUa_Int32_Min     (OpcUa_Int32)(-2147483647L-1)
#define OpcUa_Int32_Max     (OpcUa_Int32)2147483647L
#define OpcUa_UInt32_Min    (OpcUa_UInt32)0UL
#define OpcUa_UInt32_Max    (OpcUa_UInt32)4294967295UL
#define OPCUA_STRINGLENZEROTERMINATED   0xffffffffL
#define OPCUA_STRING_LENDONTCARE        OPCUA_STRINGLENZEROTERMINATED
#define OpcUa_uiMagic       0

#define OpcUa_SPrintfA									sprintf
#define OpcUa_Alloc										malloc
#define OpcUa_StrnCatA(xDst, xDstSize, xSrc, xCount)	strncat(xDst, xSrc, xCount)
#define OpcUa_StrLenA(xStr)                             (OpcUa_UInt32)strlen(xStr)
#define OpcUa_Free		                                free
#define OpcUa_MemCpy(xDst, xDstSize, xSrc, xCount)      memcpy(xDst, xSrc, xDstSize)
