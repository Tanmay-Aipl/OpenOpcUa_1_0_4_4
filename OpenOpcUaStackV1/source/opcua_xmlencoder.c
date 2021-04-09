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
// New file from RCL project
/* core */
#include <opcua.h>
#include <opcua_mutex.h>

/* types */
#include <opcua_p_binary.h>

/* self */
#include <opcua_xmlencoder.h>
#include <opcua_binaryencoder.h>
#include <opcua_binaryencoderinternal.h>

/*============================================================================
 * OpcUa_XmlEncoder
 *
 * Stores the state of a memory stream.
 *
 * Ostrm  - The stream to write data to.
 * Closed - Whether the encoder has been closed.
 *===========================================================================*/
typedef struct _OpcUa_XmlEncoder
{
    OpcUa_UInt32          SanityCheck;
    OpcUa_OutputStream*   Ostrm;
    OpcUa_MessageContext* Context;
    OpcUa_Boolean         Closed;
    OpcUa_Mutex           Mutex;
}
OpcUa_XmlEncoder;

/*============================================================================
 * OpcUa_XmlEncoder_SanityCheck
 *
 * The sanity check reduces the likely hood of a fatal error caused by 
 * casting a bad handle to a OpcUa_XmlEncoder structure. The value 
 * was created by generating a new guid and taking the first for bytes.
 *===========================================================================*/
#define OpcUa_XmlEncoder_SanityCheck 0x0E0FA495

/*============================================================================
 * OpcUa_XmlEncoder_VerifyState
 *===========================================================================*/
#define OpcUa_XmlEncoder_VerifyState(xType) \
OpcUa_ReturnErrorIfInvalidObject(OpcUa_XmlEncoder, a_pEncoder, Write##xType); \
pHandle = (OpcUa_XmlEncoder*)a_pEncoder->Handle; \
OpcUa_ReturnErrorIfArgumentNull(pHandle); \
OpcUa_ReturnErrorIfTrue(pHandle->Closed, OpcUa_BadInvalidState);

/*============================================================================
 * OpcUa_Encode_FixedLengthType
 *===========================================================================*/
#define OpcUa_Encode_FixedLengthType(xType) \
{ \
    OpcUa_##xType##_Wire oValue; \
    uStatus = OpcUa_##xType##_P_NativeToWire(&oValue, &a_nValue); \
    OpcUa_GotoErrorIfBad(uStatus); \
    \
    uStatus = a_pOstrm->Write(a_pOstrm, (OpcUa_Byte*)&oValue, sizeof(OpcUa_##xType##_Wire)); \
    OpcUa_GotoErrorIfBad(uStatus); \
}

/*============================================================================
 * Begin_OpcUa_XmlEncoder_Write
 *===========================================================================*/
#define Begin_OpcUa_XmlEncoder_Write(xType) \
OpcUa_XmlEncoder* pHandle = OpcUa_Null; \
\
OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_Write" #xType); \
\
OpcUa_ReturnErrorIfArgumentNull(a_pEncoder); \
OpcUa_ReturnErrorIfArgumentNull(a_pValue); \
OpcUa_ReferenceParameter(a_sFieldName); \
OpcUa_XmlEncoder_VerifyState(xType); 

/*============================================================================
 * End_OpcUa_XmlEncoder_Write
 *===========================================================================*/
#define End_OpcUa_XmlEncoder_Write(xType) \
OpcUa_ReturnStatusCode; \
OpcUa_BeginErrorHandling; \
OpcUa_FinishErrorHandling;

/*============================================================================
 * Implement_OpcUa_XmlEncoder_WriteFixedLengthType
 *===========================================================================*/
#define Implement_OpcUa_XmlEncoder_WriteFixedLengthType(xType) \
Begin_OpcUa_XmlEncoder_Write(xType) \
\
if (a_pSize == OpcUa_Null) \
{ \
    uStatus = OpcUa_##xType##_XmlEncode(*a_pValue, pHandle->Ostrm); \
    OpcUa_GotoErrorIfBad(uStatus);  \
} \
else \
{ \
    *a_pSize = sizeof(OpcUa_##xType##_Wire); \
} \
\
End_OpcUa_XmlEncoder_Write(xType);

/*============================================================================
 * OpcUa_GetSize_FixedLengthArrayType
 *===========================================================================*/
#define OpcUa_GetSize_FixedLengthArrayType(xType) \
if (a_pSize != OpcUa_Null) \
{ \
    OpcUa_Int32 iSize = 0; \
 \
    *a_pSize = -1; \
    iSize += sizeof(OpcUa_Int32); \
 \
    if (a_pArray != OpcUa_Null) \
    { \
        iSize += sizeof(OpcUa_##xType)*a_nCount; \
    } \
 \
    *a_pSize = iSize; \
 \
    OpcUa_ReturnStatusCode; \
} 

/*============================================================================
 * OpcUa_GetSize_VariableLengthArrayType
 *===========================================================================*/
#define OpcUa_GetSize_VariableLengthArrayType(xType) \
if (a_pSize != OpcUa_Null) \
{ \
    OpcUa_Int32 ii = 0; \
    OpcUa_Int32 iSize = 0; \
 \
    *a_pSize = -1; \
    iSize += sizeof(OpcUa_Int32); \
 \
    if (a_pArray != OpcUa_Null) \
    { \
        for (ii = 0; ii < a_nCount; ii++) \
        { \
            OpcUa_Int32 iElementSize = 0; \
            uStatus = OpcUa_XmlEncoder_Write##xType(a_pEncoder, OpcUa_Null, &(((OpcUa_##xType*)a_pArray)[ii]), &iElementSize); \
            OpcUa_GotoErrorIfBad(uStatus); \
            iSize += iElementSize; \
        } \
    } \
 \
    *a_pSize = iSize; \
 \
    OpcUa_ReturnStatusCode; \
} 

/*============================================================================
 * OpcUa_Encode_ArrayType
 *===========================================================================*/
#define OpcUa_Encode_ArrayType(xType) \
{ \
    OpcUa_Int32 ii = 0; \
    OpcUa_Int32 iLength = -1; \
    \
    iLength = a_nCount; \
    if (iLength <= 0) \
    { \
        uStatus = OpcUa_XmlEncoder_WriteInt32(a_pEncoder, OpcUa_Null, &iLength, OpcUa_Null); \
        OpcUa_GotoErrorIfBad(uStatus); \
        OpcUa_ReturnStatusCode; \
    } \
    \
    OpcUa_GotoErrorIfTrue((a_pArray == OpcUa_Null), OpcUa_BadInvalidArgument); \
    \
    uStatus = OpcUa_XmlEncoder_WriteInt32(a_pEncoder, OpcUa_Null, &iLength, OpcUa_Null); \
    OpcUa_GotoErrorIfBad(uStatus); \
    \
    if (pHandle->Context->MaxArrayLength > 0 && iLength > (OpcUa_Int32)pHandle->Context->MaxArrayLength) \
    { \
        OpcUa_GotoErrorWithStatus(OpcUa_BadEncodingError); \
    } \
    \
    for (ii = 0; ii < iLength; ii++) \
    { \
        uStatus = OpcUa_XmlEncoder_Write##xType(a_pEncoder, OpcUa_Null, &(((OpcUa_##xType*)a_pArray)[ii]), OpcUa_Null); \
        OpcUa_GotoErrorIfBad(uStatus); \
    } \
}

/*============================================================================
 * OpcUa_XmlEncoder_Open
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_Open(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_OutputStream*    a_pOstrm,
    OpcUa_MessageContext*  a_pContext,
    OpcUa_Handle*          a_phEncodeContext)
{
    struct _OpcUa_Encoder* pEncodeContext = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_Open");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pOstrm);
    OpcUa_ReturnErrorIfArgumentNull(a_pContext);
    OpcUa_ReturnErrorIfArgumentNull(a_phEncodeContext);

    OpcUa_ReturnErrorIfInvalidObject(OpcUa_XmlEncoder, a_pEncoder, Open);

    *a_phEncodeContext = OpcUa_Null;

    OpcUa_Mutex_Lock(((OpcUa_XmlEncoder*)a_pEncoder->Handle)->Mutex);

    OpcUa_GotoErrorIfArgumentNull((OpcUa_XmlEncoder*)a_pEncoder->Handle);
    OpcUa_GotoErrorIfTrue(!((OpcUa_XmlEncoder*)a_pEncoder->Handle)->Closed, OpcUa_BadInvalidState);

    /* create handle */
    pEncodeContext = (struct _OpcUa_Encoder*)OpcUa_Alloc(sizeof(struct _OpcUa_Encoder));
    OpcUa_GotoErrorIfAllocFailed(pEncodeContext);
    OpcUa_MemCpy(pEncodeContext, sizeof(struct _OpcUa_Encoder), a_pEncoder, sizeof(struct _OpcUa_Encoder));

    pEncodeContext->Handle = OpcUa_Alloc(sizeof(OpcUa_XmlEncoder));
    OpcUa_GotoErrorIfAllocFailed(pEncodeContext->Handle);    
    
    ((OpcUa_XmlEncoder*)pEncodeContext->Handle)->SanityCheck = ((OpcUa_XmlEncoder*)a_pEncoder->Handle)->SanityCheck;
    ((OpcUa_XmlEncoder*)pEncodeContext->Handle)->Ostrm       = a_pOstrm;
    ((OpcUa_XmlEncoder*)pEncodeContext->Handle)->Context     = a_pContext;
    ((OpcUa_XmlEncoder*)pEncodeContext->Handle)->Closed      = OpcUa_False;
    ((OpcUa_XmlEncoder*)pEncodeContext->Handle)->Mutex       = OpcUa_Null;

    OpcUa_Mutex_Unlock(((OpcUa_XmlEncoder*)a_pEncoder->Handle)->Mutex);

    *a_phEncodeContext = pEncodeContext;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_Mutex_Unlock(((OpcUa_XmlEncoder*)a_pEncoder->Handle)->Mutex);

    if(pEncodeContext != OpcUa_Null)
    {
        if(pEncodeContext->Handle != OpcUa_Null)
        {
            OpcUa_Free(pEncodeContext->Handle);
        }
        OpcUa_Free(pEncodeContext);
    }

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_Close
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_Close(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_Handle*          a_phEncodeContext)
{
    struct _OpcUa_Encoder* pEncoderContext = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_Close");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_phEncodeContext);
    OpcUa_ReturnErrorIfArgumentNull(*a_phEncodeContext);
    OpcUa_ReturnErrorIfInvalidObject(OpcUa_XmlEncoder, a_pEncoder, Close);

    pEncoderContext = (struct _OpcUa_Encoder*)*a_phEncodeContext;

    OpcUa_Free(pEncoderContext->Handle);
    OpcUa_Free(pEncoderContext);

    *a_phEncodeContext = OpcUa_Null;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    /* nothing to do */

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_Delete
 *===========================================================================*/
OpcUa_Void OpcUa_XmlEncoder_Delete(
    struct _OpcUa_Encoder** a_ppEncoder)
{
    if (a_ppEncoder != OpcUa_Null && *a_ppEncoder != OpcUa_Null)
    {
        OpcUa_XmlEncoder* pHandle = (OpcUa_XmlEncoder*)(*a_ppEncoder)->Handle;

        OpcUa_Mutex_Delete(&pHandle->Mutex);

        OpcUa_Free(pHandle);
        
        OpcUa_Free(*a_ppEncoder);
        *a_ppEncoder = OpcUa_Null;
    }
}

/*============================================================================
 * OpcUa_XmlEncoder_PushNamespace
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_PushNamespace(
    struct _OpcUa_Encoder* a_pEncoder, 
    OpcUa_String*          a_sNamespaceUri)
{
    OpcUa_ReferenceParameter(a_pEncoder);
    OpcUa_ReferenceParameter(a_sNamespaceUri);
    
    /* not used in the Xml encoding */ 
    
    return OpcUa_Good;
}

/*============================================================================
 * OpcUa_XmlEncoder_PopNamespace
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_PopNamespace(
    struct _OpcUa_Encoder* a_pEncoder)
{
    OpcUa_ReferenceParameter(a_pEncoder);
    
    /* not used in the Xml encoding */ 
    
    return OpcUa_Good;
}

/*============================================================================
 * OpcUa_Boolean_XmlEncode
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Boolean_XmlEncode(
    OpcUa_Boolean       a_nValue,
    OpcUa_OutputStream* a_pOstrm)
{
    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_Boolean_XmlEncode");

    OpcUa_ReturnErrorIfArgumentNull(a_pOstrm);

    OpcUa_Encode_FixedLengthType(Boolean);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteBoolean
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteBoolean(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Boolean*         a_pValue,
    OpcUa_Int32*           a_pSize)
{
    Implement_OpcUa_XmlEncoder_WriteFixedLengthType(Boolean);
}

/*============================================================================
 * OpcUa_SByte_XmlEncode
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SByte_XmlEncode(
    OpcUa_SByte         a_nValue,
    OpcUa_OutputStream* a_pOstrm)
{
    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_SByte_XmlEncode");

    OpcUa_ReturnErrorIfArgumentNull(a_pOstrm);

    OpcUa_Encode_FixedLengthType(SByte);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteSByte
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteSByte(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_SByte*           a_pValue,
    OpcUa_Int32*           a_pSize)
{
    Implement_OpcUa_XmlEncoder_WriteFixedLengthType(SByte);
}

/*============================================================================
 * OpcUa_Byte_XmlEncode
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Byte_XmlEncode(
    OpcUa_Byte          a_nValue,
    OpcUa_OutputStream* a_pOstrm)
{
    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_Byte_XmlEncode");

    OpcUa_ReturnErrorIfArgumentNull(a_pOstrm);

    OpcUa_Encode_FixedLengthType(Byte);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteByte
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteByte(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Byte*            a_pValue,
    OpcUa_Int32*           a_pSize)
{    
    Implement_OpcUa_XmlEncoder_WriteFixedLengthType(SByte);
}

/*============================================================================
 * OpcUa_Int16_XmlEncode
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Int16_XmlEncode(
    OpcUa_Int16         a_nValue,
    OpcUa_OutputStream* a_pOstrm)
{
    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_Int16_XmlEncode");

    OpcUa_ReturnErrorIfArgumentNull(a_pOstrm);

    OpcUa_Encode_FixedLengthType(Int16);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteInt16
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteInt16(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Int16*           a_pValue,
    OpcUa_Int32*           a_pSize)
{   
    Implement_OpcUa_XmlEncoder_WriteFixedLengthType(Int16);
}

/*============================================================================
 * OpcUa_UInt16_XmlEncode
 *===========================================================================*/
OpcUa_StatusCode OpcUa_UInt16_XmlEncode(
    OpcUa_UInt16        a_nValue,
    OpcUa_OutputStream* a_pOstrm)
{
    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_UInt16_XmlEncode");

    OpcUa_ReturnErrorIfArgumentNull(a_pOstrm);

    OpcUa_Encode_FixedLengthType(UInt16);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteUInt16
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteUInt16(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_UInt16*          a_pValue,
    OpcUa_Int32*           a_pSize)
{
    Implement_OpcUa_XmlEncoder_WriteFixedLengthType(UInt16);
}

/*============================================================================
 * OpcUa_Int32_XmlEncode
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Int32_XmlEncode(
    OpcUa_Int32         a_nValue,
    OpcUa_OutputStream* a_pOstrm)
{
    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_Int32_XmlEncode");

    OpcUa_ReturnErrorIfArgumentNull(a_pOstrm);

    OpcUa_Encode_FixedLengthType(Int32);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteInt32
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteInt32(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Int32*           a_pValue,
    OpcUa_Int32*           a_pSize)
{
    Implement_OpcUa_XmlEncoder_WriteFixedLengthType(Int32);
}

/*============================================================================
 * OpcUa_UInt32_XmlEncode
 *===========================================================================*/
OpcUa_StatusCode OpcUa_UInt32_XmlEncode(
    OpcUa_UInt32        a_nValue,
    OpcUa_OutputStream* a_pOstrm)
{
    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_UInt32_XmlEncode");

    OpcUa_ReturnErrorIfArgumentNull(a_pOstrm);

    OpcUa_Encode_FixedLengthType(UInt32);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteUInt32
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteUInt32(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_UInt32*          a_pValue,
    OpcUa_Int32*           a_pSize)
{
    Implement_OpcUa_XmlEncoder_WriteFixedLengthType(UInt32);
}

/*============================================================================
 * OpcUa_Int64_XmlEncode
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Int64_XmlEncode(
    OpcUa_Int64         a_nValue,
    OpcUa_OutputStream* a_pOstrm)
{
    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_Int64_XmlEncode");

    OpcUa_ReturnErrorIfArgumentNull(a_pOstrm);

    OpcUa_Encode_FixedLengthType(Int64);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteInt64
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteInt64(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Int64*           a_pValue,
    OpcUa_Int32*           a_pSize)
{
    Implement_OpcUa_XmlEncoder_WriteFixedLengthType(Int64);
}

/*============================================================================
 * OpcUa_UInt64_XmlEncode
 *===========================================================================*/
OpcUa_StatusCode OpcUa_UInt64_XmlEncode(
    OpcUa_UInt64        a_nValue,
    OpcUa_OutputStream* a_pOstrm)
{
    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_UInt64_XmlEncode");

    OpcUa_ReturnErrorIfArgumentNull(a_pOstrm);

    OpcUa_Encode_FixedLengthType(UInt64);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteUInt64
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteUInt64(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_UInt64*          a_pValue,
    OpcUa_Int32*           a_pSize)
{
    Implement_OpcUa_XmlEncoder_WriteFixedLengthType(UInt64);
}

/*============================================================================
 * OpcUa_Float_XmlEncode
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Float_XmlEncode(
    OpcUa_Float         a_nValue,
    OpcUa_OutputStream* a_pOstrm)
{
    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_Float_XmlEncode");

    OpcUa_ReturnErrorIfArgumentNull(a_pOstrm);

    OpcUa_Encode_FixedLengthType(Float);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteFloat
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteFloat(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Float*           a_pValue,
    OpcUa_Int32*           a_pSize)
{
    Implement_OpcUa_XmlEncoder_WriteFixedLengthType(Float);
}

/*============================================================================
 * OpcUa_Double_XmlEncode
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Double_XmlEncode(
    OpcUa_Double        a_nValue,
    OpcUa_OutputStream* a_pOstrm)
{
    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_Double_XmlEncode");

    OpcUa_ReturnErrorIfArgumentNull(a_pOstrm);

    OpcUa_Encode_FixedLengthType(Double);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteDouble
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteDouble(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Double*          a_pValue,
    OpcUa_Int32*           a_pSize)
{
    Implement_OpcUa_XmlEncoder_WriteFixedLengthType(Double);
}

/*============================================================================
 * OpcUa_String_XmlEncode
 *===========================================================================*/
OpcUa_StatusCode OpcUa_String_XmlEncode(
    OpcUa_String*       a_pValue,
    OpcUa_OutputStream* a_pOstrm)
{
    OpcUa_Int32 nLength = -1;

OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_String_XmlEncode");

    OpcUa_ReturnErrorIfArgumentNull(a_pValue);
    OpcUa_ReturnErrorIfArgumentNull(a_pOstrm);
    
    /* encode a length of -1 to indicate a null string. */
    if(OpcUa_String_IsNull(a_pValue))
    {
        uStatus = OpcUa_Int32_XmlEncode(nLength, a_pOstrm);
        OpcUa_GotoErrorIfBad(uStatus);
        OpcUa_ReturnStatusCode;
    }

    /* determine length of string in bytes (excluding null terminator) */
    nLength = (OpcUa_Int32)OpcUa_String_StrSize(a_pValue);

    /* encode length of string */
    uStatus = OpcUa_Int32_XmlEncode(nLength, a_pOstrm);
    OpcUa_GotoErrorIfBad(uStatus);

    /* write bytes of string */
    uStatus = a_pOstrm->Write(a_pOstrm, (OpcUa_Byte*)OpcUa_String_GetRawString(a_pValue), nLength);
    OpcUa_GotoErrorIfBad(uStatus);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    /* nothing to do */

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteString
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteString(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_String*          a_pValue,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteString");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);

    OpcUa_ReferenceParameter(a_sFieldName);

    OpcUa_XmlEncoder_VerifyState(String);

   
    if(a_pSize != OpcUa_Null)
    { 
        /* calculate encoded size of string */
        *a_pSize = sizeof(OpcUa_Int32_Wire);

        if (!OpcUa_String_IsNull(a_pValue))
        {
            *a_pSize += (OpcUa_Int32)OpcUa_String_StrSize(a_pValue);
        }

        OpcUa_ReturnStatusCode;
    }
    else
    {
        /* encode string */
        uStatus = OpcUa_String_XmlEncode(a_pValue, pHandle->Ostrm);
        OpcUa_GotoErrorIfBad(uStatus);
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    /* nothing to do */

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_DateTime_XmlEncode
 *===========================================================================*/
OpcUa_StatusCode OpcUa_DateTime_XmlEncode(
    OpcUa_DateTime*     a_pValue,
    OpcUa_OutputStream* a_pOstrm)
{
    OpcUa_Int64 nBuffer = 0;
    OpcUa_Int64 nValue  = 0;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_DateTime_XmlEncode");

    OpcUa_ReturnErrorIfArgumentNull(a_pValue);
    OpcUa_ReturnErrorIfArgumentNull(a_pOstrm);
    
    /* convert structure to Int64 */
    nBuffer = (OpcUa_Int64)a_pValue->dwHighDateTime;

    if (nBuffer < 0) 
    { 
        nBuffer += (((OpcUa_Int64)OpcUa_Int32_Max)+1); 
    } 

    nValue = (nBuffer<<32);

    nBuffer = (OpcUa_Int64)a_pValue->dwLowDateTime;

    if (nBuffer < 0)
    {       
        nBuffer += (((OpcUa_Int64)OpcUa_Int32_Max)+1); 
    }

    nValue += nBuffer;

    /* write value */
    uStatus = OpcUa_Int64_XmlEncode(nValue, a_pOstrm);
    OpcUa_GotoErrorIfBad(uStatus);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteDateTime
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteDateTime(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_DateTime*        a_pValue,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteDateTime");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(DateTime);

    /* calculate size of date/time */
    if (a_pSize != OpcUa_Null)
    {
        *a_pSize = sizeof(OpcUa_Int64_Wire);
        OpcUa_ReturnStatusCode;
    }

    /* encode date/time */
    uStatus = OpcUa_DateTime_XmlEncode(a_pValue, pHandle->Ostrm);
    OpcUa_GotoErrorIfBad(uStatus);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Guid_XmlEncode
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Guid_XmlEncode(
    OpcUa_Guid*         a_pValue,
    OpcUa_OutputStream* a_pOstrm)
{
    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_Guid_XmlEncode");

    OpcUa_ReturnErrorIfArgumentNull(a_pValue);
    OpcUa_ReturnErrorIfArgumentNull(a_pOstrm);
    
    uStatus = OpcUa_UInt32_XmlEncode(a_pValue->Data1, a_pOstrm);
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = OpcUa_UInt16_XmlEncode(a_pValue->Data2, a_pOstrm);
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = OpcUa_UInt16_XmlEncode(a_pValue->Data3, a_pOstrm);
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = a_pOstrm->Write(a_pOstrm, (OpcUa_Byte*)a_pValue->Data4, sizeof(a_pValue->Data4));
    OpcUa_GotoErrorIfBad(uStatus);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteGuid
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteGuid(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Guid*            a_pValue,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteGuid");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(Guid);

    /* calculate size of date/time */
    if (a_pSize != OpcUa_Null)
    {
        *a_pSize  = sizeof(OpcUa_UInt32_Wire);
        *a_pSize += sizeof(OpcUa_UInt16_Wire);
        *a_pSize += sizeof(OpcUa_UInt16_Wire);
        *a_pSize += sizeof(a_pValue->Data4);

        OpcUa_ReturnStatusCode;
    }

    /* encode date/time */
    uStatus = OpcUa_Guid_XmlEncode(a_pValue, pHandle->Ostrm);
    OpcUa_GotoErrorIfBad(uStatus);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteByteString
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_ByteString_XmlEncode(
    OpcUa_ByteString*   a_pValue,
    OpcUa_OutputStream* a_pOstrm)
{
    OpcUa_Int32 nLength = -1;

OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_String_XmlEncode");

    OpcUa_ReturnErrorIfArgumentNull(a_pValue);
    OpcUa_ReturnErrorIfArgumentNull(a_pOstrm);
   
    /* determine length of byte string */
    if (a_pValue->Length >= 0 && a_pValue->Data != OpcUa_Null)
    {
        nLength = a_pValue->Length;
    }

    /* encode the length of the byte string */
    uStatus = OpcUa_Int32_XmlEncode(nLength, a_pOstrm);
    OpcUa_GotoErrorIfBad(uStatus);

    /* encode the bytes */
    if (a_pValue->Data != OpcUa_Null)
    {
        uStatus = a_pOstrm->Write(a_pOstrm, a_pValue->Data, nLength);
        OpcUa_ReturnErrorIfBad(uStatus);
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    /* nothing to do */

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteByteString
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteByteString(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_ByteString*      a_pValue,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_Int32 nLength = -1;
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteByteString");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(ByteString);

    /* determine length of byte string */
    if (a_pValue->Length >= 0 && a_pValue->Data != OpcUa_Null)
    {
        nLength = a_pValue->Length;
    }

    /* calculate size */
    if (a_pSize != OpcUa_Null)
    {
        *a_pSize = sizeof(OpcUa_Int32);

        if (nLength > 0)
        {
            *a_pSize += nLength;
        }

        OpcUa_ReturnStatusCode;
    }

    uStatus = OpcUa_ByteString_XmlEncode(a_pValue, pHandle->Ostrm);
    OpcUa_GotoErrorIfBad(uStatus);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteXmlElement
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteXmlElement(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_XmlElement*      a_pValue,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteXmlElement");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(XmlElement);

    uStatus = OpcUa_XmlEncoder_WriteByteString(a_pEncoder, a_sFieldName, a_pValue, a_pSize);
    OpcUa_GotoErrorIfBad(uStatus);
    
    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_NodeId_GetEncodingType
 *===========================================================================*/
/** @brief Returns the 8-bit identifer that precedes a UA Xml encoded node id. */
static OpcUa_NodeEncoding OpcUa_NodeId_GetEncodingType(OpcUa_NodeId* a_pNodeId)
{
    OpcUa_NodeEncoding encodingType = (OpcUa_NodeEncoding)0;

    if(OpcUa_Null != a_pNodeId)
    {
        switch (a_pNodeId->IdentifierType)
        {
            case OpcUa_IdentifierType_String: { encodingType = OpcUa_NodeEncoding_String;     break; }
            case OpcUa_IdentifierType_Guid:   { encodingType = OpcUa_NodeEncoding_Guid;       break; }
            case OpcUa_IdentifierType_Opaque: { encodingType = OpcUa_NodeEncoding_ByteString; break; }

            default:
            case OpcUa_IdentifierType_Numeric:
            {
                if (a_pNodeId->NamespaceIndex == 0 && a_pNodeId->Identifier.Numeric <= OpcUa_Byte_Max)
                {
                    encodingType = OpcUa_NodeEncoding_TwoByte;
                    break;
                }

                if (a_pNodeId->NamespaceIndex <= OpcUa_Byte_Max && a_pNodeId->Identifier.Numeric <= OpcUa_UInt16_Max)
                {
                    encodingType = OpcUa_NodeEncoding_FourByte;
                    break;
                }

                encodingType = OpcUa_NodeEncoding_Numeric;
                break;
            }
        }
    }

    return encodingType;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteNodeIdBody
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_XmlEncoder_WriteNodeIdBody(
    OpcUa_Encoder*      a_pEncoder,
    OpcUa_NodeId*       a_pValue, 
    OpcUa_NodeEncoding  a_eEncodingType)
{
    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteNodeIdBody");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue); 
    
    switch (a_eEncodingType & OpcUa_NodeEncoding_TypeMask)
    {
        case OpcUa_NodeEncoding_TwoByte:
        {
            OpcUa_Byte nIdentifier = (OpcUa_Byte)a_pValue->Identifier.Numeric;
            uStatus = OpcUa_XmlEncoder_WriteByte(a_pEncoder, OpcUa_Null, &nIdentifier, OpcUa_Null);
            OpcUa_GotoErrorIfBad(uStatus);
            break;
        }

        case OpcUa_NodeEncoding_FourByte:
        {
            OpcUa_Byte nNamespace = (OpcUa_Byte)a_pValue->NamespaceIndex;
            OpcUa_UInt16 nIdentifier = (OpcUa_UInt16)a_pValue->Identifier.Numeric;

            if((a_eEncodingType & OpcUa_NodeEncoding_UriMask) != 0)
            {
                nNamespace = 0;
            }

            uStatus = OpcUa_XmlEncoder_WriteByte(a_pEncoder, OpcUa_Null, &nNamespace, OpcUa_Null);
            OpcUa_GotoErrorIfBad(uStatus);

            uStatus = OpcUa_XmlEncoder_WriteUInt16(a_pEncoder, OpcUa_Null, &nIdentifier, OpcUa_Null);
            OpcUa_GotoErrorIfBad(uStatus);
            break;
        }

        case OpcUa_NodeEncoding_Numeric:
        {
            OpcUa_UInt16 nNamespace = a_pValue->NamespaceIndex;
            OpcUa_UInt32 nIdentifier = a_pValue->Identifier.Numeric;

            if((a_eEncodingType & OpcUa_NodeEncoding_UriMask) != 0)
            {
                nNamespace = 0;
            }

            uStatus = OpcUa_XmlEncoder_WriteUInt16(a_pEncoder, OpcUa_Null, &nNamespace, OpcUa_Null);
            OpcUa_GotoErrorIfBad(uStatus);

            uStatus = OpcUa_XmlEncoder_WriteUInt32(a_pEncoder, OpcUa_Null, &nIdentifier, OpcUa_Null);
            OpcUa_GotoErrorIfBad(uStatus);
            break;
        }

        case OpcUa_NodeEncoding_String:
        {
            OpcUa_UInt16 nNamespace = a_pValue->NamespaceIndex;
            OpcUa_String sIdentifier = a_pValue->Identifier.String;

            if((a_eEncodingType & OpcUa_NodeEncoding_UriMask) != 0)
            {
                nNamespace = 0;
            }

            uStatus = OpcUa_XmlEncoder_WriteUInt16(a_pEncoder, OpcUa_Null, &nNamespace, OpcUa_Null);
            OpcUa_GotoErrorIfBad(uStatus);

            uStatus = OpcUa_XmlEncoder_WriteString(a_pEncoder, OpcUa_Null, &sIdentifier, OpcUa_Null);
            OpcUa_GotoErrorIfBad(uStatus);          
            break;
        }

        case OpcUa_NodeEncoding_Guid:
        {
            OpcUa_UInt16 nNamespace = a_pValue->NamespaceIndex;
            OpcUa_Guid* pIdentifier = a_pValue->Identifier.Guid;

            if((a_eEncodingType & OpcUa_NodeEncoding_UriMask) != 0)
            {
                nNamespace = 0;
            }

            uStatus = OpcUa_XmlEncoder_WriteUInt16(a_pEncoder, OpcUa_Null, &nNamespace, OpcUa_Null);
            OpcUa_GotoErrorIfBad(uStatus);

            uStatus = OpcUa_XmlEncoder_WriteGuid(a_pEncoder, OpcUa_Null, pIdentifier, OpcUa_Null);
            OpcUa_GotoErrorIfBad(uStatus);  
            break;
        }

        case OpcUa_NodeEncoding_ByteString:
        {
            OpcUa_UInt16 nNamespace = a_pValue->NamespaceIndex;
            OpcUa_ByteString* pIdentifier = &a_pValue->Identifier.ByteString;

            if((a_eEncodingType & OpcUa_NodeEncoding_UriMask) != 0)
            {
                nNamespace = 0;
            }

            uStatus = OpcUa_XmlEncoder_WriteUInt16(a_pEncoder, OpcUa_Null, &nNamespace, OpcUa_Null);
            OpcUa_GotoErrorIfBad(uStatus);

            uStatus = OpcUa_XmlEncoder_WriteByteString(a_pEncoder, OpcUa_Null, pIdentifier, OpcUa_Null);
            OpcUa_GotoErrorIfBad(uStatus);          
            break;
        }
    }   

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_NodeIdGetSize
 *===========================================================================*/
 OpcUa_StatusCode OpcUa_XmlEncoder_NodeIdGetSize(
    OpcUa_Encoder*     a_pEncoder,
    OpcUa_NodeId*      a_pValue, 
    OpcUa_NodeEncoding a_eEncodingType,
    OpcUa_Int32*       a_pSize)
{
    OpcUa_Int32 iSize = 0;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_NodeIdGetSize");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);
    OpcUa_ReturnErrorIfArgumentNull(a_pSize);

    *a_pSize = -1;

    iSize += sizeof(OpcUa_Byte);

    if (a_pValue == OpcUa_Null)
    {
        iSize += sizeof(OpcUa_Byte);
        OpcUa_ReturnStatusCode;
    }

    switch (a_eEncodingType & OpcUa_NodeEncoding_TypeMask)
    {
        case OpcUa_NodeEncoding_TwoByte:
        {
            iSize += sizeof(OpcUa_Byte);
            break;
        }

        case OpcUa_NodeEncoding_FourByte:
        {
            iSize += sizeof(OpcUa_Byte);
            iSize += sizeof(OpcUa_UInt16);
            break;
        }

        case OpcUa_NodeEncoding_Numeric:
        {
            iSize += sizeof(OpcUa_UInt16);
            iSize += sizeof(OpcUa_UInt32);
            break;
        }

        case OpcUa_NodeEncoding_String:
        {
            OpcUa_Int32 iFieldSize = 0;
            uStatus = OpcUa_XmlEncoder_WriteString(a_pEncoder, OpcUa_Null, &a_pValue->Identifier.String, &iFieldSize);
            OpcUa_GotoErrorIfBad(uStatus);

            iSize += sizeof(OpcUa_UInt16);
            iSize += iFieldSize;
            break;
        }

        case OpcUa_NodeEncoding_Guid:
        {
            iSize += sizeof(OpcUa_UInt16);
            iSize += sizeof(OpcUa_Guid);
            break;
        }

        case OpcUa_NodeEncoding_ByteString:
        {
            OpcUa_Int32 iFieldSize = 0;
            uStatus = OpcUa_XmlEncoder_WriteByteString(a_pEncoder, OpcUa_Null, &a_pValue->Identifier.ByteString, &iFieldSize);
            OpcUa_GotoErrorIfBad(uStatus);

            iSize += sizeof(OpcUa_UInt16);
            iSize += iFieldSize;
            break;
        }
    }

    *a_pSize = iSize;

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    *a_pSize = -1;

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteNodeId
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteNodeId(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_NodeId*          a_pValue,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;
    OpcUa_NodeEncoding eEncodingType = OpcUa_NodeId_GetEncodingType(a_pValue);
    OpcUa_Byte byEncodingType = (OpcUa_Byte)eEncodingType;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteNodeId");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(NodeId);

    if (a_pSize != OpcUa_Null)
    {
        uStatus = OpcUa_XmlEncoder_NodeIdGetSize(a_pEncoder, a_pValue, eEncodingType, a_pSize);
        OpcUa_GotoErrorIfBad(uStatus);
        OpcUa_ReturnStatusCode;
    }

    /* write the encoding byte */    
    uStatus = OpcUa_XmlEncoder_WriteByte(a_pEncoder, OpcUa_Null, &byEncodingType, OpcUa_Null);
    OpcUa_GotoErrorIfBad(uStatus);

    /* write node id body. */
    uStatus = OpcUa_XmlEncoder_WriteNodeIdBody(a_pEncoder, a_pValue, eEncodingType);
    OpcUa_GotoErrorIfBad(uStatus);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteExpandedNodeId
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteExpandedNodeId(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_ExpandedNodeId*  a_pValue,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;
    OpcUa_Byte uEncodingType = 0;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteExpandedNodeId");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(ExpandedNodeId);

    uEncodingType = (OpcUa_Byte)OpcUa_NodeId_GetEncodingType(&a_pValue->NodeId);
            
    if((!OpcUa_String_IsNull(&(a_pValue->NamespaceUri))) && OpcUa_StrLen(&(a_pValue->NamespaceUri)) > 0)
    {
        /* set flag indicating uri is encoded. */
        uEncodingType |= OpcUa_NodeEncoding_UriMask;
    }

    /* set flag indicating server index is encoded. */
    if(a_pValue->ServerIndex > 0)
    {
        uEncodingType |= OpcUa_NodeEncoding_ServerIndexMask;
    }

    /* calculate size */
    if (a_pSize != OpcUa_Null)
    {
        OpcUa_Int32 iSize = 0;
        *a_pSize = -1;

        uStatus = OpcUa_XmlEncoder_NodeIdGetSize(a_pEncoder, &a_pValue->NodeId, (OpcUa_NodeEncoding)uEncodingType, &iSize);
        OpcUa_GotoErrorIfBad(uStatus);
        *a_pSize = iSize;

        if ((uEncodingType & OpcUa_NodeEncoding_UriMask) != 0)
        {
            uStatus = OpcUa_XmlEncoder_WriteString(a_pEncoder, OpcUa_Null, &a_pValue->NamespaceUri, &iSize);
            OpcUa_GotoErrorIfBad(uStatus);
            *a_pSize += iSize;
        }

        if ((uEncodingType & OpcUa_NodeEncoding_ServerIndexMask) != 0)
        {
            uStatus = OpcUa_XmlEncoder_WriteUInt32(a_pEncoder, OpcUa_Null, &a_pValue->ServerIndex, &iSize);
            OpcUa_GotoErrorIfBad(uStatus);
            *a_pSize += iSize;
        }

        OpcUa_ReturnStatusCode;
    }

    /* write the encoding byte */    
    uStatus = OpcUa_XmlEncoder_WriteByte(a_pEncoder, OpcUa_Null, &uEncodingType, OpcUa_Null);
    OpcUa_GotoErrorIfBad(uStatus);

    /* write node id body. */
    uStatus = OpcUa_XmlEncoder_WriteNodeIdBody(a_pEncoder, &a_pValue->NodeId, (OpcUa_NodeEncoding)uEncodingType);
    OpcUa_GotoErrorIfBad(uStatus);

    /* write namespace uri. */
    if ((uEncodingType & OpcUa_NodeEncoding_UriMask) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteString(a_pEncoder, OpcUa_Null, &a_pValue->NamespaceUri, OpcUa_Null);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* write server index. */
    if ((uEncodingType & OpcUa_NodeEncoding_ServerIndexMask) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteUInt32(a_pEncoder, OpcUa_Null, &a_pValue->ServerIndex, OpcUa_Null);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteStatusCode
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteStatusCode(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_StatusCode*      a_pValue,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteStatusCode");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(StatusCode);

    if (a_pSize != OpcUa_Null)
    {
        *a_pSize = sizeof(OpcUa_UInt32_Wire);
        OpcUa_ReturnStatusCode;
    }

    uStatus = OpcUa_UInt32_XmlEncode(*a_pValue, pHandle->Ostrm);
    OpcUa_GotoErrorIfBad(uStatus);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}
    
/*============================================================================
 * OpcUa_DiagnosticInfo_GetEncodingByte
 *===========================================================================*/
//OpcUa_Byte OpcUa_DiagnosticInfo_GetEncodingByte(OpcUa_DiagnosticInfo* a_pValue)
//{
//    OpcUa_Byte uEncodingByte = 0;
//
//    if (a_pValue == OpcUa_Null)
//    {
//        return uEncodingByte;
//    }
//
//    if (a_pValue->SymbolicId >= 0)
//    {
//        uEncodingByte |= OpcUa_DiagnosticInfo_EncodingByte_SymbolicId;
//    }
//
//    if (a_pValue->NamespaceUri >= 0)
//    {
//        uEncodingByte |= OpcUa_DiagnosticInfo_EncodingByte_NamespaceUri;
//    }
//
//    if (a_pValue->Locale >= 0)
//    {
//        uEncodingByte |= OpcUa_DiagnosticInfo_EncodingByte_Locale;
//    }
//
//    if (a_pValue->LocalizedText >= 0)
//    {
//        uEncodingByte |= OpcUa_DiagnosticInfo_EncodingByte_LocalizedText;
//    }
//
//    if(!OpcUa_String_IsNull(&(a_pValue->AdditionalInfo)))
//    {
//        uEncodingByte |= OpcUa_DiagnosticInfo_EncodingByte_AdditionalInfo;
//    }
//
//    if (a_pValue->InnerStatusCode != OpcUa_Good)
//    {
//        uEncodingByte |= OpcUa_DiagnosticInfo_EncodingByte_InnerStatusCode;
//    }
//
//    if (a_pValue->InnerDiagnosticInfo != OpcUa_Null)
//    {
//        uEncodingByte |= OpcUa_DiagnosticInfo_EncodingByte_InnerDiagnosticInfo;
//    }
//
//    return uEncodingByte;
//}

/*============================================================================
 * OpcUa_XmlEncoder_WriteDiagnosticInfo
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteDiagnosticInfo(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_DiagnosticInfo*  a_pValue,
    OpcUa_Int32*           a_pSize);

/*============================================================================
 * OpcUa_XmlEncoder_DiagnosticInfoGetSize
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_DiagnosticInfoGetSize(
    OpcUa_Encoder*        a_pEncoder,
    OpcUa_DiagnosticInfo* a_pValue, 
    OpcUa_Byte            a_uEncodingByte,
    OpcUa_Int32*          a_pSize)
{
    OpcUa_UInt32 iSize = 0;
    OpcUa_Int32 iFieldSize = 0;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_DiagnosticInfoGetSize");

    OpcUa_ReferenceParameter(a_pEncoder); 
    OpcUa_ReferenceParameter(a_pValue); 
    OpcUa_ReferenceParameter(a_pSize);

    *a_pSize = -1;

    if (a_pValue == OpcUa_Null)
    {
        *a_pSize = 0;
        OpcUa_ReturnStatusCode;
    }

    iSize += sizeof(OpcUa_Byte);
    
    if ((a_uEncodingByte & OpcUa_DiagnosticInfo_EncodingByte_SymbolicId) != 0)
    {
        iSize += sizeof(OpcUa_Int32);
    }

    if ((a_uEncodingByte & OpcUa_DiagnosticInfo_EncodingByte_NamespaceUri) != 0)
    {
        iSize += sizeof(OpcUa_Int32);
    }

    if ((a_uEncodingByte & OpcUa_DiagnosticInfo_EncodingByte_Locale) != 0)
    {
        iSize += sizeof(OpcUa_Int32);
    }

    if ((a_uEncodingByte & OpcUa_DiagnosticInfo_EncodingByte_LocalizedText) != 0)
    {
        iSize += sizeof(OpcUa_Int32);
    }

    if ((a_uEncodingByte & OpcUa_DiagnosticInfo_EncodingByte_AdditionalInfo) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteString(a_pEncoder, OpcUa_Null, &a_pValue->AdditionalInfo, &iFieldSize);
        OpcUa_GotoErrorIfBad(uStatus);
        iSize += iFieldSize;
    }

    if ((a_uEncodingByte & OpcUa_DiagnosticInfo_EncodingByte_InnerStatusCode) != 0)
    {
        iSize += sizeof(OpcUa_StatusCode);
    }

    if ((a_uEncodingByte & OpcUa_DiagnosticInfo_EncodingByte_InnerDiagnosticInfo) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteDiagnosticInfo(a_pEncoder, OpcUa_Null, a_pValue->InnerDiagnosticInfo, &iFieldSize);
        OpcUa_GotoErrorIfBad(uStatus);
        iSize += iFieldSize;
    }
    
    *a_pSize = iSize;

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    *a_pSize = -1;

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteDiagnosticInfo
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteDiagnosticInfo(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_DiagnosticInfo*  a_pValue,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;
    OpcUa_Byte uEncodingByte = OpcUa_DiagnosticInfo_GetEncodingByte(a_pValue);

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteDiagnosticInfo");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(DiagnosticInfo);

    /* calculate size */
    if (a_pSize != OpcUa_Null)
    {
        uStatus = OpcUa_XmlEncoder_DiagnosticInfoGetSize(a_pEncoder, a_pValue, uEncodingByte, a_pSize);
        OpcUa_GotoErrorIfBad(uStatus);
        OpcUa_ReturnStatusCode;
    }

    /* write encoding byte */
    uStatus = OpcUa_XmlEncoder_WriteByte(a_pEncoder, OpcUa_Null, &uEncodingByte, OpcUa_Null);
    OpcUa_GotoErrorIfBad(uStatus);

    /* write symbolic id */
    if ((uEncodingByte & OpcUa_DiagnosticInfo_EncodingByte_SymbolicId) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteInt32(a_pEncoder, OpcUa_Null, &a_pValue->SymbolicId, OpcUa_Null);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* write namespace uri */
    if ((uEncodingByte & OpcUa_DiagnosticInfo_EncodingByte_NamespaceUri) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteInt32(a_pEncoder, OpcUa_Null, &a_pValue->NamespaceUri, OpcUa_Null);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* write locale */
    if ((uEncodingByte & OpcUa_DiagnosticInfo_EncodingByte_Locale) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteInt32(a_pEncoder, OpcUa_Null, &a_pValue->Locale, OpcUa_Null);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* write localized text */
    if ((uEncodingByte & OpcUa_DiagnosticInfo_EncodingByte_LocalizedText) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteInt32(a_pEncoder, OpcUa_Null, &a_pValue->LocalizedText, OpcUa_Null);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* write additional info */
    if ((uEncodingByte & OpcUa_DiagnosticInfo_EncodingByte_AdditionalInfo) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteString(a_pEncoder, OpcUa_Null, &a_pValue->AdditionalInfo, OpcUa_Null);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* write inner status code */
    if ((uEncodingByte & OpcUa_DiagnosticInfo_EncodingByte_InnerStatusCode) != 0)
    {        
        uStatus = OpcUa_XmlEncoder_WriteStatusCode(a_pEncoder, OpcUa_Null, &a_pValue->InnerStatusCode, OpcUa_Null);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* write inner diagnostic info */
    if ((uEncodingByte & OpcUa_DiagnosticInfo_EncodingByte_InnerDiagnosticInfo) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteDiagnosticInfo(a_pEncoder, OpcUa_Null, a_pValue->InnerDiagnosticInfo, OpcUa_Null);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_LocalizedText_GetEncodingByte
 *===========================================================================*/
//OpcUa_Byte OpcUa_LocalizedText_GetEncodingByte(OpcUa_LocalizedText* a_pValue)
//{
//    OpcUa_Byte uEncodingByte = 0;
//
//    if (a_pValue == OpcUa_Null)
//    {
//        return uEncodingByte;
//    }
//
//    if(!OpcUa_String_IsNull(&(a_pValue->Locale)))
//    {
//        uEncodingByte |= OpcUa_LocalizedText_EncodingByte_Locale;
//    }
//
//    if(!OpcUa_String_IsNull(&(a_pValue->Text)))
//    {
//        uEncodingByte |= OpcUa_LocalizedText_EncodingByte_Text;
//    }
//
//    return uEncodingByte;
//}

/*============================================================================
 * OpcUa_XmlEncoder_LocalizedTextGetSize
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_LocalizedTextGetSize(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_LocalizedText*   a_pValue,
    OpcUa_Byte             a_uEncodingByte,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_Int32 iSize = 0;
    OpcUa_Int32 iFieldSize = 0;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_LocalizedTextGetSize");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);
    OpcUa_ReturnErrorIfArgumentNull(a_pSize);

    *a_pSize = -1;

    iSize += sizeof(OpcUa_Byte);

    if (a_pValue == OpcUa_Null)
    {
        *a_pSize = iSize;
        OpcUa_ReturnStatusCode;
    }
    
    if ((a_uEncodingByte & OpcUa_LocalizedText_EncodingByte_Locale) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteString(a_pEncoder, OpcUa_Null, &a_pValue->Locale, &iFieldSize);
        OpcUa_GotoErrorIfBad(uStatus);
        iSize += iFieldSize;
    }

    if ((a_uEncodingByte & OpcUa_LocalizedText_EncodingByte_Text) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteString(a_pEncoder, OpcUa_Null, &a_pValue->Text, &iFieldSize);
        OpcUa_GotoErrorIfBad(uStatus);
        iSize += iFieldSize;
    }

    *a_pSize = iSize;

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    *a_pSize = -1;

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteLocalizedText
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteLocalizedText(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_LocalizedText*   a_pValue,
    OpcUa_Int32*           a_pSize)
{   
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;
    OpcUa_Byte uEncodingByte = OpcUa_LocalizedText_GetEncodingByte(a_pValue);

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteLocalizedText");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(LocalizedText);

    /* calculate size */
    if (a_pSize != OpcUa_Null)
    {
        uStatus = OpcUa_XmlEncoder_LocalizedTextGetSize(a_pEncoder, a_pValue, uEncodingByte, a_pSize);
        OpcUa_GotoErrorIfBad(uStatus);
        OpcUa_ReturnStatusCode;
    }

    /* write encoding byte */
    uStatus = OpcUa_XmlEncoder_WriteByte(a_pEncoder, OpcUa_Null, &uEncodingByte, OpcUa_Null);
    OpcUa_GotoErrorIfBad(uStatus);

    /* write locale */
    if ((uEncodingByte & OpcUa_LocalizedText_EncodingByte_Locale) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteString(a_pEncoder, OpcUa_Null, &a_pValue->Locale, OpcUa_Null);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* write test */
    if ((uEncodingByte & OpcUa_LocalizedText_EncodingByte_Text) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteString(a_pEncoder, OpcUa_Null, &a_pValue->Text, OpcUa_Null);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteQualifiedName
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteQualifiedName(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_QualifiedName*   a_pValue,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_UInt32 iSize = 0;
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteQualifiedName");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(QualifiedName);

    if (a_pSize != OpcUa_Null)
    {        
        *a_pSize = -1;

        OpcUa_Field_GetSize(UInt16, NamespaceIndex);
        OpcUa_Field_GetSize(String, Name);
        
        *a_pSize = iSize;
    }
    else
    {
        OpcUa_Field_Write(UInt16, NamespaceIndex);
        OpcUa_Field_Write(String, Name);
    }

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteEncodeable
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteEncodeable(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Void*            a_pValue,
    OpcUa_EncodeableType*  a_pType,
    OpcUa_Int32*           a_pSize);

/*============================================================================
 * OpcUa_XmlEncoder_ExtensionObjectGetSize
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_ExtensionObjectGetSize(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_ExtensionObject* a_pValue,
    OpcUa_NodeId*          a_pTypeId,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_Int32 iSize = 0;
    OpcUa_Int32 iBodySize = 0;
    OpcUa_Int32 iFieldSize = 0;
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_ExtensionObjectGetSize");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);
    OpcUa_ReturnErrorIfArgumentNull(a_pTypeId);
    OpcUa_ReturnErrorIfArgumentNull(a_pSize);

    pHandle = (OpcUa_XmlEncoder*)a_pEncoder->Handle;
    OpcUa_ReturnErrorIfArgumentNull(pHandle);

    *a_pSize = -1;

    /* get any previously calculated body size. */
    iBodySize = a_pValue->BodySize;

    /* get size of type id */
    uStatus = OpcUa_XmlEncoder_WriteNodeId(a_pEncoder, OpcUa_Null, a_pTypeId, &iFieldSize);
    OpcUa_GotoErrorIfBad(uStatus);
    iSize += iFieldSize;

    /* get size of encoding byte */
    iSize += sizeof(OpcUa_Byte);

    /* check for empty object */
    if (a_pValue->Encoding == OpcUa_ExtensionObjectEncoding_None)
    {
        *a_pSize = iSize;
        a_pValue->BodySize = 0;
        return OpcUa_Good;
    }

    /* length of body */
    iSize += sizeof(OpcUa_Int32);

    /* check if the body size must always be re-recalculated. */
    if (pHandle->Context->AlwaysCheckLengths)
    {
        iBodySize = 0;
    }

    if (iBodySize == 0)
    {
        /* get size of byte string body */
        if (a_pValue->Encoding == OpcUa_ExtensionObjectEncoding_Xml)
        {
            uStatus = OpcUa_XmlEncoder_WriteByteString(a_pEncoder, OpcUa_Null, &a_pValue->Body.Xml, &iBodySize);
            OpcUa_GotoErrorIfBad(uStatus);

            iBodySize -= sizeof(OpcUa_Int32);
        }

        /* get size of byte string body */
        else if (a_pValue->Encoding == OpcUa_ExtensionObjectEncoding_Xml)
        {
            uStatus = OpcUa_XmlEncoder_WriteXmlElement(a_pEncoder, OpcUa_Null, &a_pValue->Body.Xml, &iBodySize);
            OpcUa_GotoErrorIfBad(uStatus);

            iBodySize -= sizeof(OpcUa_Int32);
        }

        /* get size of encodeable object body */
        else if (a_pValue->Encoding == OpcUa_ExtensionObjectEncoding_EncodeableObject)
        {   
            /* length of encoded object */
            uStatus = OpcUa_XmlEncoder_WriteEncodeable(
                a_pEncoder, 
                OpcUa_Null, 
                a_pValue->Body.EncodeableObject.Object, 
                a_pValue->Body.EncodeableObject.Type, 
                &iBodySize);

            OpcUa_GotoErrorIfBad(uStatus);
        }
    }
    else
    {
        a_pValue->BodySize = 0;
    }

    /* add body size */
    iSize += iBodySize;

    *a_pSize = iSize;

    /* update body size in object. */
    a_pValue->BodySize = iBodySize;

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    *a_pSize = -1;

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteExtensionObject
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteExtensionObject(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_ExtensionObject* a_pValue,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_ExpandedNodeId cTypeId;
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;
    OpcUa_Byte uEncodingByte = 0;

OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteExtensionObject");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(ExtensionObject);

    OpcUa_ExpandedNodeId_Initialize(&cTypeId);

    if (a_pValue->Encoding == OpcUa_ExtensionObjectEncoding_EncodeableObject)
    {
        OpcUa_ReturnErrorIfArgumentNull(a_pValue->Body.EncodeableObject.Type);
    }   

    /* make an updateable copy but don't duplicate the memory so clear does not need to be called */
    if (!OpcUa_ExpandedNodeId_IsNull(&a_pValue->TypeId))
    {
        OpcUa_MemCpy(&cTypeId, sizeof(OpcUa_ExpandedNodeId), &a_pValue->TypeId, sizeof(OpcUa_ExpandedNodeId));
    }

    /* initialize type id from encodeable object */
    else if (a_pValue->Encoding == OpcUa_ExtensionObjectEncoding_EncodeableObject)
    {
        OpcUa_EncodeableType* pType = a_pValue->Body.EncodeableObject.Type;

        cTypeId.NodeId.IdentifierType     = OpcUa_IdentifierType_Numeric;
        cTypeId.NodeId.Identifier.Numeric = pType->XmlEncodingTypeId;
        cTypeId.NodeId.NamespaceIndex     = 0;
        
        if (pType->NamespaceUri != OpcUa_Null)
        {
            uStatus = OpcUa_String_AttachReadOnly(&cTypeId.NamespaceUri, pType->NamespaceUri);
            OpcUa_GotoErrorIfBad(uStatus);
        }
    }

    /* lookup namespace index */
    if (!OpcUa_String_IsNull(&cTypeId.NamespaceUri))
    {
        OpcUa_Int32 iIndex = -1;
        uStatus = OpcUa_StringTable_FindIndex(pHandle->Context->NamespaceUris, &cTypeId.NamespaceUri, &iIndex);
        OpcUa_GotoErrorIfBad(uStatus);

        cTypeId.NodeId.NamespaceIndex = (OpcUa_UInt16)iIndex;
    }

    /* calculate size */
    if (a_pSize != OpcUa_Null)
    {
        *a_pSize = -1;
        uStatus = OpcUa_XmlEncoder_ExtensionObjectGetSize(a_pEncoder, a_pValue, &cTypeId.NodeId, a_pSize);
        OpcUa_GotoErrorIfBad(uStatus);
        OpcUa_ReturnStatusCode;
    }

    /* write type id */
    uStatus = OpcUa_XmlEncoder_WriteNodeId(a_pEncoder, OpcUa_Null, &cTypeId.NodeId, OpcUa_Null);
    OpcUa_GotoErrorIfBad(uStatus);
    
    /* write encoding byte */
    uEncodingByte = (OpcUa_Byte)a_pValue->Encoding;

    if (a_pValue->Encoding == OpcUa_ExtensionObjectEncoding_EncodeableObject)
    {
        uEncodingByte = OpcUa_ExtensionObjectEncoding_Xml;
    }

    uStatus = OpcUa_XmlEncoder_WriteByte(a_pEncoder, OpcUa_Null, &uEncodingByte, OpcUa_Null);
    OpcUa_GotoErrorIfBad(uStatus);

    if (a_pValue->Encoding == OpcUa_ExtensionObjectEncoding_EncodeableObject)
    {
        /* must pre-calculate size if stream does not support seeking */
        if (a_pValue->BodySize <= 0)
        {   
            uStatus = OpcUa_XmlEncoder_WriteEncodeable(
                a_pEncoder, 
                OpcUa_Null, 
                a_pValue->Body.EncodeableObject.Object, 
                a_pValue->Body.EncodeableObject.Type, 
                &a_pValue->BodySize);

            OpcUa_GotoErrorIfBad(uStatus);
        }

        /* write body size */
        uStatus = OpcUa_XmlEncoder_WriteInt32(a_pEncoder, OpcUa_Null, &a_pValue->BodySize, OpcUa_Null);
        OpcUa_GotoErrorIfBad(uStatus);

        /* write body */        
        uStatus = OpcUa_XmlEncoder_WriteEncodeable(
            a_pEncoder, 
            OpcUa_Null, 
            a_pValue->Body.EncodeableObject.Object, 
            a_pValue->Body.EncodeableObject.Type, 
            OpcUa_Null);
        
        OpcUa_GotoErrorIfBad(uStatus);
    }
    else if (a_pValue->Encoding == OpcUa_ExtensionObjectEncoding_Xml)
    {        
        /* write byte string body */
        uStatus = OpcUa_XmlEncoder_WriteByteString(a_pEncoder, OpcUa_Null, &a_pValue->Body.Xml, OpcUa_Null);
        OpcUa_GotoErrorIfBad(uStatus);
    }
    else if (a_pValue->Encoding == OpcUa_ExtensionObjectEncoding_Xml)
    {        
        /* write xml element body */
        uStatus = OpcUa_XmlEncoder_WriteXmlElement(a_pEncoder, OpcUa_Null, &a_pValue->Body.Xml, OpcUa_Null);
        OpcUa_GotoErrorIfBad(uStatus);
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_ExpandedNodeId_Clear(&cTypeId);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_DataValue_GetEncodingByte
 *===========================================================================*/
//OpcUa_Byte OpcUa_DataValue_GetEncodingByte(OpcUa_DataValue* a_pValue)
//{
//    OpcUa_Byte uEncodingByte = 0;
//
//    if (a_pValue == OpcUa_Null)
//    {
//        return uEncodingByte;
//    }
//
//    if (a_pValue->Value.Datatype != OpcUaType_Null)
//    {
//        uEncodingByte |= OpcUa_DataValue_EncodingByte_Value;
//    }
//
//    if (a_pValue->StatusCode != OpcUa_Good)
//    {
//        uEncodingByte |= OpcUa_DataValue_EncodingByte_StatusCode;
//    }
//
//    if (a_pValue->SourceTimestamp.dwHighDateTime != 0 || a_pValue->SourceTimestamp.dwLowDateTime  != 0)
//    {
//        uEncodingByte |= OpcUa_DataValue_EncodingByte_SourceTimestamp;
//    }
//
//    if (a_pValue->SourcePicoseconds != 0)
//    {
//        uEncodingByte |= OpcUa_DataValue_EncodingByte_SourcePicoseconds;
//    }
//
//    if (a_pValue->ServerTimestamp.dwHighDateTime != 0 || a_pValue->ServerTimestamp.dwLowDateTime  != 0)
//    {
//        uEncodingByte |= OpcUa_DataValue_EncodingByte_ServerTimestamp;
//    }
//
//    if (a_pValue->ServerPicoseconds != 0)
//    {
//        uEncodingByte |= OpcUa_DataValue_EncodingByte_ServerPicoseconds;
//    }
//
//    return uEncodingByte;
//}

/*============================================================================
 * OpcUa_XmlEncoder_WriteVariant
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteVariant(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Variant*         a_pValue,
    OpcUa_Int32*           a_pSize);

/*============================================================================
 * OpcUa_XmlEncoder_LocalizedTextGetSize
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_DataValueGetSize(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_DataValue*       a_pValue,
    OpcUa_Byte             a_uEncodingByte,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_Int32 iSize = 0;
    OpcUa_Int32 iFieldSize = 0;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_DataValueGetSize");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);
    OpcUa_ReturnErrorIfArgumentNull(a_pSize);

    *a_pSize = -1;

    iSize += sizeof(OpcUa_Byte);

    if (a_pValue == OpcUa_Null)
    {
        *a_pSize = iSize;
        OpcUa_ReturnStatusCode;
    }
    
    /* write value */
    if ((a_uEncodingByte & OpcUa_DataValue_EncodingByte_Value) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteVariant(a_pEncoder, OpcUa_Null, &a_pValue->Value, &iFieldSize);
        OpcUa_GotoErrorIfBad(uStatus);
        iSize += iFieldSize;
    }

    /* write status code */
    if ((a_uEncodingByte & OpcUa_DataValue_EncodingByte_StatusCode) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteStatusCode(a_pEncoder, OpcUa_Null, &a_pValue->StatusCode, &iFieldSize);
        OpcUa_GotoErrorIfBad(uStatus);
        iSize += iFieldSize;
    }

    /* write source timestamp */
    if ((a_uEncodingByte & OpcUa_DataValue_EncodingByte_SourceTimestamp) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteDateTime(a_pEncoder, OpcUa_Null, &a_pValue->SourceTimestamp, &iFieldSize);
        OpcUa_GotoErrorIfBad(uStatus);
        iSize += iFieldSize;
    }

    /* write source picoseconds */
    if ((a_uEncodingByte & OpcUa_DataValue_EncodingByte_SourcePicoseconds) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteUInt16(a_pEncoder, OpcUa_Null, &a_pValue->SourcePicoseconds, &iFieldSize);
        OpcUa_GotoErrorIfBad(uStatus);
        iSize += iFieldSize;
    }

    /* write server timestamp */
    if ((a_uEncodingByte & OpcUa_DataValue_EncodingByte_ServerTimestamp) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteDateTime(a_pEncoder, OpcUa_Null, &a_pValue->ServerTimestamp, &iFieldSize);
        OpcUa_GotoErrorIfBad(uStatus);
        iSize += iFieldSize;
    }

    /* write server picoseconds */
    if ((a_uEncodingByte & OpcUa_DataValue_EncodingByte_ServerPicoseconds) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteUInt16(a_pEncoder, OpcUa_Null, &a_pValue->ServerPicoseconds, &iFieldSize);
        OpcUa_GotoErrorIfBad(uStatus);
        iSize += iFieldSize;
    }

    *a_pSize = iSize;

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    *a_pSize = -1;

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteDataValue
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteDataValue(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_DataValue*       a_pValue,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;
    OpcUa_Byte uEncodingByte = OpcUa_DataValue_GetEncodingByte(a_pValue);

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteDataValue");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(DataValue);
    
    /* calculate size */
    if (a_pSize != OpcUa_Null)
    {
        uStatus = OpcUa_XmlEncoder_DataValueGetSize(a_pEncoder, a_pValue, uEncodingByte, a_pSize);
        OpcUa_GotoErrorIfBad(uStatus);
        OpcUa_ReturnStatusCode;
    }

    /* write encoding byte */
    uStatus = OpcUa_XmlEncoder_WriteByte(a_pEncoder, OpcUa_Null, &uEncodingByte, OpcUa_Null);
    OpcUa_GotoErrorIfBad(uStatus);

    /* write value */
    if ((uEncodingByte & OpcUa_DataValue_EncodingByte_Value) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteVariant(a_pEncoder, OpcUa_Null, &a_pValue->Value, OpcUa_Null);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* write status code */
    if ((uEncodingByte & OpcUa_DataValue_EncodingByte_StatusCode) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteStatusCode(a_pEncoder, OpcUa_Null, &a_pValue->StatusCode, OpcUa_Null);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* write source timestamp */
    if ((uEncodingByte & OpcUa_DataValue_EncodingByte_SourceTimestamp) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteDateTime(a_pEncoder, OpcUa_Null, &a_pValue->SourceTimestamp, OpcUa_Null);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* write source picoseconds */
    if ((uEncodingByte & OpcUa_DataValue_EncodingByte_SourcePicoseconds) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteUInt16(a_pEncoder, OpcUa_Null, &a_pValue->SourcePicoseconds, OpcUa_Null);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* write server timestamp */
    if ((uEncodingByte & OpcUa_DataValue_EncodingByte_ServerTimestamp) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteDateTime(a_pEncoder, OpcUa_Null, &a_pValue->ServerTimestamp, OpcUa_Null);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* write server picoseconds */
    if ((uEncodingByte & OpcUa_DataValue_EncodingByte_ServerPicoseconds) != 0)
    {
        uStatus = OpcUa_XmlEncoder_WriteUInt16(a_pEncoder, OpcUa_Null, &a_pValue->ServerPicoseconds, OpcUa_Null);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}
    
/*============================================================================
 * OpcUa_XmlEncoder_WriteEncodeable
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteEncodeable(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Void*            a_pValue,
    OpcUa_EncodeableType*  a_pType,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_Int32             iSize   = -1;
    OpcUa_XmlEncoder*    pHandle = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteEncodeable");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);
    OpcUa_ReturnErrorIfArgumentNull(a_pType);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(Encodeable);

    if (pHandle->Context->AlwaysCheckLengths || a_pSize != OpcUa_Null)
    {
        /* get the size of the encodeable object */
        uStatus = a_pType->GetSize(a_pValue, a_pEncoder, &iSize);   
        OpcUa_GotoErrorIfBad(uStatus);

        /* update size to return */
        if (a_pSize != OpcUa_Null)
        {
            *a_pSize = iSize;
        }
    }

    /* encode the object */
    if (a_pSize == OpcUa_Null)
    {
        uStatus = a_pType->Encode(a_pValue, a_pEncoder);    
        OpcUa_GotoErrorIfBad(uStatus);
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    if (a_pSize != OpcUa_Null)
    {
        *a_pSize = -1;
    }

OpcUa_FinishErrorHandling;
}
    
/*============================================================================
 * OpcUa_XmlEncoder_WriteEnumerated
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteEnumerated(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Int32*           a_pValue,
    OpcUa_EnumeratedType*  a_pType,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteEnumerated");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);
    OpcUa_ReturnErrorIfArgumentNull(a_pType);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(Enumerated);

    uStatus = OpcUa_XmlEncoder_WriteInt32(a_pEncoder, OpcUa_Null, a_pValue, a_pSize);
    OpcUa_GotoErrorIfBad(uStatus);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteBooleanArray
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteBooleanArray(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Boolean*         a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteBooleanArray");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(BooleanArray);

    OpcUa_GetSize_FixedLengthArrayType(Boolean);
    OpcUa_Encode_ArrayType(Boolean);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteSByteArray
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteSByteArray(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_SByte*           a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteSByteArray");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(SByteArray);

    OpcUa_GetSize_FixedLengthArrayType(SByte);
    OpcUa_Encode_ArrayType(SByte);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteByteArray
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteByteArray(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Byte*            a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteByteArray");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(ByteArray);

    OpcUa_GetSize_FixedLengthArrayType(Byte);
    OpcUa_Encode_ArrayType(Byte);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteInt16Array
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteInt16Array(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Int16*           a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteInt16Array");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(Int16Array);

    OpcUa_GetSize_FixedLengthArrayType(Int16);
    OpcUa_Encode_ArrayType(Int16);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteUInt16Array
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteUInt16Array(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_UInt16*          a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteUInt16Array");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(UInt16Array);

    OpcUa_GetSize_FixedLengthArrayType(UInt16);
    OpcUa_Encode_ArrayType(UInt16);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteInt32Array
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteInt32Array(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Int32*           a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteInt32Array");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(Int32Array);

    OpcUa_GetSize_FixedLengthArrayType(Int32);
    OpcUa_Encode_ArrayType(Int32);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteUInt32Array
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteUInt32Array(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_UInt32*          a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteUInt32Array");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(UInt32Array);

    OpcUa_GetSize_FixedLengthArrayType(UInt32);
    OpcUa_Encode_ArrayType(UInt32);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteInt64Array
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteInt64Array(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Int64*           a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteInt64Array");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(Int64Array);

    OpcUa_GetSize_FixedLengthArrayType(Int64);
    OpcUa_Encode_ArrayType(Int64);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteUInt64Array
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteUInt64Array(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_UInt64*          a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteUInt64Array");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(UInt64Array);

    OpcUa_GetSize_FixedLengthArrayType(UInt64);
    OpcUa_Encode_ArrayType(UInt64);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteFloatArray
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteFloatArray(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Float*           a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteFloatArray");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(FloatArray);

    OpcUa_GetSize_FixedLengthArrayType(Float);
    OpcUa_Encode_ArrayType(Float);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteDoubleArray
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteDoubleArray(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Double*          a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteDoubleArray");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(DoubleArray);

    OpcUa_GetSize_FixedLengthArrayType(Double);
    OpcUa_Encode_ArrayType(Double);
    
    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteStringArray
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteStringArray(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_String*          a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteStringArray");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(StringArray);

    OpcUa_GetSize_VariableLengthArrayType(String);
    OpcUa_Encode_ArrayType(String);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteDateTimeArray
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteDateTimeArray(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_DateTime*        a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteDateTimeArray");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(DateTimeArray);

    OpcUa_GetSize_FixedLengthArrayType(DateTime);
    OpcUa_Encode_ArrayType(DateTime);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteGuidArray
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteGuidArray(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Guid*            a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteGuidArray");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(GuidArray);

    OpcUa_GetSize_FixedLengthArrayType(Guid);
    OpcUa_Encode_ArrayType(Guid);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteByteStringArray
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteByteStringArray(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_ByteString*      a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteByteStringArray");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(ByteStringArray);

    OpcUa_GetSize_VariableLengthArrayType(ByteString);
    OpcUa_Encode_ArrayType(ByteString);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteXmlElementArray
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteXmlElementArray(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_XmlElement*      a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteXmlElementArray");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(XmlElementArray);

    OpcUa_GetSize_VariableLengthArrayType(XmlElement);
    OpcUa_Encode_ArrayType(XmlElement);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteNodeIdArray
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteNodeIdArray(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_NodeId*          a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteNodeIdArray");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(NodeIdArray);

    OpcUa_GetSize_VariableLengthArrayType(NodeId);
    OpcUa_Encode_ArrayType(NodeId);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteExpandedNodeIdArray
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteExpandedNodeIdArray(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_ExpandedNodeId*  a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteExpandedNodeIdArray");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(ExpandedNodeIdArray);

    OpcUa_GetSize_VariableLengthArrayType(ExpandedNodeId);
    OpcUa_Encode_ArrayType(ExpandedNodeId);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteStatusCodeArray
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteStatusCodeArray(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_StatusCode*      a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteStatusCodeArray");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(StatusCodeArray);

    OpcUa_GetSize_FixedLengthArrayType(StatusCode);
    OpcUa_Encode_ArrayType(StatusCode);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteDiagnosticInfoArray
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteDiagnosticInfoArray(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_DiagnosticInfo*  a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteDiagnosticInfoArray");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(DiagnosticInfoArray);

    OpcUa_GetSize_VariableLengthArrayType(DiagnosticInfo);
    OpcUa_Encode_ArrayType(DiagnosticInfo);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteLocalizedTextArray
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteLocalizedTextArray(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_LocalizedText*   a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteLocalizedTextArray");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(LocalizedTextArray);

    OpcUa_GetSize_VariableLengthArrayType(LocalizedText);
    OpcUa_Encode_ArrayType(LocalizedText);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteQualifiedNameArray
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteQualifiedNameArray(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_QualifiedName*   a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteQualifiedNameArray");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(QualifiedNameArray);

    OpcUa_GetSize_VariableLengthArrayType(QualifiedName);
    OpcUa_Encode_ArrayType(QualifiedName);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteExtensionObjectArray
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteExtensionObjectArray(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_ExtensionObject* a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteExtensionObjectArray");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(ExtensionObjectArray);

    OpcUa_GetSize_VariableLengthArrayType(ExtensionObject);
    OpcUa_Encode_ArrayType(ExtensionObject);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteDataValueArray
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteDataValueArray(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_DataValue*       a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteDataValueArray");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(DataValueArray);

    OpcUa_GetSize_VariableLengthArrayType(DataValue);
    OpcUa_Encode_ArrayType(DataValue);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteVariantArray
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteVariantArray(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Variant*         a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteVariantArray");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(VariantArray);

    OpcUa_GetSize_VariableLengthArrayType(Variant);
    OpcUa_Encode_ArrayType(Variant);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}
    
/*============================================================================
 * OpcUa_XmlEncoder_WriteEncodeableArray
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteEncodeableArray(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Void*            a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_EncodeableType*  a_pType,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_Int32 ii = 0;
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteEncodeableArray");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pType);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(EncodeableArray);

    /* calculate size */
    if (a_pSize != OpcUa_Null)
    { 
        OpcUa_Int32 iSize = 0; 
     
        *a_pSize = -1;
        iSize += sizeof(OpcUa_Int32);
     
        if (a_pArray != OpcUa_Null)
        {
            for (ii = 0; ii < a_nCount; ii++)
            { 
                OpcUa_Int32 iElementSize = 0;
                OpcUa_UInt32 uPosition = ii*a_pType->AllocationSize;

                uStatus = OpcUa_XmlEncoder_WriteEncodeable(
                    a_pEncoder, 
                    OpcUa_Null, 
                    &(((OpcUa_Byte*)a_pArray)[uPosition]), 
                    a_pType, 
                    &iElementSize);

                OpcUa_GotoErrorIfBad(uStatus);
                
                iSize += iElementSize;
            }
        }
    
        *a_pSize = iSize; 
     
        OpcUa_ReturnStatusCode;
    }

    /* write null array */
    if (a_pArray == OpcUa_Null)
    { 
        OpcUa_Int32 iLength = -1;
        uStatus = OpcUa_XmlEncoder_WriteInt32(a_pEncoder, OpcUa_Null, &iLength, OpcUa_Null);
        OpcUa_GotoErrorIfBad(uStatus);
        OpcUa_ReturnStatusCode; 
    } 

    /* write length of array */
    uStatus = OpcUa_XmlEncoder_WriteInt32(a_pEncoder, OpcUa_Null, &a_nCount, OpcUa_Null);
    OpcUa_GotoErrorIfBad(uStatus);
    
    /* write elements of array */
    for (ii = 0; ii < a_nCount; ii++)
    { 
        OpcUa_UInt32 uPosition = ii*a_pType->AllocationSize;

        uStatus = OpcUa_XmlEncoder_WriteEncodeable(
            a_pEncoder, 
            OpcUa_Null, 
            &(((OpcUa_Byte*)a_pArray)[uPosition]), 
            a_pType, 
            OpcUa_Null);

        OpcUa_GotoErrorIfBad(uStatus);
    }

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}
    
/*============================================================================
 * OpcUa_XmlEncoder_WriteEnumeratedArray
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteEnumeratedArray(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Int32*           a_pArray,
    OpcUa_Int32            a_nCount,
    OpcUa_EnumeratedType*  a_pType,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_Int32 ii = 0;
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteEnumeratedArray");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pType);
    OpcUa_ReferenceParameter(a_sFieldName);
    OpcUa_XmlEncoder_VerifyState(EnumeratedArray);
 
    /* calculate size */
    if (a_pSize != OpcUa_Null)
    { 
        OpcUa_Int32 iSize = 0; 
     
        *a_pSize = -1;
        iSize += sizeof(OpcUa_Int32);
     
        if (a_pArray != OpcUa_Null)
        {
            iSize += sizeof(OpcUa_Int32)*a_nCount;
        }
    
        *a_pSize = iSize; 
     
        OpcUa_ReturnStatusCode;
    }

    /* write null array */
    if (a_pArray == OpcUa_Null)
    { 
        OpcUa_Int32 iLength = -1;
        uStatus = OpcUa_XmlEncoder_WriteInt32(a_pEncoder, OpcUa_Null, &iLength, OpcUa_Null);
        OpcUa_GotoErrorIfBad(uStatus);
        OpcUa_ReturnStatusCode; 
    } 

    /* write length of array */
    uStatus = OpcUa_XmlEncoder_WriteInt32(a_pEncoder, OpcUa_Null, &a_nCount, OpcUa_Null);
    OpcUa_GotoErrorIfBad(uStatus);
    
    /* write elements of array */
    for (ii = 0; ii < a_nCount; ii++)
    { 
        uStatus = OpcUa_XmlEncoder_WriteInt32(a_pEncoder, OpcUa_Null, &(a_pArray[ii]), OpcUa_Null);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Variant_XmlEncode_ValueType
 *===========================================================================*/
/** @brief Helper macro for encoding a Variant of value type. */
#define OpcUa_Variant_XmlEncode_ValueType(xName) \
uStatus = OpcUa_XmlEncoder_Write##xName(a_pEncoder, OpcUa_Null, &a_pValue->Value.xName, pSize); \
OpcUa_GotoErrorIfBad(uStatus); \
if (pSize != OpcUa_Null) *a_pSize += *pSize;

/*============================================================================
 * OpcUa_Variant_XmlEncode_ReferenceType
 *===========================================================================*/
/** @brief Helper macro for encoding a Variant of reference type. */
#define OpcUa_Variant_XmlEncode_ReferenceType(xName) \
uStatus = OpcUa_XmlEncoder_Write##xName(a_pEncoder, OpcUa_Null, a_pValue->Value.xName, pSize); \
OpcUa_GotoErrorIfBad(uStatus); \
if (pSize != OpcUa_Null) *a_pSize += *pSize;

/*============================================================================
 * OpcUa_AnyArray_XmlEncode
 *===========================================================================*/
/** @brief Helper macro for encoding a Variant of array type. */
#define OpcUa_AnyArray_XmlEncode(xName) \
uStatus = OpcUa_XmlEncoder_Write##xName##Array(a_pEncoder, OpcUa_Null, pArray->xName##Array, iLength, pSize); \
OpcUa_GotoErrorIfBad(uStatus); \
if (pSize != OpcUa_Null) *a_pSize += *pSize;

/*============================================================================
 * OpcUa_XmlEncoder_WriteVariant
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteVariant(
    struct _OpcUa_Encoder* a_pEncoder,
    OpcUa_StringA          a_sFieldName,
    OpcUa_Variant*         a_pValue,
    OpcUa_Int32*           a_pSize)
{
    OpcUa_Int32 iSize = 0;
    OpcUa_Int32* pSize = OpcUa_Null;
    OpcUa_Byte uEncodingByte = 0;
    OpcUa_Int32 iLength = 0;
    OpcUa_VariantArrayUnion* pArray = 0;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteVariant");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfInvalidObject(OpcUa_XmlEncoder, a_pEncoder, WriteVariant);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);
    OpcUa_ReferenceParameter(a_sFieldName);

    if (a_pSize != OpcUa_Null)
    {
        *a_pSize = 0;
        pSize = &iSize;
    }

    /* create encoding byte */
    uEncodingByte = a_pValue->Datatype;

    if (a_pValue->ArrayType != OpcUa_VariantArrayType_Scalar)
    {
        uEncodingByte |= OpcUa_Variant_ArrayMask;

        if (a_pValue->ArrayType == OpcUa_VariantArrayType_Matrix)
        {
            uEncodingByte |= OpcUa_Variant_ArrayDimensionsMask;

            iLength = OpcUa_VariantMatrix_GetElementCount(&a_pValue->Value.Matrix);
            pArray  = &a_pValue->Value.Matrix.Value;
        }
        else
        {
            iLength = a_pValue->Value.Array.Length;
            pArray  = &a_pValue->Value.Array.Value;
        }
    }

    /* write encoding byte */
    uStatus = OpcUa_XmlEncoder_WriteByte(a_pEncoder, OpcUa_Null, &uEncodingByte, pSize);
    OpcUa_ReturnErrorIfBad(uStatus);
    if (pSize != OpcUa_Null) *a_pSize += *pSize;

    if (a_pValue->ArrayType != OpcUa_VariantArrayType_Scalar)
    {
        switch(a_pValue->Datatype)
        {
            case OpcUaType_Boolean:
            {
                OpcUa_AnyArray_XmlEncode(Boolean);
                break;
            }

            case OpcUaType_SByte:
            {
                OpcUa_AnyArray_XmlEncode(SByte);
                break;
            }

            case OpcUaType_Byte:
            {
                OpcUa_AnyArray_XmlEncode(Byte);
                break;
            }

            case OpcUaType_Int16:
            {
                OpcUa_AnyArray_XmlEncode(Int16);
                break;
            }

            case OpcUaType_UInt16:
            {
                OpcUa_AnyArray_XmlEncode(UInt16);
                break;
            }

            case OpcUaType_Int32:
            {
                OpcUa_AnyArray_XmlEncode(Int32);
                break;
            }

            case OpcUaType_UInt32:
            {
                OpcUa_AnyArray_XmlEncode(UInt32);
                break;
            }

            case OpcUaType_Int64:
            {
                OpcUa_AnyArray_XmlEncode(Int64);
                break;
            }

            case OpcUaType_UInt64:
            {
                OpcUa_AnyArray_XmlEncode(UInt64);
                break;
            }

            case OpcUaType_Float:
            {
                OpcUa_AnyArray_XmlEncode(Float);
                break;
            }

            case OpcUaType_Double:
            {
                OpcUa_AnyArray_XmlEncode(Double);
                break;
            }

            case OpcUaType_String:
            {
                OpcUa_AnyArray_XmlEncode(String);
                break;
            }

            case OpcUaType_DateTime:
            {
                OpcUa_AnyArray_XmlEncode(DateTime);
                break;
            }

            case OpcUaType_Guid:
            {
                OpcUa_AnyArray_XmlEncode(Guid);
                break;
            }

            case OpcUaType_ByteString:
            {
                OpcUa_AnyArray_XmlEncode(ByteString);
                break;
            }

            case OpcUaType_XmlElement:
            {
                OpcUa_AnyArray_XmlEncode(XmlElement);
                break;
            }

            case OpcUaType_NodeId:
            {
                OpcUa_AnyArray_XmlEncode(NodeId);
                break;
            }

            case OpcUaType_ExpandedNodeId:
            {
                OpcUa_AnyArray_XmlEncode(ExpandedNodeId);
                break;
            }

            case OpcUaType_StatusCode:
            {
                OpcUa_AnyArray_XmlEncode(StatusCode);
                break;
            }

            case OpcUaType_LocalizedText:
            {
                OpcUa_AnyArray_XmlEncode(LocalizedText);
                break;
            }

            case OpcUaType_QualifiedName:
            {
                OpcUa_AnyArray_XmlEncode(QualifiedName);
                break;
            }

            case OpcUaType_ExtensionObject:
            {
                OpcUa_AnyArray_XmlEncode(ExtensionObject);
                break;
            }

            case OpcUaType_DataValue:
            {
                OpcUa_AnyArray_XmlEncode(DataValue);
                break;
            }

            case OpcUaType_Variant:
            {
                OpcUa_AnyArray_XmlEncode(Variant);
                break;
            }

            default:
            {
                OpcUa_GotoErrorWithStatus(OpcUa_BadEncodingError);
            }
        }

        /* write the number of dimensions for matrix types. */
        if (a_pValue->ArrayType == OpcUa_VariantArrayType_Matrix)
        {
            uStatus = OpcUa_XmlEncoder_WriteInt32Array(
                a_pEncoder, 
                OpcUa_Null, 
                a_pValue->Value.Matrix.Dimensions, 
                a_pValue->Value.Matrix.NoOfDimensions, 
                pSize);

            OpcUa_GotoErrorIfBad(uStatus);
            
            if (pSize != OpcUa_Null) *a_pSize += *pSize;
        }
    }
    else
    {
        switch(a_pValue->Datatype)
        {
            case OpcUaType_Null:
            {
                break;
            }

            case OpcUaType_Boolean:
            {
                OpcUa_Variant_XmlEncode_ValueType(Boolean);
                break;
            }

            case OpcUaType_SByte:
            {
                OpcUa_Variant_XmlEncode_ValueType(SByte);
                break;
            }

            case OpcUaType_Byte:
            {
                OpcUa_Variant_XmlEncode_ValueType(Byte);
                break;
            }

            case OpcUaType_Int16:
            {
                OpcUa_Variant_XmlEncode_ValueType(Int16);
                break;
            }

            case OpcUaType_UInt16:
            {
                OpcUa_Variant_XmlEncode_ValueType(UInt16);
                break;
            }

            case OpcUaType_Int32:
            {
                OpcUa_Variant_XmlEncode_ValueType(Int32);
                break;
            }

            case OpcUaType_UInt32:
            {
                OpcUa_Variant_XmlEncode_ValueType(UInt32);
                break;
            }

            case OpcUaType_Int64:
            {
                OpcUa_Variant_XmlEncode_ValueType(Int64);
                break;
            }

            case OpcUaType_UInt64:
            {
                OpcUa_Variant_XmlEncode_ValueType(UInt64);
                break;
            }

            case OpcUaType_Float:
            {
                OpcUa_Variant_XmlEncode_ValueType(Float);
                break;
            }

            case OpcUaType_Double:
            {
                OpcUa_Variant_XmlEncode_ValueType(Double);
                break;
            }

            case OpcUaType_String:
            {
                OpcUa_Variant_XmlEncode_ValueType(String);
                break;
            }

            case OpcUaType_DateTime:
            {
                OpcUa_Variant_XmlEncode_ValueType(DateTime);
                break;
            }

            case OpcUaType_Guid:
            {
                OpcUa_Variant_XmlEncode_ReferenceType(Guid);
                break;
            }

            case OpcUaType_ByteString:
            {
                OpcUa_Variant_XmlEncode_ValueType(ByteString);
                break;
            }

            case OpcUaType_XmlElement:
            {
                OpcUa_Variant_XmlEncode_ValueType(XmlElement);
                break;
            }

            case OpcUaType_NodeId:
            {
                OpcUa_Variant_XmlEncode_ReferenceType(NodeId);
                break;
            }

            case OpcUaType_ExpandedNodeId:
            {
                OpcUa_Variant_XmlEncode_ReferenceType(ExpandedNodeId);
                break;
            }

            case OpcUaType_StatusCode:
            {
                OpcUa_Variant_XmlEncode_ValueType(StatusCode);
                break;
            }

            case OpcUaType_LocalizedText:
            {
                OpcUa_Variant_XmlEncode_ReferenceType(LocalizedText);
                break;
            }

            case OpcUaType_QualifiedName:
            {
                OpcUa_Variant_XmlEncode_ReferenceType(QualifiedName);
                break;
            }

            case OpcUaType_ExtensionObject:
            {
                OpcUa_Variant_XmlEncode_ReferenceType(ExtensionObject);
                break;
            }

            case OpcUaType_DataValue:
            {
                OpcUa_Variant_XmlEncode_ReferenceType(DataValue);
                break;
            }

            default:
            {
                OpcUa_GotoErrorWithStatus(OpcUa_BadEncodingError);
            }
        }
    }

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_WriteMessage
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_WriteMessage(
    OpcUa_Encoder*         a_pEncoder,
    OpcUa_Void*            a_pMessage,
    OpcUa_EncodeableType*  a_pMessageType)
{
    OpcUa_NodeId            cTypeId;
    OpcUa_XmlEncoder*    pHandle         = OpcUa_Null;
    OpcUa_MessageContext*   pContext        = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_WriteMessage");

    OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
    OpcUa_ReturnErrorIfArgumentNull(a_pMessage);
    OpcUa_ReturnErrorIfArgumentNull(a_pMessageType);

    pHandle = (OpcUa_XmlEncoder*)a_pEncoder->Handle;
    OpcUa_ReturnErrorIfArgumentNull(pHandle);
    OpcUa_ReturnErrorIfTrue(pHandle->Closed, OpcUa_BadInvalidState);
    pContext = pHandle->Context;
    OpcUa_ReturnErrorIfArgumentNull(pContext);

    /* create type id */
    cTypeId.IdentifierType     = OpcUa_IdentifierType_Numeric;
    cTypeId.Identifier.Numeric = a_pMessageType->XmlEncodingTypeId;
    cTypeId.NamespaceIndex     = 0;
    
    /* do not support non-UA messages */
    if (a_pMessageType->NamespaceUri != OpcUa_Null)
    {
        OpcUa_GotoErrorWithStatus(OpcUa_BadNotSupported);
    }

    /* write type id */
    uStatus = a_pEncoder->WriteNodeId(a_pEncoder, OpcUa_Null, &cTypeId, OpcUa_Null);
    OpcUa_GotoErrorIfBad(uStatus);
            
    /* write message */
    uStatus = OpcUa_XmlEncoder_WriteEncodeable(
        a_pEncoder, 
        OpcUa_Null, 
        a_pMessage, 
        a_pMessageType, 
        OpcUa_Null);

    OpcUa_GotoErrorIfBad(uStatus);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    /* nothing to do */

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_XmlEncoder_Create
 *===========================================================================*/
OpcUa_StatusCode OpcUa_XmlEncoder_Create(
    OpcUa_Encoder** a_ppEncoder)
{
    OpcUa_XmlEncoder* pHandle = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "OpcUa_XmlEncoder_Create");

    OpcUa_ReturnErrorIfArgumentNull(a_ppEncoder);

    *a_ppEncoder = OpcUa_Null;

    pHandle = (OpcUa_XmlEncoder*)OpcUa_Alloc(sizeof(OpcUa_XmlEncoder));
    OpcUa_GotoErrorIfAllocFailed(pHandle);
    OpcUa_MemSet(pHandle, 0, sizeof(OpcUa_XmlEncoder));

    pHandle->SanityCheck = OpcUa_XmlEncoder_SanityCheck;
    pHandle->Closed      = OpcUa_True;
    pHandle->Ostrm       = OpcUa_Null;
    pHandle->Context     = OpcUa_Null;

    uStatus = OpcUa_Mutex_Create(&pHandle->Mutex);
    OpcUa_ReturnErrorIfBad(uStatus);

    *a_ppEncoder = (OpcUa_Encoder*)OpcUa_Alloc(sizeof(OpcUa_Encoder));
    OpcUa_GotoErrorIfAllocFailed(*a_ppEncoder);
    OpcUa_MemSet(*a_ppEncoder, 0, sizeof(OpcUa_Encoder));

    (*a_ppEncoder)->Handle      = pHandle;
    (*a_ppEncoder)->EncoderType = OpcUa_EncoderType_Xml;

    (*a_ppEncoder)->Open                      = OpcUa_XmlEncoder_Open;
    (*a_ppEncoder)->Close                     = OpcUa_XmlEncoder_Close;
    (*a_ppEncoder)->Delete                    = OpcUa_XmlEncoder_Delete;
    (*a_ppEncoder)->PushNamespace             = OpcUa_XmlEncoder_PushNamespace;
    (*a_ppEncoder)->PopNamespace              = OpcUa_XmlEncoder_PopNamespace;
    (*a_ppEncoder)->WriteBoolean              = OpcUa_XmlEncoder_WriteBoolean;
    (*a_ppEncoder)->WriteSByte                = OpcUa_XmlEncoder_WriteSByte;
    (*a_ppEncoder)->WriteByte                 = OpcUa_XmlEncoder_WriteByte;
    (*a_ppEncoder)->WriteInt16                = OpcUa_XmlEncoder_WriteInt16;
    (*a_ppEncoder)->WriteUInt16               = OpcUa_XmlEncoder_WriteUInt16;
    (*a_ppEncoder)->WriteInt32                = OpcUa_XmlEncoder_WriteInt32;
    (*a_ppEncoder)->WriteUInt32               = OpcUa_XmlEncoder_WriteUInt32;
    (*a_ppEncoder)->WriteInt64                = OpcUa_XmlEncoder_WriteInt64;
    (*a_ppEncoder)->WriteUInt64               = OpcUa_XmlEncoder_WriteUInt64;
    (*a_ppEncoder)->WriteFloat                = OpcUa_XmlEncoder_WriteFloat;
    (*a_ppEncoder)->WriteDouble               = OpcUa_XmlEncoder_WriteDouble;
    (*a_ppEncoder)->WriteString               = OpcUa_XmlEncoder_WriteString;
    (*a_ppEncoder)->WriteDateTime             = OpcUa_XmlEncoder_WriteDateTime;
    (*a_ppEncoder)->WriteGuid                 = OpcUa_XmlEncoder_WriteGuid;
    (*a_ppEncoder)->WriteByteString           = OpcUa_XmlEncoder_WriteByteString;
    (*a_ppEncoder)->WriteXmlElement           = OpcUa_XmlEncoder_WriteXmlElement;
    (*a_ppEncoder)->WriteNodeId               = OpcUa_XmlEncoder_WriteNodeId;
    (*a_ppEncoder)->WriteExpandedNodeId       = OpcUa_XmlEncoder_WriteExpandedNodeId;
    (*a_ppEncoder)->WriteStatusCode           = OpcUa_XmlEncoder_WriteStatusCode;
    (*a_ppEncoder)->WriteDiagnosticInfo       = OpcUa_XmlEncoder_WriteDiagnosticInfo;
    (*a_ppEncoder)->WriteLocalizedText        = OpcUa_XmlEncoder_WriteLocalizedText;
    (*a_ppEncoder)->WriteQualifiedName        = OpcUa_XmlEncoder_WriteQualifiedName;
    (*a_ppEncoder)->WriteExtensionObject      = OpcUa_XmlEncoder_WriteExtensionObject;
    (*a_ppEncoder)->WriteDataValue            = OpcUa_XmlEncoder_WriteDataValue;
    (*a_ppEncoder)->WriteVariant              = OpcUa_XmlEncoder_WriteVariant;
    (*a_ppEncoder)->WriteEncodeable           = OpcUa_XmlEncoder_WriteEncodeable;
    (*a_ppEncoder)->WriteEnumerated           = OpcUa_XmlEncoder_WriteEnumerated;
    (*a_ppEncoder)->WriteBooleanArray         = OpcUa_XmlEncoder_WriteBooleanArray;
    (*a_ppEncoder)->WriteSByteArray           = OpcUa_XmlEncoder_WriteSByteArray;
    (*a_ppEncoder)->WriteByteArray            = OpcUa_XmlEncoder_WriteByteArray;
    (*a_ppEncoder)->WriteInt16Array           = OpcUa_XmlEncoder_WriteInt16Array;
    (*a_ppEncoder)->WriteUInt16Array          = OpcUa_XmlEncoder_WriteUInt16Array;
    (*a_ppEncoder)->WriteInt32Array           = OpcUa_XmlEncoder_WriteInt32Array;
    (*a_ppEncoder)->WriteUInt32Array          = OpcUa_XmlEncoder_WriteUInt32Array;
    (*a_ppEncoder)->WriteInt64Array           = OpcUa_XmlEncoder_WriteInt64Array;
    (*a_ppEncoder)->WriteUInt64Array          = OpcUa_XmlEncoder_WriteUInt64Array;
    (*a_ppEncoder)->WriteFloatArray           = OpcUa_XmlEncoder_WriteFloatArray;
    (*a_ppEncoder)->WriteDoubleArray          = OpcUa_XmlEncoder_WriteDoubleArray;
    (*a_ppEncoder)->WriteStringArray          = OpcUa_XmlEncoder_WriteStringArray;
    (*a_ppEncoder)->WriteDateTimeArray        = OpcUa_XmlEncoder_WriteDateTimeArray;
    (*a_ppEncoder)->WriteGuidArray            = OpcUa_XmlEncoder_WriteGuidArray;
    (*a_ppEncoder)->WriteByteStringArray      = OpcUa_XmlEncoder_WriteByteStringArray;
    (*a_ppEncoder)->WriteXmlElementArray      = OpcUa_XmlEncoder_WriteXmlElementArray;
    (*a_ppEncoder)->WriteNodeIdArray          = OpcUa_XmlEncoder_WriteNodeIdArray;
    (*a_ppEncoder)->WriteExpandedNodeIdArray  = OpcUa_XmlEncoder_WriteExpandedNodeIdArray;
    (*a_ppEncoder)->WriteStatusCodeArray      = OpcUa_XmlEncoder_WriteStatusCodeArray;
    (*a_ppEncoder)->WriteDiagnosticInfoArray  = OpcUa_XmlEncoder_WriteDiagnosticInfoArray;
    (*a_ppEncoder)->WriteLocalizedTextArray   = OpcUa_XmlEncoder_WriteLocalizedTextArray;
    (*a_ppEncoder)->WriteQualifiedNameArray   = OpcUa_XmlEncoder_WriteQualifiedNameArray;
    (*a_ppEncoder)->WriteExtensionObjectArray = OpcUa_XmlEncoder_WriteExtensionObjectArray;
    (*a_ppEncoder)->WriteDataValueArray       = OpcUa_XmlEncoder_WriteDataValueArray;
    (*a_ppEncoder)->WriteVariantArray         = OpcUa_XmlEncoder_WriteVariantArray;
    (*a_ppEncoder)->WriteEncodeableArray      = OpcUa_XmlEncoder_WriteEncodeableArray;
    (*a_ppEncoder)->WriteEnumeratedArray      = OpcUa_XmlEncoder_WriteEnumeratedArray;
    (*a_ppEncoder)->WriteMessage              = OpcUa_XmlEncoder_WriteMessage;
    
    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    OpcUa_Free(pHandle);
    OpcUa_Free(*a_ppEncoder);
    *a_ppEncoder = OpcUa_Null;

    OpcUa_FinishErrorHandling;
}
