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
/*  -----------------------------------------------------------------------
	DESCRIPTION

	This XML loader requires the following components: 
	XML SAX parser, XML output.  Or if building a project from source components,
	this tutorial requires the following source files: xmlparse.c, xmlrole.c, 
	xmltok.c, xmlop.c (located in INSTALL_DIR/target/src/webservices/xml).
  ----------------------------------------------------------------------- */

#include "stdafx.h"
#include <stdexcept>
#include "expat.h"
#include "xmlop.h"
#include "xmlCanon.h"

#define BD_MAX_BUFFER_SIZE 0x45000 // Windows CE Info : 0x45000 require by the official Opc.Ua.NodeSet2.Part5.xml due to some object initValue

//char gbuf[BD_MAX_BUFFER_SIZE];      // temp buffer to write file to to send to parser
//char ErrMsg[BD_MAX_BUFFER_SIZE];   // temp buffer to write error message
/****************************
* Used for userData example *
****************************/
typedef struct myData 
{
	BOOL myBool;        /* this variable turns handler printing on/off */
	int myTabCount;     /* this variable keeps track of the XML document hierarchy printing */
	BOOL usingNSparser; /* set to true if using the namespace parser  */
	FILE *pOutputFile;  /* file pointer for output file */
	XML_Output xmlOut;  /* pointer to xml output object */
}MY_DATA;

/******************************
* Handler function prototypes *
******************************/
void xmlElementDeclHandler(void *userData, const XML_Char *name, XML_Content *model);
void xmlXmlDeclHandler(void *userData, const XML_Char *version, const XML_Char *encoding, int standalone);
void xmlAttlistDeclHandler(void *userData, const XML_Char *elname, const XML_Char *attname, const XML_Char *att_type, const XML_Char *dflt, int isrequired);
extern "C" void __stdcall xmlStartElementHandler(void *userData, const XML_Char *name, const XML_Char **atts);
void xmlEndElementHandler(void *userData, const XML_Char *name);
//void xmlEmptyElementHandler(void *userData, const XML_Char *name, const XML_Char **atts);
void xmlCharacterDataHandler(void *userData, const XML_Char *s, int len);
void xmlProcessingInstructionHandler(void *userData, const XML_Char *target, const XML_Char *data);
void xmlCommentHandler(void *userData, const XML_Char *data);
void xmlStartCdataSectionHandler(void *userData);
void xmlEndCdataSectionHandler(void *userData);
/*void xmlDefaultHandler(void *userData, const XML_Char *s, int len);*/
//void xmlStartDoctypeDeclHandler(void *userData, const XML_Char *doctypeName, const XML_Char *sysid, const XML_Char *pubid, int has_internal_subset);
void xmlEndDoctypeDeclHandler(void *userData);
void xmlEntityDeclHandler(void *userData, const XML_Char *entityName, int is_parameter_entity, const XML_Char *value, int value_length, const XML_Char *base, const XML_Char *systemId, const XML_Char *publicId, const XML_Char *notationName);
/*void xmlUnparsedEntityDeclHandler(void *userData, const XML_Char *entityName, const XML_Char *base, const XML_Char *systemId, const XML_Char *publicId, const XML_Char *notationName);*/
void xmlNotationDeclHandler(void *userData, const XML_Char *notationName, const XML_Char *base, const XML_Char *systemId, const XML_Char *publicId);
void xmlStartNamespaceDeclHandler(void *userData, const XML_Char *prefix, const XML_Char *uri);
void xmlEndNamespaceDeclHandler(void *userData, const XML_Char *prefix);
int xmlNotStandaloneHandler(void *userData);
//int xmlExternalEntityRefHandler(XML_Parser parser, const XML_Char *context, const XML_Char *base, const XML_Char *systemId, const XML_Char *publicId);
void xmlSkippedEntityHandler(void *userData, const XML_Char *entityName, int is_parameter_entity);
int xmlUnknownEncodingHandler(void *encodingHandlerData, const XML_Char *name, XML_Encoding *info);
void xmlPrintTabs(int numTabs);

/*--------------------------------------------------------------------------*//*

	SERVICE NAME :  atoi2
	----------------------------------------------------------------

	DESCRIPTION :
		Convert asci hexa data to integer value.
	 
	EXCEPTIONS : 

	PARAMETERS
		StringValue		String to convert.

	RETURN:  
		Integer value.
*//*--------------------------------------------------------------------------*/
int		atoi2(const XML_Char*	str)
{
	int		v ;

	if(strstr((char*)str, "0x") != (char*) NULL) {
		if(sscanf((char*)str,"%x", &v)!= -1)
			return v;
	}
	else {
		if(sscanf((char*)str,"%d", &v)!= -1)
			return v;
	}

	return -1 ;
}
	
OpcUa_StatusCode xml4CE_SAXSetElementHandler(XML_Parser* pXmlParser,XML_StartElementHandler start,XML_EndElementHandler end)
{
	OpcUa_StatusCode hr=OpcUa_Good;
	XML_SetElementHandler(*pXmlParser, start, end);
	return hr;
}
OpcUa_StatusCode xml4CE_SAXSetCharacterDataHandler(XML_Parser* pXmlParser,XML_CharacterDataHandler xmlCharacterDataHandler)
{
	OpcUa_StatusCode hr = OpcUa_Good;
	XML_SetCharacterDataHandler(*pXmlParser, xmlCharacterDataHandler);
	return hr;
}
OpcUa_StatusCode xml4CE_SAXSetAttlistDeclHandler(XML_Parser* pXmlParser,XML_AttlistDeclHandler attdecl)
{
	OpcUa_StatusCode hr = OpcUa_Good;
	XML_SetAttlistDeclHandler(*pXmlParser, attdecl);
	return hr;
}
OpcUa_StatusCode xml4CE_SAXSetCDataSectionHandler(XML_Parser* pXmlParser,XML_StartCdataSectionHandler start,XML_EndCdataSectionHandler end)
{
	OpcUa_StatusCode hr = OpcUa_Good;
	XML_SetCdataSectionHandler(*pXmlParser, start, end);
	return hr;
}
/* pParseFile file pointer for file opened for parsing */
// extern "C" OpcUa_StatusCode __stdcall xml4CE_SAXOpenParser(char *pBasePath,char *pParseFileName, FILE **pParseFile, XML_Parser* pXmlParser)
OpcUa_StatusCode xml4CE_SAXOpenParser(char *pBasePath,char *pParseFileName, FILE **pParseFile, XML_Parser* pXmlParser)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	MY_DATA appData;                /* user data to be sent throughout the parser */
	char buf[MAX_PATH+1];
	/*****************************************
	*     Open the file to parse (read-only) *
	*****************************************/
	ZeroMemory(buf, MAX_PATH + 1);
	if (pBasePath)
	{
		strcpy(buf, pBasePath);
		if (pParseFileName)
		{
			strcat(buf, pParseFileName);
			*pParseFile = fopen(buf, "r");
			if (NULL == *pParseFile)
				uStatus = OpcUa_BadInvalidArgument;
			else
			{
				memset(buf, 0, MAX_PATH);
				/* initialize application data, this data is later passed throughout the
				handlers as userdata */
				appData.myBool = FALSE;
				appData.myTabCount = 0;
				appData.usingNSparser = FALSE;
				*pXmlParser = XML_ParserCreateNS(NULL, NSSEP);
				appData.usingNSparser = TRUE;
				appData.xmlOut = NULL;//XML_OutputCreate( "UTF-8", NSSEP, appData.pOutputFile );
			}
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
// Free the parser
OpcUa_StatusCode __cdecl Xml4CE_SAXCloseParser(XML_Parser* pXmlParser)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (pXmlParser)
	{
		XML_ParserFree(*pXmlParser);
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
// pParseFile  file pointer for file opened for parsing
// pXmlParser pointer to xml parser object
OpcUa_StatusCode xml4CE_SAXParseBegin(FILE *pParseFile, XML_Parser* pXmlParser)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	enum XML_Error xmlErrCode   = XML_ERROR_NONE; /* xml error code placeholder */
	char cChar;
	int nIndex = 0;
	BOOL bSlashCarFound= FALSE;
	BOOL bDashCarFound= FALSE;
	BOOL bQuesMarkCarFound= FALSE;
	BOOL bExclamMarkCarFound= FALSE;
	size_t lFileSize= 0;
	BOOL done= FALSE;
	char *buf = (char *) malloc(BD_MAX_BUFFER_SIZE);
	if (buf)
	{		
		memset(buf,0,BD_MAX_BUFFER_SIZE);

		// retrieve filesize;
		while(fread(&cChar, 1, sizeof(char), pParseFile))
			lFileSize++;
		// Move to File start 
		if (fseek(pParseFile, 0L, SEEK_SET)!=-1)
		{
		
			/**************************************
			*     Begin parsing your XML document *
			**************************************/
			// search for the first <
			while(fread(&cChar, 1, sizeof(char), pParseFile))
			{
				lFileSize--;
				if (cChar=='<')
				{
					nIndex=0;
					buf[nIndex++] = cChar;
					break;
				}
			}
			while(fread(&cChar, 1, sizeof(char), pParseFile) && (uStatus ==OpcUa_Good))
			{
				lFileSize--;
				if (nIndex<BD_MAX_BUFFER_SIZE)
				{
					buf[nIndex++] = cChar;
					if (!bExclamMarkCarFound)
					{
						if (bQuesMarkCarFound && cChar != '>')
							bQuesMarkCarFound=FALSE;
						if (bDashCarFound && cChar != '>')
							bDashCarFound=FALSE;
						if (nIndex > BD_MAX_BUFFER_SIZE-1)
						{
							uStatus = OpcUa_BadOutOfMemory;
							memset(buf,0,BD_MAX_BUFFER_SIZE);
							nIndex=0;
						}
						else
						{
							if (cChar == '!')
							{
								if (buf[nIndex-2]=='<')
									bExclamMarkCarFound= TRUE; // Attention il peut s'agir d'un point d'exclamation utilisé "NORMALEMENT"
							}
							if (cChar == '/')
								bSlashCarFound = TRUE;
							if (cChar == '?') // fin entête fichier
								bQuesMarkCarFound = TRUE;
							if (cChar == '-') // fin commentaire
								bDashCarFound = TRUE;

							if (cChar == '>')
							{
								if (lFileSize == 0)
									done = TRUE;
								// Call the parser to wake up the correct call backs
								if ( XML_Parse(*pXmlParser, buf, nIndex, done) == 0 )
								{
									xmlErrCode = XML_GetErrorCode(*pXmlParser);
									if (xmlErrCode != 0)
									{
										printf("Error(%d):<%s> Failed to parse : %s\n",							        
												xmlErrCode,
												XML_ErrorString(xmlErrCode),
												buf);
									}
								}
								bSlashCarFound = FALSE;
								bQuesMarkCarFound = FALSE;
								bDashCarFound = FALSE;
								memset(buf,0,BD_MAX_BUFFER_SIZE);
								nIndex = 0;
							}
						}
					}
					else
					{
						// on ne demande pas au parser d'analyser les commentaires
						if (bExclamMarkCarFound && bDashCarFound && cChar != '>')
							bDashCarFound=FALSE;
						if (bExclamMarkCarFound && bDashCarFound && cChar == '>')
						{
							bExclamMarkCarFound = FALSE;	
							memset(buf,0,BD_MAX_BUFFER_SIZE);
							nIndex = 0;
						}
						else
						{
							if (bExclamMarkCarFound  && cChar == '-')
								bDashCarFound=TRUE;
						}
					}
				}
				else
				{
					printf("An XML element is too big to fit to multiplatform constraints.. Please make it shorter\n\t [%s]\n", buf);
					uStatus=OpcUa_BadOutOfMemory;
				}
			}
		}
		else
			uStatus=OpcUa_BadInternalError;
		free(buf);
	}
	else
		uStatus=OpcUa_BadOutOfMemory;
	return uStatus;
}
/// <summary>
/// This function will create an XML file .
/// </summary>
/// <param name="pBasePath">The p base path.</param>
/// <param name="pParseFileName">Name of the p parse file.</param>
/// <param name="pOutput">The p output.</param>
/// <returns></returns>
OpcUa_StatusCode xml4CE_SAXCreateOutput(char *pBasePath,char *pParseFileName, XML_Output* pOutput)
{
	OpcUa_StatusCode uStatus = OpcUa_Bad;
	XML_Char * encoding=(XML_Char *)"UTF-8";/* output char encoding, not presently used, exists only to match the SAX API */
	//XML_Char nsSep;/* namespace separator char */
	FILE * fp  ;
	char buf[MAX_PATH+1];
	memset(buf,0,MAX_PATH+1);
	strcpy(buf, pBasePath);
	strcat(buf, pParseFileName);
	fp= fopen(buf, "w+");
	if (fp)
	{
		*pOutput = XML_OutputCreate(encoding, NSSEP, fp);
		if (pOutput)
		{
			uStatus = OpcUa_Good;
			//(*pOutput)->doFormat = true;
			// Par défaut on va ajouter la déclaration
			uStatus = (OpcUa_StatusCode)XML_XmlDeclWrite(*pOutput, "1.0", encoding, -1);
		}
	}
	else
		uStatus = OpcUa_BadFileNotFound;
	
	return uStatus;
}
OpcUa_StatusCode xml4CE_SAXCloseOutput(XML_Output* pOutput)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	fclose((*pOutput)->fp);
	uStatus = (OpcUa_StatusCode)XML_OutputDestroy(*pOutput);
	return uStatus;
}
OpcUa_StatusCode xml4CE_SAXOutputFormatSet(XML_Output* pOutput, bool bFormat)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	uStatus = (OpcUa_StatusCode)XML_OutputFormatSet(*pOutput, (XML_Bool)bFormat);
	return uStatus;
}
OpcUa_StatusCode xml4CE_SAXCommentWrite (XML_Output* pOutput, char *comment)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	uStatus = (OpcUa_StatusCode)XML_CommentWrite(*pOutput, (const XML_Char*)comment);
	return uStatus;
}
OpcUa_StatusCode xml4CE_SAXCharacterDataWrite(XML_Output* pOutput, const char *szData, int ilen)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	if (pOutput)
		uStatus = XML_CharacterDataWrite(*pOutput, (const XML_Char*)szData, ilen);
	return uStatus;
}
OpcUa_StatusCode xml4CE_SAXStartDoctypeDeclWrite(XML_Output* pOutput,XML_Char *doctypeName, XML_Char *sysid, XML_Char *pubid, int has_internal_subset)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	uStatus = (OpcUa_StatusCode)XML_StartDoctypeDeclWrite(*pOutput, doctypeName, sysid, pubid, has_internal_subset);
	return uStatus;
}
OpcUa_StatusCode xml4CE_SAXEndDoctypeDeclWrite(XML_Output* pOutput)
{
	OpcUa_StatusCode hr=OpcUa_BadInvalidArgument;
	hr = (OpcUa_StatusCode)XML_EndDoctypeDeclWrite(*pOutput);
	return hr;
}
OpcUa_StatusCode xml4CE_SAXStartElementWrite(XML_Output* pOutput, XML_Char *name, XML_Char **atts)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	uStatus = (OpcUa_StatusCode)XML_StartElementWrite(*pOutput, name, (const XML_Char**)atts);

	return uStatus;
}
OpcUa_StatusCode xml4CE_SAXEndElementWrite(XML_Output* pOutput, const XML_Char *name)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	uStatus = (OpcUa_StatusCode) XML_EndElementWrite(*pOutput, name);

	return uStatus;
}
OpcUa_StatusCode xml4CE_SAXSetUserData(XML_Parser* pXmlParser, void* pUserData)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	if (pXmlParser)
	{
		if (pUserData)
		{
			XML_SetUserData(*pXmlParser, pUserData);
			uStatus = OpcUa_Good;
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	return uStatus ;
}

/*************************************************
*         Functions you can call inside handlers *
*************************************************/
/*      XML_ParserReset(XML_Parser parser, const XML_Char *encoding);
		XML_DefaultCurrent(XML_Parser parser);
		XML_GetSpecifiedAttributeCount(XML_Parser parser);
		XML_GetIdAttributeIndex(XML_Parser parser);*/


/***************************************************************************
* xmlElementDeclHandler - This is called on an !ELEMENT declaration
*
* This is called on an !ELEMENT declaration.  It is the caller's
* responsiblity to free model when finished with it.  Below is the structure
* for model:
*  struct XML_cp {
*  enum XML_Content_Type         type;
*  enum XML_Content_Quant        quant;
*  XML_Char *                    name;
*  unsigned int                  numchildren;
*  XML_Content *                 children;
*
* RETURNS: N/A
*
* ERRNO: N/A
*/

void xmlElementDeclHandler
	(
	void *userData,         /* user defined data */
	const XML_Char *name,   /* name of the !ELEMENT */
	XML_Content *model      /* structure containing ELEMENT related data */
	)
	{
	BOOL printThis = ((MY_DATA*)userData)->myBool;

	/* Write the Element declartion to the XML output file */
	XML_ElementDeclWrite( ((MY_DATA*)userData)->xmlOut, name, model );

	if( printThis == TRUE )
		{
		printf("XML ELEMENT DECLARATION HANDLER called\n");
		printf("!ELEMENT name = %s\n", name);
		printf("\n");
		}

	/* It is the caller's responsibility to free model when finished */
	if(model != NULL)
		{
		free(model);
		}
	}

/***************************************************************************
* xmlXmlDeclHandler - This is called for both XML and TEXT declarations
*
* The XML declaration handler is called for *both* XML declarations
* and text declarations. The way to distinguish is that the version
* parameter will be NULL for text declarations. The encoding
* parameter may be NULL for XML declarations. The standalone
* parameter will be -1, 0, or 1 indicating respectively that there
* was no standalone parameter in the declaration, that it was given
* as no, or that it was given as yes.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
void xmlXmlDeclHandler
	(
	void *userData,             /* user defined data */
	const XML_Char *version,    /* XML version */
	const XML_Char *encoding,   /* XML document encoding type */
	int standalone              /* bool indicating if document is standalone */
	)
	{
	BOOL printThis = ((MY_DATA*)userData)->myBool;

	/* Write the XML declartion to the XML output file */
	XML_XmlDeclWrite( ((MY_DATA*)userData)->xmlOut, version, encoding, standalone );

	if( printThis == TRUE )
		{
		if(version != NULL)
			{
			printf("XML XML DECLARATION HANDLER called\n");
			printf("Version = %s, Encoding = %s\n", version, encoding);
			}
		else
			{
			printf("XML XML(TEXT) DECLARATION HANDLER called\n");
			printf("Encoding = %s ", encoding);
			}

		switch(standalone)
			{
			case -1:
				printf("standalone = no standalone parameter\n");
				break;
			case 0:
				printf("standalone = no\n");
				break;
			case 1:
				printf("standalone = yes\n");
				break;
			default:
				break;
			}

		}
	}

/***************************************************************************
* xmlAttlistDeclHandler - This is called for each ATTRIBUTE in an ATTLIST
*
* The Attlist declaration handler is called for *each* attribute. So
* a single Attlist declaration with multiple attributes declared will
* generate multiple calls to this handler. The "default" parameter
* may be NULL in the case of the "#IMPLIED" or "#REQUIRED"
* keyword. The "isrequired" parameter will be true and the default
* value will be NULL in the case of "#REQUIRED". If "isrequired" is
* true and default is non-NULL, then this is a "#FIXED" default.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
void xmlAttlistDeclHandler
	(
	void *userData,             /* user defined data */
	const XML_Char *elname,     /* ELEMENT name that ATTLIST pertains to */
	const XML_Char *attname,    /* ATTRIBUTE name */
	const XML_Char *att_type,   /* ATTRIBUTE data type */
	const XML_Char *dflt,       /* DEFAULT value, if applicable */
	int isrequired              /* bool indicating #REQUIRED */
	)
	{
	BOOL printThis = ((MY_DATA*)userData)->myBool;

	/* Write the Attlist declaration to the XML output file */
	XML_AttlistDeclWrite( ((MY_DATA*)userData)->xmlOut, elname, attname,
						  att_type, dflt, isrequired );


	if( printThis == TRUE )
		{
		printf("XML ATTLIST HANDLER called\n");
		printf("ELEMENT name = %s\n", elname);
		printf("ATTRIBUTE name = %s, ", attname);

		if(isrequired != 0)
			{
			if(dflt == NULL)
				printf("#REQUIRED\n");
			else
				printf("#FIXED\n");
			}
		else if(isrequired == 0 && dflt == NULL)
			{
			printf("#IMPLIED\n");
			}
		else
			{
			printf("default = %s\n", dflt);
			}

		printf("ATTRIBUTE type = %s\n", att_type);
		}

	}

/***************************************************************************
* xmlStartElementHandler - This is called at the beginning of an XML element
*
* This is called at the beginning of an XML element.  Atts is an array of
* name/value pairs, i.e. atts[0] contains name, atts[1] contains value for
* atts[0], and so on... Names and values are NULL terminated.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
void __stdcall xmlStartElementHandler
(
	void *userData,         /* user defined data */
	const XML_Char *name,   /* Element name */
	const XML_Char **atts   /* Element Attribute list, provided in name value pairs */
							/* i.e. atts[0] contains name, atts[1] contains value for */
							/* atts[0], and so on...*/
	)
	{
	XML_StartElementWrite( ((MY_DATA*)userData)->xmlOut, name, atts );

}


/***************************************************************************
* xmlEndElementHandler - This is called at the end of an XML element
*
* This is called at the end of an XML element. Name contains the element
* name that is closing.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
void xmlEndElementHandler
	(
	void *userData,     /* user defined data */
	const XML_Char *name/* Element name */
	)
{

	XML_EndElementWrite( ((MY_DATA*)userData)->xmlOut, name );
}




/***************************************************************************
* xmlEmptyElementHandler - This is called when an empty element is
* encountered
*
* This is called when an empty element is encountered, such as: <element/>
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
//void xmlEmptyElementHandler
//    (
//    void *userData,         /* user defined data */
//    const XML_Char *name,   /* Element name */
//    const XML_Char **atts   /* Element Attribute list, provided in name value pairs */
//                            /* i.e. atts[0] contains name, atts[1] contains value for */
//                            /* atts[0], and so on...*/
//    )
//{
//}

/***************************************************************************
* xmlCharacterDataHandler - This is called when XML element character
* content is encountered
*
* This is called when XML element character content is encountered. The s
* XML_Char string is not NULL terminated.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
void xmlCharacterDataHandler
	(
	void *userData,     /* user defined data */
	const XML_Char *s,  /* non-null terminated character string */
	int len             /* length of s */
	)
	{
	BOOL printThis = ((MY_DATA*)userData)->myBool;

	/* Write the encountered character data to the XML output file */
	XML_CharacterDataWrite( ((MY_DATA*)userData)->xmlOut, s, len );

	if( printThis == TRUE )
		{
		if(len > 0 && !XML_IsWhiteSpace(s, len))
			{
			xmlPrintTabs(((MY_DATA*)userData)->myTabCount);
			//printf("XML CHARACTER DATA HANDLER called\n");
			xmlPrintTabs(((MY_DATA*)userData)->myTabCount);
			//printf("Character length = %d\n", len);
			xmlPrintTabs(((MY_DATA*)userData)->myTabCount);
			/*printf("Character data = ");
			for(i=0; i < len; i++)
				{
				printf("%c", s[i]);
				}

			printf("\n");*/
			}
/* the character data handler is also called on whitespace occurrences
   removing this code removes capturing that event
		else
			{
			if(len <= 0)
				{
				xmlPrintTabs(((MY_DATA*)userData)->myTabCount);
				printf("No Character data\n");
				}
			else if(XML_IsWhiteSpace(s,len))
				{
				xmlPrintTabs(((MY_DATA*)userData)->myTabCount);
				printf("All characters are white spaces\n");
				}
			}
*/

		}
	}

/***************************************************************************
* xmlProcessingInstructionHandler - This is called when a <? ... ?> element,
* or processing instruction is encountered
*
* This is called when a <? ... ?> element, or processing instruction is
* encountered. target and data are NULL terminated.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
void xmlProcessingInstructionHandler
	(
	void *userData,         /* user defined data */
	const XML_Char *target, /* Processing Instruction target */
	const XML_Char *data    /* Processing Instruction data */
	)
	{
	BOOL printThis = ((MY_DATA*)userData)->myBool;

	/* Write the encountered processing instruction to the XML output file */
	XML_ProcessingInstructionWrite( ((MY_DATA*)userData)->xmlOut, target, data );

	if( printThis == TRUE )
		{
		printf("XML PROCESSING INSTRUCTION HANDLER called\n");
		printf("PITarget = %s, PIData = %s\n", target, data);
		printf("\n");
		}

	}

/***************************************************************************
* xmlCommentHandler - This is called when a <!-- ... --> element, or XML
* comment is encountered
*
* This is called when a <!-- ... --> element, or XML comment is encountered.
* data is NULL terminated.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
void xmlCommentHandler
	(
	void *userData,     /* user defined data */
	const XML_Char *data/* XML comment contents */
	)
	{
	BOOL printThis = ((MY_DATA*)userData)->myBool;

	/* Write the encountered XML comment to the XML output file */
	XML_CommentWrite( ((MY_DATA*)userData)->xmlOut, data );

	if( printThis == TRUE )
		{
		printf("XML COMMENT HANDLER called\n");
		printf("Comment: %s\n", data);
		printf("\n");
		}

	}


/***************************************************************************
* xmlStartCdataSectionHandler - This is called when the beginning of a
* <! [CDATA[....]]>, or CDATA section is encountered.
*
* This is called when the beginning of a <! [CDATA[....]]>, or CDATA section
* is encountered.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
void xmlStartCdataSectionHandler(void *userData)
	{
	BOOL printThis = ((MY_DATA*)userData)->myBool;

	/* Write the start CDATA section declaration to the XML output file */
	XML_StartCdataSectionWrite( ((MY_DATA*)userData)->xmlOut );

	if( printThis == TRUE )
		{
		printf("XML START CDATA SECTION HANDLER called\n");
		}

	}

/***************************************************************************
* xmlEndCdataSectionHandler - This is called when the end of a
* <! [CDATA[....]]>, or CDATA section is encountered.
*
* This is called when the end of a <! [CDATA[....]]>, or CDATA section
* is encountered.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
void xmlEndCdataSectionHandler(void *userData)
	{
	BOOL printThis = ((MY_DATA*)userData)->myBool;

	/* Write the end CDATA section declaration to the XML output file */
	XML_EndCdataSectionWrite( ((MY_DATA*)userData)->xmlOut );

	if( printThis == TRUE )
		{
		printf("XML END CDATA SECITON HANDLER called\n");
		printf("\n");
		}

	}


/***************************************************************************
* xmlDefaultHandler - This is called for any characters in the XML document
* for which there is no applicable handler.
*
* This is called for any characters in the XML document for which
* there is no applicable handler.  This includes both characters that
* are part of markup which is of a kind that is not reported
* (comments, markup declarations), or characters that are part of a
* construct which could be reported but for which no handler has been
* supplied. The characters are passed exactly as they were in the XML
* document except that they will be encoded in UTF-8 or UTF-16.
* Line boundaries are not normalized. Note that a byte order mark
* character is not passed to the default handler. There are no
* guarantees about how characters are divided between calls to the
* default handler: for example, a comment might be split between
* multiple calls.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
/* this function has been purposely compiled out due to the nature of its 
functionality explained above */
#if 0
void xmlDefaultHandler
	(
	void *userData,     /* user defined data */
	const XML_Char *s,  /* non-null terminated string containing XML content */
						/* default handler was called regarding */
	int len
	)
	{
	BOOL printThis = ((MY_DATA*)userData)->myBool;

	if( printThis == TRUE )
		{
		printf("XML DEFAULT HANDLER called\n");
		printf("XML Data = %s\n", s);
		printf("\n");
		}
	return;
	}
#endif

/***************************************************************************
* xmlStartDoctypeDeclHandler - This is called for the start of the DOCTYPE
* declaration, before any DTD or internal subset is parsed.
*
* This is called for the start of the DOCTYPE declaration, before any DTD or
* internal subset is parsed. doctypeName, sysid and pubid are all NULL
* terminated.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
//void xmlStartDoctypeDeclHandler
//    (
//    void *userData,             /* user defined data */
//    const XML_Char *doctypeName,/* DOCTYPE name */
//    const XML_Char *sysid,      /* SYSTEM attribute contents */
//    const XML_Char *pubid,      /* PUBLIC attribute contents */
//    int has_internal_subset     /* bool indicating if internal or external subset */
//    )
//    {
//    BOOL printThis = ((MY_DATA*)userData)->myBool;
//
//    /* Write the start DOCTYPE declaration to the XML output file */
//    XML_StartDoctypeDeclWrite( ((MY_DATA*)userData)->xmlOut, doctypeName, sysid, pubid, has_internal_subset);
//
//    if( printThis == TRUE )
//        {
//        printf("XML START DOCTYPE HANDLER called\n");
//        printf("Doctype name = %s, sysId = %s, pubId = %s\n", doctypeName, sysid, pubid);
//        }
//
//    }

/***************************************************************************
* xmlEndDoctypeDeclHandler - This is called for the start of the DOCTYPE
* declaration when the closing > is encountered, but after processing any
* external subset.
*
* This is called for the start of the DOCTYPE declaration when the closing >
* is encountered, but after processing any external subset.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
void xmlEndDoctypeDeclHandler(void *userData)
	{
	BOOL printThis = ((MY_DATA*)userData)->myBool;

	/* Write the end DOCTYPE declaration to the XML output file */
	XML_EndDoctypeDeclWrite( ((MY_DATA*)userData)->xmlOut );

	if( printThis == TRUE )
		{
		printf("XML END DOCTYPE HANDLER called\n");
		printf("\n");
		}

	}

/***************************************************************************
* xmlEntityDeclHandler - This is called for !ENTITY declarations
*
* This is called for entity declarations. The is_parameter_entity
* argument will be non-zero if the entity is a parameter entity, zero
* otherwise.
*
* For internal entities (<!ENTITY foo "bar">), value will
* be non-NULL and systemId, publicID, and notationName will be NULL.
* The value string is NOT nul-terminated; the length is provided in
* the value_length argument. Since it is legal to have zero-length
* values, do not use this argument to test for internal entities.
*
* For external entities, value will be NULL and systemId will be
* non-NULL. The publicId argument will be NULL unless a public
* identifier was provided. The notationName argument will have a
* non-NULL value only for unparsed entity declarations.
*
* Note that is_parameter_entity can't be changed to XML_Bool, since
* that would break binary compatibility.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
void xmlEntityDeclHandler
	(
	void *userData,             /* user defined data */
	const XML_Char *entityName, /* ENTITY name */
	int is_parameter_entity,    /* bool indicating parameter entity */
	const XML_Char *value,      /* ENTITY value, not null if internal entity, null */
								/* if external entity, this string is non-null terminated */
	int value_length,           /* length of value */
	const XML_Char *base,       /* base path to entity set by XML_SetBase*/
	const XML_Char *systemId,   /* SYSTEM attribute contents */
	const XML_Char *publicId,   /* PUBLIC attribute contents */
	const XML_Char *notationName/* NOTATION attribute contents */
	)
	{
	BOOL printThis = ((MY_DATA*)userData)->myBool;
	int i=0;

	/* Write the ENTITY declaration to the XML output file */
	XML_EntityDeclWrite( ((MY_DATA*)userData)->xmlOut, entityName,
						 is_parameter_entity, value, value_length, base,
						 systemId, publicId, notationName );

	if( printThis == TRUE )
		{
		printf("XML ENTITY DECLARATION HANDLER called\n");

		/* If it is a parameter entity */
		if(is_parameter_entity != 0)
			{
			/* if it is an internal parameter entity */
			if(systemId == NULL && publicId == NULL && notationName == NULL)
				{
				printf("PARAMETER INTERNAL entity name = %s\n", entityName);
				printf("Entity value = ");
				
				for(i=0; i < value_length; i++)
					{
					printf("%c", value[i]);
					}
				
				printf("\n");
				printf("Entity base = %s\n", base);
				}
			/* if it is an external parameter entity */
			else
				{
				printf("PARAMETER EXTERNAL entity name = %s\n", entityName);
				printf("Entity systemId = %s\n", systemId);

				if(publicId != NULL)
					{
					printf("Entity publicId = %s\n", publicId);
					}

				if(notationName != NULL)
					{
					printf("Entity notationName = %s\n", notationName);
					}
				}
			}
		else
			{
			/* if it is an internal entity */
			if(systemId == NULL && publicId == NULL && notationName == NULL)
				{
				printf("INTERNAL entity name = %s\n", entityName);
				printf("Entity value = ");
				
				for(i=0; i < value_length; i++)
					{
					printf("%c", value[i]);
					}
				
				printf("\n");
				printf("Entity base = %s\n", base);
				}
			/* if it is an external entity */
			else
				{
				printf("EXTERNAL entity name = %s\n", entityName);
				printf("Entity systemId = %s\n", systemId);

				if(publicId != NULL)
					{
					printf("Entity publicId = %s\n", publicId);
					}

				if(notationName != NULL)
					{
					printf("Entity notationName = %s\n", notationName);
					}
				}
			}

		}

	}

/***************************************************************************
* xmlUnparsedEntityDeclHandler - OBSOLETE, This handler has been superceded
* by the EntityDeclHandler above.
*
* OBSOLETE -- OBSOLETE -- OBSOLETE
* This handler has been superceded by the EntityDeclHandler above.
* It is provided here for backward compatibility.
*
* This is called for a declaration of an unparsed (NDATA) entity.
* The base argument is whatever was set by XML_SetBase. The
* entityName, systemId and notationName arguments will never be
* NULL. The other arguments may be.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
#if 0
void xmlUnparsedEntityDeclHandler
	(
	void *userData,             /* user defined data */
	const XML_Char *entityName, /* ENTITY name */
	const XML_Char *base,       /* base path to entity set by XML_SetBase*/
	const XML_Char *systemId,   /* SYSTEM attribute contents */
	const XML_Char *publicId,   /* PUBLIC attribute contents */
	const XML_Char *notationName/* NOTATION attribute contents */
	)
	{
	BOOL printThis = ((MY_DATA*)userData)->myBool;

	if( printThis == TRUE )
		{
		printf("XML UNPARSED ENTITY DECLARATION HANDLER called\n");
		printf("Entity Name = %s, base = %s\n", entityName, base);
		printf("sytemId = %s, publicId = %s, Notation Name = %s\n", systemId, publicId, notationName);
		printf("\n");
		}

	}
#endif

/***************************************************************************
* xmlNotationDeclHandler - This is called for a declaration of !NOTATION
*
* This is called for a declaration of notation.  The base argument is
* whatever was set by XML_SetBase. The notationName will never be
* NULL.  The other arguments can be.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
void xmlNotationDeclHandler
	(
	void *userData,                 /* user defined data */
	const XML_Char *notationName,   /* NOTATION attribute contents */
	const XML_Char *base,           /* base path to entity set by XML_SetBase*/
	const XML_Char *systemId,       /* SYSTEM attribute contents */
	const XML_Char *publicId        /* PUBLIC attribute contents */
	)
	{
	BOOL printThis = ((MY_DATA*)userData)->myBool;

	/* Write the NOTATION declaration to the XML output file */
	XML_NotationDeclWrite( ((MY_DATA*)userData)->xmlOut, notationName, base,
						   systemId, publicId );


	if( printThis == TRUE )
		{
		printf("XML NOTATION DECLARATION HANDLER called\n");
		printf("Notation Name = %s, base = %s, systemId = %s, publicId = %s\n", notationName, base, systemId, publicId);
		printf("\n");
		}

	}

/***************************************************************************
* xmlStartNamespaceDeclHandler - When namespace processing is enabled, this
* is called once for each namespace declaration.
*
* When namespace processing is enabled, this is called once for
* each namespace declaration. The call to the start and end element
* handlers occur between the calls to the start and end namespace
* declaration handlers. For an xmlns attribute, prefix will be
* NULL.  For an xmlns="" attribute, uri will be NULL.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
void xmlStartNamespaceDeclHandler
	(    void *userData,         /* user defined data */
	const XML_Char *prefix, /* prefix representing namespace */
	const XML_Char *uri     /* URI for the namespace (unique identifier) */
	)
	{
	BOOL printThis = ((MY_DATA*)userData)->myBool;

	/* Write the encountered namespace declaration to the XML output file */
	XML_StartNamespaceDeclWrite( ((MY_DATA*)userData)->xmlOut, prefix, uri );

	if( printThis == TRUE )
		{
		printf("XML START NAMESPACE DECLARATION HANDLER called\n");
		printf("Prefix = %s, URI = %s\n", prefix, uri);
		printf("\n");
		}

	}

/***************************************************************************
* xmlEndNamespaceDeclHandler - When namespace processing is enabled, this
* is called once for each namespace declaration.
*
* When namespace processing is enabled, this is called once for
* each namespace declaration. The call to the start and end element
* handlers occur between the calls to the start and end namespace
* declaration handlers. For an xmlns attribute, prefix will be
* NULL.  For an xmlns="" attribute, uri will be NULL.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
void xmlEndNamespaceDeclHandler
	(
	void *userData,         /* user defined data */
	const XML_Char *prefix  /* prefix representing namespace */
	)
	{
	BOOL printThis = ((MY_DATA*)userData)->myBool;

	/* Write the end namespace declaration to the XML output file */
	XML_EndNamespaceDeclWrite( ((MY_DATA*)userData)->xmlOut, prefix );

	if( printThis == TRUE )
		{
		printf("XML END NAMESPACE DECLARATION HANDLER called\n");
		printf("Prefix = %s\n", prefix);
		printf("\n");
		}

	}

/***************************************************************************
* xmlNotStandaloneHandler - This is called if the document is not standalone
*
* This is called if the document is not standalone, that is, it has an
* external subset or a reference to a parameter entity, but does not
* have standalone="yes". If this handler returns 0, then processing
* will not continue, and the parser will return a
* XML_ERROR_NOT_STANDALONE error.
* If parameter entity parsing is enabled, then in addition to the
* conditions above this handler will only be called if the referenced
* entity was actually read.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
int xmlNotStandaloneHandler(void *userData)
	{
	BOOL printThis = ((MY_DATA*)userData)->myBool;

	if( printThis == TRUE )
		{
		printf("XML NOTSTANDALONE HANDLER called\n");
		printf("\n");
		}

	return XML_STATUS_ERROR;
	}

/***************************************************************************
* xmlExternalEntityRefHandler - This is called for a reference to an
* external parsed general entity.
*
* This is called for a reference to an external parsed general
* entity.  The referenced entity is not automatically parsed.  The
* application can parse it immediately or later using
* XML_ExternalEntityParserCreate.
*
* The parser argument is the parser parsing the entity containing the
* reference; it can be passed as the parser argument to
* XML_ExternalEntityParserCreate.  The systemId argument is the
* system identifier as specified in the entity declaration; it will
* not be NULL.
*
* The base argument is the system identifier that should be used as
* the base for resolving systemId if systemId was relative; this is
* set by XML_SetBase; it may be NULL.
*
* The publicId argument is the public identifier as specified in the
* entity declaration, or NULL if none was specified; the whitespace
* in the public identifier will have been normalized as required by
* the XML spec.
*
* The context argument specifies the parsing context in the format
* expected by the context argument to XML_ExternalEntityParserCreate;
* context is valid only until the handler returns, so if the
* referenced entity is to be parsed later, it must be copied.
*
* The handler should return 0 if processing should not continue
* because of a fatal error in the handling of the external entity.
* In this case the calling parser will return an
* XML_ERROR_EXTERNAL_ENTITY_HANDLING error.
*
* Note that unlike other handlers the first argument is the parser,
* not userData.
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
//int xmlExternalEntityRefHandler
//    (
//    XML_Parser parser,          /* XML parser object currently parsing the docuemnt */
//    const XML_Char *context,    /* pointer to XML document context */
//    const XML_Char *base,       /* base path to entity set by XML_SetBase*/
//    const XML_Char *systemId,   /* SYSTEM attribute contents */
//    const XML_Char *publicId    /* PUBLIC attribute contents */
//    )
//    {
//    BOOL printThis = ((MY_DATA*)XML_GetUserData(parser))->myBool;
//    //BOOL parseExtEntity = ((MY_DATA*)XML_GetUserData(parser))->myIndicator;
//    BOOL parseContext = ((MY_DATA*)XML_GetUserData(parser))->usingNSparser;
//
//	if(context == NULL) return XML_STATUS_OK;
//    if (TRUE == parseContext)
//        {
//        XML_Char *extEntityName=NULL;
//        XML_Char *i;
//
//        /*  parse for entity name  */
//        for ( i = (XML_Char *)context; *i != '\0'; i++ )
//            {
//            if ( '\f' == *i )
//                {
//                extEntityName = &(i[1]);
//                }
//            }
//        /* Write the entity reference to the XML output file */
//        XML_EntityRefWrite(  ((MY_DATA*)XML_GetUserData(parser))->xmlOut, extEntityName );
//        } 
//    else
//        {
//        /* Write the entity reference to the XML output file */
//        XML_EntityRefWrite(  ((MY_DATA*)XML_GetUserData(parser))->xmlOut, context );
//        }
//
//
//
//    if( printThis == TRUE )
//        {
//        printf("XML EXTERNAL ENTITY HANDLER called\n");
//        printf("Context = %s, base = %s, sytemId = %s, publicId = %s\n", context, base, systemId, publicId);
//        if(parseExtEntity == TRUE)
//            {
//            /* Parse the file pointed to by external entity reference 
//            xmlSaxParseFile( (char *)base, (char *)systemId, parser);*/
//            xmlSaxParseFile( (char *)base, (char *)systemId);
//            }
//        printf("\n");
//        }
//    return XML_STATUS_OK;
//    }

/***************************************************************************
* xmlSkippedEntityHandler - This is called for special entity reference
* situations
*
* This is called in two situations:
*   1) An entity reference is encountered for which no declaration
*      has been read *and* this is not an error.
*   2) An internal entity reference is read, but not expanded, because
*      XML_SetDefaultHandler has been called.
*   Note: skipped parameter entities in declarations and skipped general
*         entities in attribute values cannot be reported, because
*         the event would be out of sync with the reporting of the
*         declarations or attribute values
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
void xmlSkippedEntityHandler
	(
	void *userData,             /* user defined data */
	const XML_Char *entityName, /* ENTITY name */
	int is_parameter_entity     /* bool indicating parameter entity */
	)
{
	BOOL printThis = ((MY_DATA*)userData)->myBool;

	if( printThis == TRUE )
		{
		printf("XML SKIPPED ENTITY HANDLER called\n");
		printf("Entity name = %s is_parameter_entity=%u\n", entityName,is_parameter_entity);
		printf("\n");
		}

}

/***************************************************************************
* xmlUnknownDecodingHandler - This is called for an encoding that is unknown
* to the parser
*
* This is called for an encoding that is unknown to the parser.
*
* The encodingHandlerData argument is that which was passed as the
* second argument to XML_SetUnknownEncodingHandler.
*
* The name argument gives the name of the encoding as specified in
* the encoding declaration.
*
* If the callback can provide information about the encoding, it must
* fill in the XML_Encoding structure, and return 1.  Otherwise it
* must return 0.
*
* If info does not describe a suitable encoding, then the parser will
* return an XML_UNKNOWN_ENCODING error.
*
* Filling in the XML_Encoding structure:
*
*
* This structure is filled in by the XML_UnknownEncodingHandler to
* provide information to the parser about encodings that are unknown
* to the parser.
*
* The map[b] member gives information about byte sequences whose
* first byte is b.
*
* If map[b] is c where c is >= 0, then b by itself encodes the
* Unicode scalar value c.
*
* If map[b] is -1, then the byte sequence is malformed.
*
* If map[b] is -n, where n >= 2, then b is the first byte of an
* n-byte sequence that encodes a single Unicode scalar value.
*
* The data member will be passed as the first argument to the convert
* function.
*
* The convert function is used to convert multibyte sequences; s will
* point to a n-byte sequence where map[(unsigned char)*s] == -n.  The
* convert function must return the Unicode scalar value represented
* by this byte sequence or -1 if the byte sequence is malformed.
*
* The convert function may be NULL if the encoding is a single-byte
* encoding, that is if map[b] >= -1 for all bytes b.
*
* When the parser is finished with the encoding, then if release is
* not NULL, it will call release passing it the data member; once
* release has been called, the convert function will not be called
* again.
*
* Expat places certain restrictions on the encodings that are supported
* using this mechanism.
*
* 1. Every ASCII character that can appear in a well-formed XML document,
*   other than the characters
*
*   $@\^`{}~
*
*   must be represented by a single byte, and that byte must be the
*   same byte that represents that character in ASCII.
*
* 2. No character may require more than 4 bytes to encode.
*
* 3. All characters encoded must have Unicode scalar values <=
*   0xFFFF, (i.e., characters that would be encoded by surrogates in
*   UTF-16 are  not allowed).  Note that this restriction doesn't
*   apply to the built-in support for UTF-8 and UTF-16.
*
* 4. No Unicode character may be encoded by more than one distinct
*   sequence of bytes.
*
*   typedef struct {
*     int map[256];
*     void *data;
*     int (*convert)(void *data, const char *s);
*     void (*release)(void *data);
*   } XML_Encoding;
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
int xmlUnknownEncodingHandler
	(
	void *encodingHandlerData,  /* user data */
	const XML_Char *name,       /* encoding name */
	XML_Encoding *info          /* structure containing encoding info */
	)
{
	(void)(info);
	(void)(encodingHandlerData);
	printf("XML UNKNOWN ENCODING HANDLER called \n");
	printf("name = %s\n", name);
	printf("\n");

	return XML_STATUS_ERROR;
}


/***************************************************************************
* xmlPrintTabs - Prints to stdout the number of tabs specified by numTabs
*
* Prints to stdout the number of tabs specified by numTabs
*
* RETURNS: N/A
*
* ERRNO: N/A
*/
void xmlPrintTabs(int numTabs)
	{
	int i=0;

	for(i=1; i<numTabs; i++)
		{
		printf("\t");
		}

	}


