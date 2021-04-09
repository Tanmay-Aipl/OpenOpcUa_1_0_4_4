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

#include <opcua.h>

#ifdef OPCUA_HAVE_XMLAPI

#include <opcua_stream.h>
#include <opcua_xmlwriter.h>

#define OPCUA_P_XMLWRITER_CREATE OpcUa_ProxyStub_g_PlatformLayerCalltable->CreateXmlWriter
#define OPCUA_P_XMLWRITER_DELETE OpcUa_ProxyStub_g_PlatformLayerCalltable->DeleteXmlWriter

/*============================================================================
 * OpcUa_XmlWriter_WriteCallback
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_XmlWriter_WriteCallback(
    struct _OpcUa_XmlWriter*    a_pXmlWriter,
    OpcUa_Void*                 a_pWriteContext, 
    OpcUa_Byte*                 a_pWriteBuffer, 
    OpcUa_UInt32                a_uBufferLength)
{
    OpcUa_OutputStream* pOutputStream = OpcUa_Null;

    OpcUa_ReturnErrorIfArgumentNull(a_pXmlWriter);
    OpcUa_ReturnErrorIfArgumentNull(a_pWriteContext);

    pOutputStream = (OpcUa_OutputStream*)a_pWriteContext;

    return pOutputStream->Write(pOutputStream, a_pWriteBuffer, a_uBufferLength);
}

/*============================================================================
 * OpcUa_XmlWriter_CloseCallback
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_XmlWriter_CloseCallback(
    struct _OpcUa_XmlWriter*    a_pXmlWriter,
    OpcUa_Void*                 a_pWriteContext)
{
    OpcUa_ReferenceParameter(a_pXmlWriter);
    OpcUa_ReferenceParameter(a_pWriteContext);

    return OpcUa_Good;
}

/*============================================================================
 * OpcUa_XmlWriter_Create
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlWriter_Create(
    struct _OpcUa_XmlWriter**   a_ppXmlWriter,
    struct _OpcUa_OutputStream* a_pOutputStream)
{
OpcUa_InitializeStatus(OpcUa_Module_XmlWriter, "OpcUa_XmlWriter_Create");

    OpcUa_ReturnErrorIfArgumentNull(a_ppXmlWriter);
    OpcUa_ReturnErrorIfArgumentNull(a_pOutputStream);

    *a_ppXmlWriter = (OpcUa_XmlWriter*)OpcUa_Alloc(sizeof(OpcUa_XmlWriter));
    OpcUa_GotoErrorIfAllocFailed(*a_ppXmlWriter);

    uStatus = OPCUA_P_XMLWRITER_CREATE(a_pOutputStream, 
                                       OpcUa_XmlWriter_WriteCallback,
                                       OpcUa_XmlWriter_CloseCallback,
                                       *a_ppXmlWriter);
    OpcUa_GotoErrorIfBad(uStatus);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
    
    OpcUa_Free(*a_ppXmlWriter);
    *a_ppXmlWriter = OpcUa_Null;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlWriter_Delete
 *===========================================================================*/
OPCUA_EXPORT OpcUa_Void OpcUa_XmlWriter_Delete(
    struct _OpcUa_XmlWriter**   a_ppXmlWriter)
{
    if(a_ppXmlWriter != OpcUa_Null && *a_ppXmlWriter != OpcUa_Null)
    {
        OPCUA_P_XMLWRITER_DELETE(*a_ppXmlWriter);
        OpcUa_Free(*a_ppXmlWriter);
        *a_ppXmlWriter = OpcUa_Null;
    }
}

/*============================================================================
 * OpcUa_XmlWriter_StartElement
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlWriter_StartElement(
    struct _OpcUa_XmlWriter*    a_pXmlWriter,
    OpcUa_StringA               a_sNamespacePrefix,
    OpcUa_StringA               a_sElementName,
    OpcUa_StringA               a_sNamespaceUri)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlWriter);
    OpcUa_ReturnErrorIfArgumentNull(a_pXmlWriter);
    OpcUa_ReturnErrorIfNull(a_pXmlWriter->StartElement, OpcUa_BadNotSupported);
    
    return a_pXmlWriter->StartElement(a_pXmlWriter, a_sNamespacePrefix, a_sElementName, a_sNamespaceUri);
}

/*============================================================================
 * OpcUa_XmlWriter_EndElement
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlWriter_EndElement(
    struct _OpcUa_XmlWriter*    a_pXmlWriter)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlWriter);
    OpcUa_ReturnErrorIfArgumentNull(a_pXmlWriter);
    OpcUa_ReturnErrorIfNull(a_pXmlWriter->EndElement, OpcUa_BadNotSupported);

    return a_pXmlWriter->EndElement(a_pXmlWriter);
}

/*============================================================================
 * OpcUa_XmlWriter_WriteAttribute
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlWriter_WriteAttribute(
    struct _OpcUa_XmlWriter*    a_pXmlWriter,
    OpcUa_StringA               a_sNamespacePrefix,
    OpcUa_StringA               a_sAttributeName,
    OpcUa_StringA               a_sNamespaceUri,
    OpcUa_StringA               a_sAttributeValue)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlWriter);
    OpcUa_ReturnErrorIfArgumentNull(a_pXmlWriter);
    OpcUa_ReturnErrorIfNull(a_pXmlWriter->WriteAttribute, OpcUa_BadNotSupported);

    return a_pXmlWriter->WriteAttribute(a_pXmlWriter, 
                                        a_sNamespacePrefix, 
                                        a_sAttributeName, 
                                        a_sNamespaceUri, 
                                        a_sAttributeValue);
}

/*============================================================================
 * OpcUa_XmlWriter_WriteString
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlWriter_WriteString(
    struct _OpcUa_XmlWriter*    a_pXmlWriter,
    OpcUa_StringA               a_sValue)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlWriter);
    OpcUa_ReturnErrorIfArgumentNull(a_pXmlWriter);
    OpcUa_ReturnErrorIfNull(a_pXmlWriter->WriteString, OpcUa_BadNotSupported);

    return a_pXmlWriter->WriteString(a_pXmlWriter, a_sValue);
}

/*============================================================================
 * OpcUa_XmlWriter_WriteFormatted
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlWriter_WriteFormatted(
    struct _OpcUa_XmlWriter*    a_pXmlWriter,
    OpcUa_StringA               a_sFormat,
                                ...)
{
    OpcUa_P_VA_List           arguments;
    
OpcUa_InitializeStatus(OpcUa_Module_XmlWriter, "OpcUa_XmlWriter_WriteFormatted");
    
    OpcUa_ReturnErrorIfArgumentNull(a_pXmlWriter);
    OpcUa_ReturnErrorIfNull(a_pXmlWriter->WriteFormatted, OpcUa_BadNotSupported);

    OPCUA_P_VA_START(arguments, a_sFormat);
    uStatus = a_pXmlWriter->WriteFormatted(a_pXmlWriter, a_sFormat, arguments);
    OPCUA_P_VA_END(arguments);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlWriter_WriteRaw
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlWriter_WriteRaw(
    struct _OpcUa_XmlWriter*    a_pXmlWriter,
    OpcUa_Byte*                 a_pRawData,
    OpcUa_UInt32                a_uDataLength)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlWriter);
    OpcUa_ReturnErrorIfArgumentNull(a_pXmlWriter);
    OpcUa_ReturnErrorIfNull(a_pXmlWriter->WriteRaw, OpcUa_BadNotSupported);

    return a_pXmlWriter->WriteRaw(a_pXmlWriter, a_pRawData, a_uDataLength);
}

/*============================================================================
 * OpcUa_XmlWriter_WriteNode
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlWriter_WriteNode(
    struct _OpcUa_XmlWriter*     a_pXmlWriter,
    struct _OpcUa_XmlReader*     a_pXmlReader)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlWriter);
    OpcUa_ReturnErrorIfArgumentNull(a_pXmlWriter);
    OpcUa_ReturnErrorIfNull(a_pXmlWriter->WriteNode, OpcUa_BadNotSupported);

    return a_pXmlWriter->WriteNode(a_pXmlWriter, a_pXmlReader);
}

/*============================================================================
 * OpcUa_XmlWriter_Flush
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlWriter_Flush(
    struct _OpcUa_XmlWriter*    a_pXmlWriter)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlWriter);
    OpcUa_ReturnErrorIfArgumentNull(a_pXmlWriter);
    OpcUa_ReturnErrorIfNull(a_pXmlWriter->Flush, OpcUa_BadNotSupported);

    return a_pXmlWriter->Flush(a_pXmlWriter);
}

/*============================================================================
 * OpcUa_XmlWriter_Close
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlWriter_Close(
    struct _OpcUa_XmlWriter*    a_pXmlWriter)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlWriter);
    OpcUa_ReturnErrorIfArgumentNull(a_pXmlWriter);
    OpcUa_ReturnErrorIfNull(a_pXmlWriter->Close, OpcUa_BadNotSupported);

    return a_pXmlWriter->Close(a_pXmlWriter);
}

#endif /* OPCUA_HAVE_XMLAPI */
