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
// Fichier entete exporté a placer dans les fichiers utilisant cette DLL
// Function name   : xmlSaxParseFile
// Description     : 
// Return type     : BOOL 
// Argument        : char *pBasePath /* base path to the file specified */
// Argument        : char *pParseFileName /* filename to be parsed, with full relative path (relative to pBasePath) */

#include "stdio.h" 
/////////////////////////////////////////////////////
// some specific definition coming from the opcua.h
// just recreate here for simplification
// Some error code retrun by the XMLParser API
#define OpcUa_Good					0x00000000
#define OpcUa_Bad					0x80000000
#define OpcUa_BadInternalError		0x80020000
#define OpcUa_BadOutOfMemory		0x80030000
#define OpcUa_BadInvalidArgument	0x80AB0000
#define OpcUa_BadFileNotFound		0x81090000
// 
#ifndef __x86_64__
typedef unsigned long       OpcUa_UInt32;
typedef OpcUa_UInt32    OpcUa_StatusCode;
#else
typedef unsigned long		OpcUa_ULong;
typedef OpcUa_ULong    OpcUa_StatusCode;
#endif

#ifdef _GNUC_
#include <opcua_platformdefs.h>
#include "opcua_p_os.h"
#endif
typedef struct XML_ParserStruct *XML_Parser;
typedef struct XML_OutputStruct * XML_Output;
typedef void (__cdecl *XML_CharacterDataHandler)(	void *userData,     /* user defined data */
													const char *s,  /* non-null terminated character string */
													int len);           /* length of s */

typedef void (__cdecl *XML_StartElementHandler)(void *userData,
												 const char *name,
												 const char **atts);

typedef void (__cdecl *XML_EndElementHandler)(void *userData,
											   const char *name);
OpcUa_StatusCode __cdecl xml4CE_SAXSetCharacterDataHandler(XML_Parser* pXmlParser,XML_CharacterDataHandler xmlCharacterDataHandler);
OpcUa_StatusCode __cdecl xml4CE_SAXSetElementHandler(XML_Parser* pXmlParser,
														 XML_StartElementHandler start,
														 XML_EndElementHandler end);
OpcUa_StatusCode __cdecl xml4CE_SAXOpenParser(char *pBasePath,
												  char *pParseFileName, 
												  FILE **pParseFile, 
												  XML_Parser* pXmlParser);
OpcUa_StatusCode __cdecl Xml4CE_SAXCloseParser(XML_Parser* pXmlParser);
OpcUa_StatusCode __cdecl xml4CE_SAXParseBegin(FILE *pParseFile, XML_Parser* pXmlParser);
typedef void (__cdecl *XML_AttlistDeclHandler) (
									void        *userData,
									const char  *elname,
									const char  *attname,
									const char  *att_type,
									const char  *dflt,
									int          isrequired);
OpcUa_StatusCode xml4CE_SAXSetAttlistDeclHandler(XML_Parser* pXmlParser,XML_AttlistDeclHandler attdecl);
//
typedef void (__cdecl *XML_StartCdataSectionHandler) (void *userData);
typedef void (__cdecl *XML_EndCdataSectionHandler) (void *userData);
OpcUa_StatusCode xml4CE_SAXSetCDataSectionHandler(XML_Parser* pXmlParser,XML_StartCdataSectionHandler start,XML_EndCdataSectionHandler end);
OpcUa_StatusCode xml4CE_SAXCreateOutput(char *pBasePath,char *pParseFileName, XML_Output* pOutput);
OpcUa_StatusCode xml4CE_SAXCloseOutput(XML_Output* pOutput);
OpcUa_StatusCode xml4CE_SAXStartDoctypeDeclWrite(XML_Output* pOutput,char *doctypeName, char *sysid, char *pubid, int has_internal_subset);
OpcUa_StatusCode xml4CE_SAXEndDoctypeDeclWrite(XML_Output* pOutput);
OpcUa_StatusCode xml4CE_SAXStartElementWrite(XML_Output* pOutput, char *name, char **atts);
OpcUa_StatusCode xml4CE_SAXEndElementWrite(XML_Output* pOutput, const char *name);
OpcUa_StatusCode xml4CE_SAXOutputFormatSet(XML_Output* pOutput, bool bFormat);                                                                     
OpcUa_StatusCode xml4CE_SAXCommentWrite (XML_Output* pOutput, char *comment);
OpcUa_StatusCode xml4CE_SAXCharacterDataWrite(XML_Output* pOutput, const char *szData, int ilen);
OpcUa_StatusCode xml4CE_SAXSetUserData(XML_Parser* pXmlParser, void* pUserData);
