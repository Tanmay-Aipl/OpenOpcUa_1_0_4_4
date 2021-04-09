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
#ifdef OPCUA_HAVE_MEMORYSTREAM

#define OPCUA_PREALLOC_MEMORYBLOCK OPCUA_CONFIG_YES

#include <opcua_buffer.h>
#include <opcua_memorystream.h>


/*============================================================================
 * OpcUa_MemoryStream
 *
 * Stores the state of a memory stream.
 *
 * Buffer     - The memory buffer that stores the stream data.
 * Closed     - Whether the stream has been closed.
 * FreeBuffer - Whether the buffer should be free when the stream is destroyed.
 *
 * If the buffer size is fixed then MaxSize is 0.
 *===========================================================================*/
typedef struct _OpcUa_MemoryStream
{
    OpcUa_UInt32  SanityCheck;
    OpcUa_Buffer* pBuffer;
    OpcUa_Void*   pUserData;
    OpcUa_Boolean Closed;
}
OpcUa_MemoryStream;

/*============================================================================
 * OpcUa_MemoryStream_SanityCheck
 *
 * The sanity check reduces the likely hood of a fatal error caused by 
 * casting a bad handle to a OpcUa_TcpConnection structure. The value 
 * was created by generating a new guid and taking the first for bytes.
 *===========================================================================*/
#define OpcUa_MemoryStream_SanityCheck 0x25B49A0E

/*============================================================================
 * OpcUa_ValidateMemoryStream
 *===========================================================================*/
#define OpcUa_ReturnErrorIfInvalidStream(xStrm, xMethod) \
if (((OpcUa_MemoryStream*)xStrm->Handle)->SanityCheck != OpcUa_MemoryStream_SanityCheck || xStrm->xMethod != OpcUa_MemoryStream_##xMethod) \
{ \
    return OpcUa_BadInvalidArgument; \
}

/*============================================================================
 * OpcUa_MemoryStream_Read
 *===========================================================================*/
OpcUa_StatusCode OpcUa_MemoryStream_Read(
    struct _OpcUa_InputStream*     istrm, 
    OpcUa_Byte*                    buffer, 
    OpcUa_UInt32*                  count, 
    OpcUa_Stream_PfnOnReadyToRead* callback, 
    OpcUa_Void*                    callbackData)
{
    OpcUa_MemoryStream* handle = OpcUa_Null;

    OpcUa_DeclareErrorTraceModule(OpcUa_Module_MemoryStream);

    OpcUa_ReturnErrorIfArgumentNull(istrm);
    OpcUa_ReturnErrorIfArgumentNull(buffer);
    OpcUa_ReturnErrorIfArgumentNull(count);
    OpcUa_ReturnErrorIfInvalidStream(istrm, Read);
    OpcUa_ReferenceParameter(callback);
    OpcUa_ReferenceParameter(callbackData);

    handle = (OpcUa_MemoryStream*)istrm->Handle;

    if (handle->Closed)
    {
        return OpcUa_BadInvalidState;
    }

    return OpcUa_Buffer_Read(handle->pBuffer, buffer, count);
}

/*============================================================================
 * OpcUa_MemoryStream_Write
 *===========================================================================*/
OpcUa_StatusCode OpcUa_MemoryStream_Write(
    struct _OpcUa_OutputStream* ostrm, 
    OpcUa_Byte*                 buffer, 
    OpcUa_UInt32                count)
{   
    OpcUa_MemoryStream* handle = OpcUa_Null;

    OpcUa_DeclareErrorTraceModule(OpcUa_Module_MemoryStream);

    OpcUa_ReturnErrorIfArgumentNull(ostrm);
    OpcUa_ReturnErrorIfArgumentNull(buffer);
    OpcUa_ReturnErrorIfInvalidStream(ostrm, Write);
    
    handle = (OpcUa_MemoryStream*)ostrm->Handle;

    if (handle->Closed)
    {
        return OpcUa_BadInvalidState;
    }

    return OpcUa_Buffer_Write(handle->pBuffer, buffer, count);
}


/*============================================================================
 * OpcUa_MemoryStream_Flush
 *===========================================================================*/
OpcUa_StatusCode OpcUa_MemoryStream_Flush(
    struct _OpcUa_OutputStream* ostrm,
    OpcUa_Boolean               lastCall)
{
    OpcUa_MemoryStream* handle = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_MemoryStream, "Flush");

    OpcUa_ReturnErrorIfArgumentNull(ostrm);
    OpcUa_ReturnErrorIfInvalidStream(ostrm, Flush);
    OpcUa_ReferenceParameter(lastCall);

    handle = (OpcUa_MemoryStream*)ostrm->Handle;

    if(handle->Closed)
    {
        return OpcUa_BadInvalidState;
    }

    if(!OpcUa_Buffer_IsEmpty((OpcUa_Buffer *)handle->pBuffer))
    {
        OpcUa_Byte*  pData          = OpcUa_Null;
        OpcUa_UInt32 uDataLength    = 0;

        uStatus = OpcUa_Buffer_GetData(handle->pBuffer, &pData, &uDataLength);
        
        if (OpcUa_IsBad(uStatus))
        {
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "Flush: FAILED with 0x%X\n", uStatus);
            OpcUa_GotoError;
        }
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_MemoryStream_Close
 *===========================================================================*/
OpcUa_StatusCode OpcUa_MemoryStream_Close(
    struct _OpcUa_Stream* strm)
{
    OpcUa_MemoryStream* handle  = OpcUa_Null;
    OpcUa_StatusCode    uStatus = OpcUa_Good;

    OpcUa_DeclareErrorTraceModule(OpcUa_Module_MemoryStream);

    OpcUa_ReturnErrorIfArgumentNull(strm);
    OpcUa_ReturnErrorIfInvalidStream(strm, Close);

    handle = (OpcUa_MemoryStream*)strm->Handle;

    if (handle->Closed)
    {
        return OpcUa_BadInvalidState;
    }

    if(strm->Type == OpcUa_StreamType_Output)
    {
        /* flush content when stream gets closed */
        uStatus = OpcUa_MemoryStream_Flush( (OpcUa_OutputStream*)strm,
                                            OpcUa_True);
    }

    handle->Closed = OpcUa_True;

    return uStatus;
}

/*============================================================================
 * OpcUa_MemoryStream_Delete
 *===========================================================================*/
OpcUa_Void OpcUa_MemoryStream_Delete(
    struct _OpcUa_Stream** strm)
{
    if (strm != OpcUa_Null && *strm != OpcUa_Null)
    {
        OpcUa_MemoryStream* handle = (OpcUa_MemoryStream*)(*strm)->Handle;

        if (handle != OpcUa_Null)
        {
            OpcUa_Buffer_Delete((OpcUa_Buffer**)&handle->pBuffer);
            OpcUa_Free(handle);
        }
        
        OpcUa_Free(*strm);
        *strm = OpcUa_Null;
    }
}

/*============================================================================
 * OpcUa_MemoryStream_GetPosition
 *===========================================================================*/
OpcUa_StatusCode OpcUa_MemoryStream_GetPosition(
    struct _OpcUa_Stream* strm, 
    OpcUa_UInt32*         position)
{
    OpcUa_MemoryStream* handle = OpcUa_Null;

    OpcUa_DeclareErrorTraceModule(OpcUa_Module_MemoryStream);

    OpcUa_ReturnErrorIfArgumentNull(strm);
    OpcUa_ReturnErrorIfArgumentNull(position);
    OpcUa_ReturnErrorIfInvalidStream(strm, GetPosition);

    handle = (OpcUa_MemoryStream*)strm->Handle;

    return OpcUa_Buffer_GetPosition(handle->pBuffer, position);
}

/*============================================================================
 * OpcUa_Stream_SetPosition
 *===========================================================================*/
OpcUa_StatusCode OpcUa_MemoryStream_SetPosition(
    struct _OpcUa_Stream* strm, 
    OpcUa_UInt32          position)
{
    OpcUa_MemoryStream* handle = OpcUa_Null;
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_MemoryStream);

    OpcUa_ReturnErrorIfArgumentNull(strm);
    OpcUa_ReturnErrorIfInvalidStream(strm, SetPosition);

    handle = (OpcUa_MemoryStream*)strm->Handle;

    if (handle->Closed)
    {
        return OpcUa_BadInvalidState;
    }

    return OpcUa_Buffer_SetPosition(handle->pBuffer, position);
}

/*============================================================================
 * OpcUa_MemoryStream_CreateWriteable
 *===========================================================================*/
OpcUa_StatusCode OpcUa_MemoryStream_AttachBuffer(   OpcUa_Stream*   a_pStrm,
                                                    OpcUa_Buffer*   a_pBuffer)
{
    OpcUa_MemoryStream* pMemoryStream = (OpcUa_MemoryStream*)a_pStrm->Handle;

    OpcUa_ReturnErrorIfArgumentNull(a_pStrm);
    OpcUa_ReturnErrorIfArgumentNull(a_pBuffer);

    OpcUa_Buffer_Clear(pMemoryStream->pBuffer);

    *(pMemoryStream->pBuffer) = *a_pBuffer;

    a_pBuffer->Data = OpcUa_Null;
    OpcUa_Buffer_Clear(a_pBuffer);
    
    return OpcUa_Good;
}

/*============================================================================
 * OpcUa_MemoryStream_CreateWriteable
 *===========================================================================*/
OpcUa_StatusCode OpcUa_MemoryStream_DetachBufferInterface(  OpcUa_Stream*   a_pStrm,
                                                            OpcUa_Buffer*   a_pBuffer,
                                                            OpcUa_Boolean*  a_bMoreData)
{
    OpcUa_MemoryStream* pMemoryStream = (OpcUa_MemoryStream*)a_pStrm->Handle;

    OpcUa_ReturnErrorIfArgumentNull(a_pStrm);
    OpcUa_ReturnErrorIfArgumentNull(a_pBuffer);

    if(a_bMoreData != OpcUa_Null)
    {
        *a_bMoreData = OpcUa_False;
    }

    *a_pBuffer = *(pMemoryStream->pBuffer);

    pMemoryStream->pBuffer->Data = OpcUa_Null;
    OpcUa_Buffer_Clear(pMemoryStream->pBuffer);

    return OpcUa_Good;
}

/*============================================================================
 * OpcUa_MemoryStream_CreateWriteable
 *===========================================================================*/
OpcUa_StatusCode OpcUa_MemoryStream_GetChunkLength( OpcUa_Stream*   a_pStrm, 
                                                    OpcUa_UInt32*   a_puLength)
{
    OpcUa_MemoryStream* pMemoryStream = OpcUa_Null;

    OpcUa_ReturnErrorIfArgumentNull(a_pStrm);
    OpcUa_ReturnErrorIfArgumentNull(a_puLength);

    pMemoryStream = (OpcUa_MemoryStream*)a_pStrm->Handle;

    *a_puLength = ((OpcUa_Buffer*)(pMemoryStream->pBuffer))->MaxSize;

    return OpcUa_Good;
}

/*============================================================================
 * OpcUa_MemoryStream_CreateReadable
 *===========================================================================*/
OpcUa_StatusCode OpcUa_MemoryStream_CreateReadable(
    const OpcUa_Byte*   buffer,
    OpcUa_UInt32        bufferSize,
    OpcUa_Boolean       freeBuffer,
    OpcUa_InputStream** istrm)
{
    OpcUa_StatusCode uStatus = OpcUa_Good;
    OpcUa_MemoryStream* handle = OpcUa_Null;

    OpcUa_DeclareErrorTraceModule(OpcUa_Module_MemoryStream);

    OpcUa_ReturnErrorIfNull(istrm, OpcUa_BadInvalidArgument);
    *istrm = OpcUa_Null;

    handle = (OpcUa_MemoryStream*)OpcUa_Alloc(sizeof(OpcUa_MemoryStream));
    OpcUa_GotoErrorIfAllocFailed(handle);
    OpcUa_MemSet(handle, 0, sizeof(OpcUa_MemoryStream));

    handle->SanityCheck = OpcUa_MemoryStream_SanityCheck;
    handle->pBuffer     = OpcUa_Null;
    handle->Closed      = OpcUa_False;

    /* Input stream never writes into buffer -> cast away const */
    uStatus = OpcUa_Buffer_Create((OpcUa_Byte*)buffer, bufferSize, bufferSize, bufferSize, freeBuffer, (OpcUa_Buffer**)&handle->pBuffer);
    OpcUa_GotoErrorIfBad(uStatus);

    *istrm = (OpcUa_InputStream*)OpcUa_Alloc(sizeof(OpcUa_InputStream));
    OpcUa_GotoErrorIfAllocFailed(*istrm);

    (*istrm)->Type              = OpcUa_StreamType_Input;
    (*istrm)->Handle            = handle;
    (*istrm)->GetPosition       = OpcUa_MemoryStream_GetPosition;
    (*istrm)->SetPosition       = OpcUa_MemoryStream_SetPosition;
    (*istrm)->Close             = OpcUa_MemoryStream_Close;
    (*istrm)->Delete            = OpcUa_MemoryStream_Delete;
    (*istrm)->Read              = OpcUa_MemoryStream_Read;
    (*istrm)->AttachBuffer      = OpcUa_MemoryStream_AttachBuffer;
    (*istrm)->DetachBuffer      = OpcUa_MemoryStream_DetachBufferInterface;
    (*istrm)->GetChunkLength    = OpcUa_MemoryStream_GetChunkLength;
    (*istrm)->NonBlocking       = OpcUa_False;
    (*istrm)->CanSeek           = OpcUa_True;

    return OpcUa_Good;

Error:

    if (handle != OpcUa_Null && handle->pBuffer != OpcUa_Null)
    {
        OpcUa_Buffer_Delete((OpcUa_Buffer**)&handle->pBuffer);
    }

    OpcUa_Free(handle);
    OpcUa_Free(*istrm);

    *istrm = OpcUa_Null;

    return uStatus;
}

/*============================================================================
 * OpcUa_MemoryStream_CreateWriteable
 *===========================================================================*/
OpcUa_StatusCode OpcUa_MemoryStream_CreateWriteable(
    OpcUa_UInt32         a_uBlockSize,
    OpcUa_UInt32         a_uMaxSize,
    OpcUa_OutputStream** a_ppOstrm)
{
    OpcUa_MemoryStream* pMemoryStream   = OpcUa_Null;

#if OPCUA_PREALLOC_MEMORYBLOCK
    OpcUa_Byte*         pbyData         = OpcUa_Null;
#endif /* OPCUA_PREALLOC_MEMORYBLOCK */

OpcUa_InitializeStatus(OpcUa_Module_MemoryStream, "CreateWritable");

    OpcUa_ReturnErrorIfNull(a_ppOstrm, OpcUa_BadInvalidArgument);
    *a_ppOstrm = OpcUa_Null;

    pMemoryStream = (OpcUa_MemoryStream*)OpcUa_Alloc(sizeof(OpcUa_MemoryStream));
    OpcUa_GotoErrorIfAllocFailed(pMemoryStream);

    pMemoryStream->SanityCheck = OpcUa_MemoryStream_SanityCheck;
    pMemoryStream->pBuffer     = OpcUa_Null;
    pMemoryStream->Closed      = OpcUa_False;

#if OPCUA_PREALLOC_MEMORYBLOCK

    pbyData = (OpcUa_Byte*)OpcUa_Alloc(a_uBlockSize);
    OpcUa_GotoErrorIfAllocFailed(pbyData);
    OpcUa_MemSetD(pbyData, 0, a_uBlockSize);
    
    uStatus = OpcUa_Buffer_Create(  pbyData, /* buffer space */
                                    a_uBlockSize, /* buffer space size */
                                    a_uBlockSize, /* allocation increment */
                                    a_uMaxSize, /* max memory block size */
                                    OpcUa_True, /* release buffer space */
                                    (OpcUa_Buffer**)&pMemoryStream->pBuffer);
    OpcUa_GotoErrorIfBad(uStatus);

#else /* OPCUA_PREALLOC_MEMORYBLOCK */

    uStatus = OpcUa_Buffer_Create(  OpcUa_Null, /* buffer space */
                                    0,/* used data in buffer space */
                                    a_uBlockSize, /* allocation increment */
                                    a_uMaxSize, /* max memory block size */
                                    OpcUa_True, /* release buffer space */
                                    (OpcUa_Buffer**)&pMemoryStream->pBuffer);
    OpcUa_GotoErrorIfBad(uStatus);

#endif /* OPCUA_PREALLOC_MEMORYBLOCK */

    *a_ppOstrm = (OpcUa_OutputStream*)OpcUa_Alloc(sizeof(OpcUa_OutputStream));
    OpcUa_GotoErrorIfAllocFailed(*a_ppOstrm);

     OpcUa_MemSetD(*a_ppOstrm, 0, sizeof(OpcUa_OutputStream));
    
    (*a_ppOstrm)->Type              = OpcUa_StreamType_Output;
    (*a_ppOstrm)->Handle            = pMemoryStream;
    (*a_ppOstrm)->GetPosition       = OpcUa_MemoryStream_GetPosition;
    (*a_ppOstrm)->SetPosition       = OpcUa_MemoryStream_SetPosition;
    (*a_ppOstrm)->Close             = OpcUa_MemoryStream_Close;
    (*a_ppOstrm)->Delete            = OpcUa_MemoryStream_Delete;
    (*a_ppOstrm)->Flush             = OpcUa_MemoryStream_Flush;
    (*a_ppOstrm)->Write             = OpcUa_MemoryStream_Write;
    (*a_ppOstrm)->CanSeek           = OpcUa_True;
    (*a_ppOstrm)->AttachBuffer      = OpcUa_MemoryStream_AttachBuffer;
    (*a_ppOstrm)->DetachBuffer      = OpcUa_MemoryStream_DetachBufferInterface;
    (*a_ppOstrm)->GetChunkLength    = OpcUa_MemoryStream_GetChunkLength;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    if(pMemoryStream != OpcUa_Null && pMemoryStream->pBuffer != OpcUa_Null)
    {
        OpcUa_Buffer_Delete((OpcUa_Buffer**)&pMemoryStream->pBuffer);
    }

    OpcUa_Free(pMemoryStream);
    OpcUa_Free(*a_ppOstrm);

    *a_ppOstrm = OpcUa_Null;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_MemoryStream_GetBuffer
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_MemoryStream_GetBuffer(
    OpcUa_OutputStream* a_pOstrm,
    OpcUa_Byte**        a_ppBuffer,
    OpcUa_UInt32*       a_puBufferSize)
{
    OpcUa_MemoryStream* handle = OpcUa_Null;
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_MemoryStream);

    OpcUa_ReturnErrorIfNull(a_pOstrm,       OpcUa_BadInvalidArgument);
    OpcUa_ReturnErrorIfNull(a_ppBuffer,     OpcUa_BadInvalidArgument);
    OpcUa_ReturnErrorIfNull(a_puBufferSize, OpcUa_BadInvalidArgument);

    if (a_pOstrm->Type != OpcUa_StreamType_Output || a_pOstrm->Write != OpcUa_MemoryStream_Write)
    {
        return OpcUa_BadInvalidArgument;
    }

    handle = (OpcUa_MemoryStream*)a_pOstrm->Handle;

    if (!handle->Closed)
    {
        return OpcUa_BadInvalidState;
    }

    return OpcUa_Buffer_GetData(handle->pBuffer, a_ppBuffer, a_puBufferSize);
}

/*============================================================================
 * OpcUa_MemoryStream_DetachBuffer
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_MemoryStream_DetachBuffer(
    OpcUa_OutputStream* a_pOstrm,
    OpcUa_Byte**        a_ppBuffer,
    OpcUa_UInt32*       a_puBufferSize)
{
    OpcUa_MemoryStream* pMemoryStream = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_MemoryStream, "CreateWritable");

    OpcUa_ReturnErrorIfNull(a_pOstrm,       OpcUa_BadInvalidArgument);
    OpcUa_ReturnErrorIfNull(a_ppBuffer,     OpcUa_BadInvalidArgument);
    OpcUa_ReturnErrorIfNull(a_puBufferSize, OpcUa_BadInvalidArgument);

    if (a_pOstrm->Type != OpcUa_StreamType_Output || a_pOstrm->Write != OpcUa_MemoryStream_Write)
    {
        return OpcUa_BadInvalidArgument;
    }

    pMemoryStream = (OpcUa_MemoryStream*)a_pOstrm->Handle;

    if (!pMemoryStream->Closed)
    {
        return OpcUa_BadInvalidState;
    }

    /* get hold of data and size */
    uStatus = OpcUa_Buffer_GetData(pMemoryStream->pBuffer, a_ppBuffer, a_puBufferSize);
    OpcUa_GotoErrorIfBad(uStatus);

    /* clear internal buffer */
    pMemoryStream->pBuffer->Data = OpcUa_Null;
    OpcUa_Buffer_Clear(pMemoryStream->pBuffer);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}
/*============================================================================
 * OpcUa_MemoryStream_SetUserData
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_MemoryStream_SetUserData(
    OpcUa_Stream*       a_pStream,
    OpcUa_Void*         a_pData)
{
    OpcUa_MemoryStream* handle = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_MemoryStream, "SetUserData");

    OpcUa_ReturnErrorIfArgumentNull(a_pStream);

    handle = (OpcUa_MemoryStream*)a_pStream->Handle;

    handle->pUserData = a_pData;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_MemoryStream_GetUserData
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_MemoryStream_GetUserData(
    OpcUa_Stream*       a_pStream,
    OpcUa_Void**        a_ppData)
{
    OpcUa_MemoryStream* handle = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_MemoryStream, "SetUserData");

    OpcUa_ReturnErrorIfArgumentNull(a_pStream);

    handle = (OpcUa_MemoryStream*)a_pStream->Handle;

    *a_ppData = handle->pUserData;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

#endif /* OPCUA_HAVE_MEMORYSTREAM */
