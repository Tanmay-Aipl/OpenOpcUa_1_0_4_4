// stdafx.h : fichier Include pour les fichiers Include système standard,
// ou les fichiers Include spécifiques aux projets qui sont utilisés fréquemment,
// et sont rarement modifiés
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclure les en-têtes Windows rarement utilisés
#define OPC_TIMEOUT 5000 // 5 Seconde timeout
// Fichiers d'en-tête Windows :
#ifndef _GNUC_
#include <windows.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#ifndef __cdecl
#define __cdecl
#endif

#ifndef __stdcall
#define __stdcall
#endif
#endif
#include <errno.h>
#include <string>
#include <vector>


#include "openopcuavpisdk.h"
// fichiers d'interface
#include "VpiDataTypes.h"

#include "VPI.h"
#include "VpiDataValue.h"
#include "VPINullEx.h"

#include "VpiInternal.h"
#include "opcuavpimutsem.h"
