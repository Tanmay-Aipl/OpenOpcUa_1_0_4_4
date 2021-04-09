//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  AddItemDlg.cpp
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************
#ifndef VPIXMLELEMENT
#define VPIXMLELEMENT
#define Vpi_XmlElement_Initialize(xValue) Vpi_ByteString_Initialize((Vpi_ByteString*)xValue);
#define Vpi_XmlElement_Clear(xValue) Vpi_ByteString_Clear((Vpi_ByteString*)xValue);
#define Vpi_XmlElement_CopyTo(xSource, xDestination) Vpi_ByteString_CopyTo((Vpi_ByteString*)xSource, (Vpi_ByteString*)xDestination)

#endif