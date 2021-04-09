//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  stdafx.h
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************
// stdafx.h�: fichier Include pour les fichiers Include syst�me standard,
// ou les fichiers Include sp�cifiques aux projets qui sont utilis�s fr�quemment,
// et sont rarement modifi�s
//
// 
#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclure les en-t�tes Windows rarement utilis�s
// Fichiers d'en-t�te Windows�:
#ifndef _GNUC_
#include <windows.h>
#endif
#include <string>
#include <vector>

#define OPC_TIMEOUT 5000 // 5 Seconde timeout
#define	ESC_ROOT_DATA1			"." // current directory


// fichiers d'interface
#include "VpiConfig.h"
#include "VpiDataTypes.h"
#include "VpiTypes.h"

#include "VpiOs.h"
#include "VPI.h"
#include "opcuavpimutsem.h"
#include "SerialPort.h"
#include "VpiPlatformdefs.h"
#include "VpiInternalThread.h"
#include "VpiSocket.h"
#include "VpiMutex.h"
#include "VpiSemaphore.h"
#include "VpiMemory.h"
#include "VpiMacroDef.h"
#include "VpiInternalSocket.h"
#include "VpiProxyStub.h"
#include "VpiString.h"
#include "VpiDatetime.h"
#include "VpiTrace.h"
#include "VpiByteString.h"
#include "VpiXmlElement.h"
#include "VpiLocalizedText.h"
#include "VpiQualifiedName.h"
#include "VpiNodeId.h"
#include "VpiExpandedNodeId.h"
#include "VpiGuid.h"
#include "VpiVariant.h"
#include "VpiDataValue.h"
