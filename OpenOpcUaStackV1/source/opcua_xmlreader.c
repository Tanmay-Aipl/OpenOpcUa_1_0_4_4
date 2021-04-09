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
#include <opcua_xmlreader.h>
#include <opcua_xmlwriter.h>
#include <opcua_memorystream.h>


#define OPCUA_P_XMLREADER_CREATE OpcUa_ProxyStub_g_PlatformLayerCalltable->CreateXmlReader
#define OPCUA_P_XMLREADER_DELETE OpcUa_ProxyStub_g_PlatformLayerCalltable->DeleteXmlReader

/*============================================================================
 * OpcUa_XmlReader_ReadCallback
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_XmlReader_ReadCallback(
    struct _OpcUa_XmlReader*    a_pXmlReader,
    OpcUa_Void*                 a_pReadContext, 
    OpcUa_Byte*                 a_pReadBuffer, 
    OpcUa_UInt32*               a_pBufferLength)
{
    OpcUa_InputStream* pInputStream = OpcUa_Null;

    OpcUa_ReturnErrorIfArgumentNull(a_pXmlReader);
    OpcUa_ReturnErrorIfArgumentNull(a_pReadContext);

    pInputStream = (OpcUa_InputStream*)a_pReadContext;

    return pInputStream->Read(pInputStream, 
                              a_pReadBuffer, 
                              a_pBufferLength, 
                              OpcUa_Null, 
                              OpcUa_Null);
}

/*============================================================================
 * OpcUa_XmlReader_CloseCallback
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_XmlReader_CloseCallback(
    struct _OpcUa_XmlReader*    a_pXmlReader,
    OpcUa_Void*                 a_pReadContext)
{
    OpcUa_ReferenceParameter(a_pXmlReader);
    OpcUa_ReferenceParameter(a_pReadContext);

    return OpcUa_Good;
}

/*============================================================================
 * OpcUa_XmlReader_Create
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlReader_Create(
    struct _OpcUa_XmlReader**   a_ppXmlReader,
    struct _OpcUa_InputStream*  a_pInputStream)
{  
OpcUa_InitializeStatus(OpcUa_Module_XmlReader, "OpcUa_XmlReader_Create");
    
    *a_ppXmlReader = (OpcUa_XmlReader*)OpcUa_Alloc(sizeof(OpcUa_XmlReader));
    OpcUa_GotoErrorIfAllocFailed(a_ppXmlReader);

    uStatus = OPCUA_P_XMLREADER_CREATE(a_pInputStream,
                                       OpcUa_XmlReader_ReadCallback,
                                       OpcUa_XmlReader_CloseCallback,
                                       *a_ppXmlReader);
    OpcUa_GotoErrorIfBad(uStatus);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_Free(a_ppXmlReader);
    *a_ppXmlReader = OpcUa_Null;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlReader_Delete
 *===========================================================================*/
OPCUA_EXPORT OpcUa_Void OpcUa_XmlReader_Delete(
    struct _OpcUa_XmlReader**   a_ppXmlReader)
{
    if(a_ppXmlReader != OpcUa_Null && *a_ppXmlReader != OpcUa_Null)
    {
        OPCUA_P_XMLREADER_DELETE(*a_ppXmlReader);
        OpcUa_Free(*a_ppXmlReader);
        *a_ppXmlReader = OpcUa_Null;
    }
}

/*============================================================================
 * OpcUa_XmlReader_MoveToContent
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlReader_MoveToContent(
    struct _OpcUa_XmlReader*    a_pXmlReader,
    OpcUa_Int32*                a_pNodeType)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlReader);
    OpcUa_ReturnErrorIfArgumentNull(a_pXmlReader);
    OpcUa_ReturnErrorIfNull(a_pXmlReader->MoveToContent, OpcUa_BadNotSupported);

    return a_pXmlReader->MoveToContent(a_pXmlReader, a_pNodeType);
}

/*============================================================================
 * OpcUa_XmlReader_MoveToElement
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlReader_MoveToElement(
    struct _OpcUa_XmlReader*    a_pXmlReader)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlReader);
    OpcUa_ReturnErrorIfArgumentNull(a_pXmlReader);
    OpcUa_ReturnErrorIfNull(a_pXmlReader->MoveToElement, OpcUa_BadNotSupported);

    return a_pXmlReader->MoveToElement(a_pXmlReader);
}


/*============================================================================
 * OpcUa_XmlReader_MoveToFirstAttribute
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlReader_MoveToFirstAttribute(
    struct _OpcUa_XmlReader*    a_pXmlReader)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlReader);
    OpcUa_ReturnErrorIfArgumentNull(a_pXmlReader);
    OpcUa_ReturnErrorIfNull(a_pXmlReader->MoveToFirstAttribute, OpcUa_BadNotSupported);
    
    return a_pXmlReader->MoveToFirstAttribute(a_pXmlReader);
}

/*============================================================================
 * OpcUa_XmlReader_MoveToNextAttribute
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlReader_MoveToNextAttribute(
    struct _OpcUa_XmlReader*    a_pXmlReader)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlReader);
    OpcUa_ReturnErrorIfArgumentNull(a_pXmlReader);
    OpcUa_ReturnErrorIfNull(a_pXmlReader->MoveToNextAttribute, OpcUa_BadNotSupported);

    return a_pXmlReader->MoveToNextAttribute(a_pXmlReader);
}

/*============================================================================
 * OpcUa_XmlReader_IsStartElement
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlReader_IsStartElement(
    struct _OpcUa_XmlReader*    a_pXmlReader,
    OpcUa_StringA               a_sLocalName,
    OpcUa_StringA               a_sNamespaceUri,
    OpcUa_Boolean*              a_pResult)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlReader);
    OpcUa_ReturnErrorIfArgumentNull(a_pXmlReader);
    OpcUa_ReturnErrorIfNull(a_pXmlReader->IsStartElement, OpcUa_BadNotSupported);

    return a_pXmlReader->IsStartElement(a_pXmlReader, a_sLocalName, a_sNamespaceUri, a_pResult);
}

/*============================================================================
 * OpcUa_XmlReader_IsEmptyElement
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlReader_IsEmptyElement(
    struct _OpcUa_XmlReader*    a_pXmlReader,
    OpcUa_Boolean*              a_pResult)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlReader);
    OpcUa_ReturnErrorIfArgumentNull(a_pXmlReader);
    OpcUa_ReturnErrorIfNull(a_pXmlReader->IsEmptyElement, OpcUa_BadNotSupported);

    return a_pXmlReader->IsEmptyElement(a_pXmlReader, a_pResult);
}

/*============================================================================
 * OpcUa_XmlReader_HasAttributes
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlReader_HasAttributes(
    struct _OpcUa_XmlReader*    a_pXmlReader,
    OpcUa_Boolean*              a_pResult)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlReader);
    OpcUa_ReturnErrorIfArgumentNull(a_pXmlReader);
    OpcUa_ReturnErrorIfNull(a_pXmlReader->HasAttributes, OpcUa_BadNotSupported);

    return a_pXmlReader->HasAttributes(a_pXmlReader, a_pResult);
}

/*============================================================================
 * OpcUa_XmlReader_IsDefault
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlReader_IsDefault(
    struct _OpcUa_XmlReader*    a_pXmlReader,
    OpcUa_Boolean*              a_pResult)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlReader);
    OpcUa_ReturnErrorIfArgumentNull(a_pXmlReader);
    OpcUa_ReturnErrorIfNull(a_pXmlReader->IsDefault, OpcUa_BadNotSupported);

    return a_pXmlReader->IsDefault(a_pXmlReader, a_pResult);
}

/*============================================================================
 * OpcUa_XmlReader_ReadStartElement
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlReader_ReadStartElement(
    struct _OpcUa_XmlReader*    a_pXmlReader,
    OpcUa_StringA               a_sLocalName,
    OpcUa_StringA               a_sNamespaceUri)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlReader);
    OpcUa_ReturnErrorIfArgumentNull(a_pXmlReader);
    OpcUa_ReturnErrorIfNull(a_pXmlReader->ReadStartElement, OpcUa_BadNotSupported);

    return a_pXmlReader->ReadStartElement(a_pXmlReader, a_sLocalName, a_sNamespaceUri);
}

/*============================================================================
 * OpcUa_XmlReader_ReadEndElement
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlReader_ReadEndElement(
    struct _OpcUa_XmlReader*    a_pXmlReader)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlReader);
    OpcUa_ReturnErrorIfArgumentNull(a_pXmlReader);
    OpcUa_ReturnErrorIfNull(a_pXmlReader->ReadEndElement, OpcUa_BadNotSupported);

    return a_pXmlReader->ReadEndElement(a_pXmlReader);
}

/*============================================================================
 * OpcUa_XmlReader_GetNodeType
 *===========================================================================*/
OPCUA_EXPORT OpcUa_Int32 OpcUa_XmlReader_GetNodeType(
    struct _OpcUa_XmlReader*    a_pXmlReader)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlReader);

    if(a_pXmlReader != OpcUa_Null && a_pXmlReader->GetNodeType != OpcUa_Null)
    {
        return a_pXmlReader->GetNodeType(a_pXmlReader);
    }

    return OpcUa_XmlReader_NodeType_None;
}

/*============================================================================
 * OpcUa_XmlReader_GetDepth
 *===========================================================================*/
OPCUA_EXPORT OpcUa_Int32 OpcUa_XmlReader_GetDepth(
    struct _OpcUa_XmlReader*    a_pXmlReader)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlReader);

    if(a_pXmlReader != OpcUa_Null && a_pXmlReader->GetDepth != OpcUa_Null)
    {
        return a_pXmlReader->GetDepth(a_pXmlReader);
    }

    return OpcUa_XmlReader_NodeType_None;
}


/*============================================================================
 * OpcUa_XmlReader_GetLocalName
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StringA OpcUa_XmlReader_GetLocalName(
    struct _OpcUa_XmlReader*    a_pXmlReader)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlReader);

    if(a_pXmlReader != OpcUa_Null && a_pXmlReader->GetLocalName != OpcUa_Null)
    {
        return a_pXmlReader->GetLocalName(a_pXmlReader);
    }

    return "";
}

/*============================================================================
 * OpcUa_XmlReader_GetName
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StringA OpcUa_XmlReader_GetName(
    struct _OpcUa_XmlReader*    a_pXmlReader)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlReader);

    if(a_pXmlReader != OpcUa_Null && a_pXmlReader->GetName != OpcUa_Null)
    {
        return a_pXmlReader->GetName(a_pXmlReader);
    }

    return "";
}

/*============================================================================
 * OpcUa_XmlReader_GetNamespaceUri
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StringA OpcUa_XmlReader_GetNamespaceUri(
    struct _OpcUa_XmlReader*    a_pXmlReader)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlReader);

    if(a_pXmlReader != OpcUa_Null && a_pXmlReader->GetNamespaceUri != OpcUa_Null)
    {
        return a_pXmlReader->GetNamespaceUri(a_pXmlReader);
    }

    return "";
}

/*============================================================================
 * OpcUa_XmlReader_GetPrefix
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StringA OpcUa_XmlReader_GetPrefix(
    struct _OpcUa_XmlReader*    a_pXmlReader)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlReader);

    if(a_pXmlReader != OpcUa_Null && a_pXmlReader->GetPrefix != OpcUa_Null)
    {
        return a_pXmlReader->GetNamespaceUri(a_pXmlReader);
    }

    return "";
}

/*============================================================================
 * OpcUa_XmlReader_GetValue
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StringA  OpcUa_XmlReader_GetValue(
    struct _OpcUa_XmlReader*    a_pXmlReader)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlReader);

    if(a_pXmlReader != OpcUa_Null && a_pXmlReader->GetValue)
    {
        return a_pXmlReader->GetValue(a_pXmlReader);
    }

    return "";
}

/*============================================================================
 * OpcUa_XmlReader_GetAttribute
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlReader_GetAttribute(
    struct _OpcUa_XmlReader*    a_pXmlReader,
    OpcUa_StringA               a_sAttributeName,
    OpcUa_StringA               a_sNamespaceUri,
    OpcUa_StringA               a_sAttributeValue,
    OpcUa_UInt32*               a_pValueLength)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlReader);
    OpcUa_ReturnErrorIfArgumentNull(a_pXmlReader);
    OpcUa_ReturnErrorIfNull(a_pXmlReader->GetAttribute, OpcUa_BadNotSupported);

    return a_pXmlReader->GetAttribute(a_pXmlReader, 
                                      a_sAttributeName, 
                                      a_sNamespaceUri,
                                      a_sAttributeValue,
                                      a_pValueLength);
}

/*============================================================================
 * OpcUa_XmlReader_Read
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlReader_Read(
    struct _OpcUa_XmlReader*    a_pXmlReader)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlReader);
    OpcUa_ReturnErrorIfArgumentNull(a_pXmlReader);
    OpcUa_ReturnErrorIfNull(a_pXmlReader->Read, OpcUa_BadNotSupported);

    return a_pXmlReader->Read(a_pXmlReader);
}

/*============================================================================
 * OpcUa_XmlReader_Skip
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlReader_Skip(
    struct _OpcUa_XmlReader*    a_pXmlReader)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlReader);
    OpcUa_ReturnErrorIfArgumentNull(a_pXmlReader);
    OpcUa_ReturnErrorIfNull(a_pXmlReader->Skip, OpcUa_BadNotSupported);

    return a_pXmlReader->Skip(a_pXmlReader);
}

/*============================================================================
 * OpcUa_XmlReader_Close
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlReader_Close(
    struct _OpcUa_XmlReader*    a_pXmlReader)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_XmlReader);
    OpcUa_ReturnErrorIfArgumentNull(a_pXmlReader);
    OpcUa_ReturnErrorIfNull(a_pXmlReader->Close, OpcUa_BadNotSupported);

    return a_pXmlReader->Close(a_pXmlReader);
}

/*============================================================================
 * OpcUa_XmlReader_NodeType_IsTextual
 *===========================================================================*/
static OpcUa_Boolean OpcUa_XmlReader_NodeType_IsTextual(
    OpcUa_XmlReader_NodeType a_eNodeType)
{
    switch(a_eNodeType)
    {
        case OpcUa_XmlReader_NodeType_Text:
        case OpcUa_XmlReader_NodeType_CDATA:
        case OpcUa_XmlReader_NodeType_Whitespace:
        case OpcUa_XmlReader_NodeType_SignificantWhitespace:
            return OpcUa_True;

        default:
            return OpcUa_False;
    }
}


#if 0
public virtual string ReadString()
{
    if (this.ReadState != ReadState.Interactive)
    {
        return string.Empty;
    }
    this.MoveToElement();
    if (this.NodeType == XmlNodeType.Element)
    {
        if (this.IsEmptyElement)
        {
            return string.Empty;
        }
        if (!this.Read())
        {
            throw new InvalidOperationException(Res.GetString("Xml_InvalidOperation"));
        }
        if (this.NodeType == XmlNodeType.EndElement)
        {
            return string.Empty;
        }
    }
    string str = string.Empty;
    while (IsTextualNode(this.NodeType))
    {
        str = str + this.Value;
        if (!this.Read())
        {
            return str;
        }
    }
    return str;
}
#endif

/*============================================================================
 * OpcUa_XmlReader_ReadString
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlReader_ReadString(
    struct _OpcUa_XmlReader*    a_pXmlReader,
    OpcUa_String*               a_pValue)
{
OpcUa_InitializeStatus(OpcUa_Module_XmlReader, "OpcUa_XmlReader_ReadInnerXml");

    OpcUa_ReturnErrorIfArgumentNull(a_pXmlReader);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);

    OpcUa_String_Initialize(a_pValue);

    uStatus = OpcUa_XmlReader_MoveToElement(a_pXmlReader);
    OpcUa_GotoErrorIfTrue(    OpcUa_IsBad(uStatus) 
                           && uStatus != OpcUa_BadNoDataAvailable,
                          uStatus);

    if(OpcUa_XmlReader_GetNodeType(a_pXmlReader) == OpcUa_XmlReader_NodeType_Element)
    {
        OpcUa_Boolean bEmptyElement = OpcUa_False;

        uStatus = OpcUa_XmlReader_IsEmptyElement(a_pXmlReader, &bEmptyElement);
        OpcUa_GotoErrorIfBad(uStatus);

        uStatus = OpcUa_XmlReader_Read(a_pXmlReader);
        OpcUa_GotoErrorIfBad(uStatus);

        if(OpcUa_XmlReader_GetNodeType(a_pXmlReader) == OpcUa_XmlReader_NodeType_EndElement)
        {
            return OpcUa_String_AttachReadOnly(a_pValue, "");
        }
    }
    
    uStatus = OpcUa_String_AttachReadOnly(a_pValue, "");
    OpcUa_GotoErrorIfBad(uStatus);

    while(OpcUa_XmlReader_NodeType_IsTextual(OpcUa_XmlReader_GetNodeType(a_pXmlReader)))
    {
        uStatus = OpcUa_String_StrnCat(
            a_pValue,
            OpcUa_String_FromCString(OpcUa_XmlReader_GetValue(a_pXmlReader)),
            OPCUA_STRINGLENZEROTERMINATED);
        OpcUa_GotoErrorIfBad(uStatus);

        uStatus = OpcUa_XmlReader_Read(a_pXmlReader);
        OpcUa_GotoErrorIfTrue(OpcUa_IsBad(uStatus) && uStatus != OpcUa_BadNoDataAvailable, uStatus);

        if(uStatus == OpcUa_BadNoDataAvailable)
        {
            uStatus = OpcUa_Good;
            break;
        }
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_String_Clear(a_pValue);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlReader_ReadInnerXml
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlReader_ReadInnerXml(
    struct _OpcUa_XmlReader*    a_pXmlReader,
    OpcUa_XmlElement*           a_pInnerXml)
{
    OpcUa_XmlReader_NodeType    eNodeType       = OpcUa_XmlReader_NodeType_None;
    OpcUa_OutputStream*         pOutputStream   = OpcUa_Null;
    OpcUa_XmlWriter*            pXmlWriter      = OpcUa_Null;
    OpcUa_Byte*                 pBufferData     = OpcUa_Null;
    OpcUa_UInt32                uBufferLength   = 0;
    
OpcUa_InitializeStatus(OpcUa_Module_XmlReader, "OpcUa_XmlReader_ReadInnerXml");

    OpcUa_ReturnErrorIfArgumentNull(a_pXmlReader);
    OpcUa_ReturnErrorIfArgumentNull(a_pInnerXml);

    OpcUa_XmlElement_Initialize(a_pInnerXml);

    eNodeType = OpcUa_XmlReader_GetNodeType(a_pXmlReader);

    if(    eNodeType != OpcUa_XmlReader_NodeType_Element 
        && eNodeType != OpcUa_XmlReader_NodeType_Attribute)  
    {
        return OpcUa_XmlReader_Read(a_pXmlReader);        
    }

    uStatus = OpcUa_MemoryStream_CreateWriteable(4096, 0, &pOutputStream);
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = OpcUa_XmlWriter_Create(&pXmlWriter, pOutputStream);
    OpcUa_GotoErrorIfBad(uStatus);

    if(eNodeType == OpcUa_XmlReader_NodeType_Attribute)
    {
        uStatus = OpcUa_XmlWriter_WriteString(pXmlWriter, OpcUa_XmlReader_GetValue(a_pXmlReader));
        OpcUa_GotoErrorIfBad(uStatus);
    }
    else if(eNodeType == OpcUa_XmlReader_NodeType_Element)
    {
        uStatus = OpcUa_XmlWriter_WriteNode(pXmlWriter, a_pXmlReader);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    uStatus = OpcUa_XmlWriter_Close(pXmlWriter);
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = pOutputStream->Close((OpcUa_Stream*)pOutputStream);
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = OpcUa_MemoryStream_GetBuffer(pOutputStream, &pBufferData, &uBufferLength);
    OpcUa_GotoErrorIfBad(uStatus);

    a_pInnerXml->Data = (OpcUa_Byte*)OpcUa_Alloc(uBufferLength);
    OpcUa_GotoErrorIfAllocFailed(a_pInnerXml->Data);

    a_pInnerXml->Length = uBufferLength;

    uStatus = OpcUa_MemCpy(a_pInnerXml->Data,
                           a_pInnerXml->Length,
                           pBufferData,
                           uBufferLength);
    OpcUa_GotoErrorIfBad(uStatus);

    OpcUa_XmlWriter_Delete(&pXmlWriter);
    OpcUa_Stream_Delete((OpcUa_Stream**)&pOutputStream);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_XmlWriter_Delete(&pXmlWriter);
    OpcUa_Stream_Delete((OpcUa_Stream**)&pOutputStream);
    OpcUa_XmlElement_Clear(a_pInnerXml);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlReader_ReadOuterXml
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlReader_ReadOuterXml(
    struct _OpcUa_XmlReader*    a_pXmlReader,
    OpcUa_XmlElement*           a_pOuterXml)
{
    OpcUa_XmlReader_NodeType    eNodeType       = OpcUa_XmlReader_NodeType_None;
    OpcUa_OutputStream*         pOutputStream   = OpcUa_Null;
    OpcUa_XmlWriter*            pXmlWriter      = OpcUa_Null;
    OpcUa_Byte*                 pBufferData     = OpcUa_Null;
    OpcUa_UInt32                uBufferLength   = 0;
    
OpcUa_InitializeStatus(OpcUa_Module_XmlReader, "OpcUa_XmlReader_ReadOuterXml");

    OpcUa_ReturnErrorIfArgumentNull(a_pXmlReader);
    OpcUa_ReturnErrorIfArgumentNull(a_pOuterXml);

    OpcUa_XmlElement_Initialize(a_pOuterXml);

    eNodeType = OpcUa_XmlReader_GetNodeType(a_pXmlReader);

    if(    eNodeType != OpcUa_XmlReader_NodeType_Element 
        && eNodeType != OpcUa_XmlReader_NodeType_Attribute)  
    {
        return OpcUa_XmlReader_Read(a_pXmlReader);        
    }

    uStatus = OpcUa_MemoryStream_CreateWriteable(4096, 0, &pOutputStream);
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = OpcUa_XmlWriter_Create(&pXmlWriter, pOutputStream);
    OpcUa_GotoErrorIfBad(uStatus);

    if(eNodeType == OpcUa_XmlReader_NodeType_Attribute)
    {
        uStatus = OpcUa_XmlWriter_WriteAttribute(
            pXmlWriter,
            OpcUa_XmlReader_GetPrefix(a_pXmlReader),
            OpcUa_XmlReader_GetLocalName(a_pXmlReader),
            OpcUa_XmlReader_GetNamespaceUri(a_pXmlReader),
            OpcUa_XmlReader_GetValue(a_pXmlReader));
        OpcUa_GotoErrorIfBad(uStatus);
    }
    else if(eNodeType == OpcUa_XmlReader_NodeType_Element)
    {
        uStatus = OpcUa_XmlWriter_WriteNode(pXmlWriter, a_pXmlReader);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    uStatus = OpcUa_XmlWriter_Close(pXmlWriter);
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = pOutputStream->Close((OpcUa_Stream*)pOutputStream);
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = OpcUa_MemoryStream_GetBuffer(pOutputStream, &pBufferData, &uBufferLength);
    OpcUa_GotoErrorIfBad(uStatus);

    a_pOuterXml->Data = (OpcUa_Byte*)OpcUa_Alloc(uBufferLength);
    OpcUa_GotoErrorIfAllocFailed(a_pOuterXml->Data);

    a_pOuterXml->Length = uBufferLength;

    uStatus = OpcUa_MemCpy(a_pOuterXml->Data,
                           a_pOuterXml->Length,
                           pBufferData,
                           uBufferLength);
    OpcUa_GotoErrorIfBad(uStatus);

    OpcUa_XmlWriter_Delete(&pXmlWriter);
    OpcUa_Stream_Delete((OpcUa_Stream**)&pOutputStream);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_XmlWriter_Delete(&pXmlWriter);
    OpcUa_Stream_Delete((OpcUa_Stream**)&pOutputStream);
    OpcUa_XmlElement_Clear(a_pOuterXml);

OpcUa_FinishErrorHandling;
}

#endif /* OPCUA_HAVE_XMLAPI */
