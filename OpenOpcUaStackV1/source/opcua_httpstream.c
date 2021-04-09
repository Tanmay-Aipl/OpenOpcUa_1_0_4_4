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

#ifdef OPCUA_HAVE_HTTPAPI

#include <opcua_list.h>
#include <opcua_socket.h>
#include <opcua_utilities.h>
#include <opcua_http_internal.h>
#include <opcua_httpstream.h>

#define OpcUa_HttpOutputStream_SanityCheck 0x83514D85
#define OpcUa_HttpInputStream_SanityCheck 0xB4155377

#define OpcUa_HttpOutputStream_BlockSize 8192
#define OpcUa_HttpInputStream_BlockSize 8192
#define OpcUa_HttpInputStream_MaxLineLength OpcUa_HttpInputStream_BlockSize


/*============================================================================
 * OpcUa_HttpStartLine
 *===========================================================================*/
/** @brief An union that contains either the request line or the status line */
union _OpcUa_HttpStartLine
{
    OpcUa_HttpRequestLine*  RequestLine;
    OpcUa_HttpStatusLine*   StatusLine;
};

typedef union _OpcUa_HttpStartLine OpcUa_HttpStartLine;

/*============================================================================
 * OpcUa_HttpInputStream
 *===========================================================================*/
/** @brief Private data structure for an OpcUa_Stream that allows reading from
  * a socket.
  */
struct _OpcUa_HttpInputStream
{
    /** @brief Inherited Fields from OpcUa_InputStream. @see OpcUa_InputStream */
    OpcUa_Int32                             Type;
    OpcUa_Handle                            Handle;
    OpcUa_Stream_PfnGetPosition*            GetPosition;
    OpcUa_Stream_PfnSetPosition*            SetPosition;
    OpcUa_Stream_PfnGetChunkLength*         GetChunkLength;
    OpcUa_Stream_PfnDetachBuffer*           DetachBuffer;
    OpcUa_Stream_PfnAttachBuffer*           AttachBuffer;
    OpcUa_Stream_PfnClose*                  Close;
    OpcUa_Stream_PfnDelete*                 Delete;
    OpcUa_Boolean                           CanSeek;
    OpcUa_Stream_PfnRead*                   Read;
    OpcUa_Boolean                           NonBlocking;

    /* Subclass Fields */
    /** @brief Check type of interface implementation. */
    OpcUa_UInt32                            SanityCheck;
    /** @brief Type of the message to be received. @see OpcUa_HttpStream_MessageType */
    OpcUa_HttpStream_MessageType            MessageType;
    /** @brief The communication handle with which the message was received from. */
    OpcUa_Socket                            Socket;
    /** @brief Tells whether the stream is closed for reading. */
    OpcUa_Boolean                           Closed;
    /** @brief The function to call if the stream has new data. */
    OpcUa_Stream_PfnOnReadyToRead*          Callback;
    /** @brief Data to return with the given callback function. */
    OpcUa_Void*                             CallbackData;
    /** @brief The current state of the stream. @see OpcUa_HttpStream_State */
    OpcUa_HttpStream_State                  State;
    /** @brief Message start line. */
    OpcUa_HttpStartLine                     StartLine;
    /** @brief Message headers. */
    OpcUa_HttpHeaderCollection*             Headers;
    /** @brief The size of the message body. */
    OpcUa_UInt32                            ContentLength;
    /** @brief The internal buffer. */
    OpcUa_Buffer                            Buffer;
    /** @brief The recent message line. */
    OpcUa_String                            MessageLine;
    /** @brief The position of the first character in a recent message line */
    OpcUa_UInt32                            LineStart;
    /** @brief The position of the last character in a recent message line */
    OpcUa_UInt32                            LineEnd;
};
typedef struct _OpcUa_HttpInputStream OpcUa_HttpInputStream;

/*============================================================================
 * OpcUa_HttpOutputStream
 *===========================================================================*/
/** @brief Private data structure for an OpcUa_Stream that allows writing to
  * a socket.
  */
struct _OpcUa_HttpOutputStream
{
    /** @brief Inherited Fields from OpcUa_OutputStream. @see OpcUa_OutputStream */
    OpcUa_Int32                             Type;
    OpcUa_Handle                            Handle;
    OpcUa_Stream_PfnGetPosition*            GetPosition;
    OpcUa_Stream_PfnSetPosition*            SetPosition;
    OpcUa_Stream_PfnGetChunkLength*         GetChunkLength;
    OpcUa_Stream_PfnDetachBuffer*           DetachBuffer;
    OpcUa_Stream_PfnAttachBuffer*           AttachBuffer;
    OpcUa_Stream_PfnClose*                  Close;
    OpcUa_Stream_PfnDelete*                 Delete;
    OpcUa_Boolean                           CanSeek;
    OpcUa_Stream_PfnWrite*                  Write;
    OpcUa_Stream_PfnFlush*                  Flush;

    /* Subclass (Http) Fields */
    /** @brief Check type of interface implementation. */
    OpcUa_UInt32                            SanityCheck;
    /** @brief Type of the message to be sent. @see OpcUa_HttpStream_MessageType */
    OpcUa_HttpStream_MessageType            MessageType;
    /** @brief The communication handle with which the message is sent. */
    OpcUa_Socket                            Socket;
    /** @brief Tells whether the stream is closed for reading. */
    OpcUa_Boolean                           Closed;
    /** @brief The function to call if the stream is ready for new data. */
    OpcUa_Stream_PfnOnReadyToWrite*         Callback;
    /** @brief The function to call if the stream has new data. */
    OpcUa_Void*                             CallbackData;
    /** @brief Handle of the underlying connection. */
    OpcUa_Void*                             hConnection;
    /** @brief Message start line. */
    OpcUa_HttpStartLine                     StartLine;
    /** @brief Message headers. */
    OpcUa_HttpHeaderCollection*             Headers;
    /** @brief The internal message buffer. */
    OpcUa_Buffer                            Buffer;
    /** @brief Disconnect notification callback. */
    OpcUa_HttpStream_PfnNotifyDisconnect*   NotifyDisconnect;
};
typedef struct _OpcUa_HttpOutputStream OpcUa_HttpOutputStream;


/*============================================================================
 * OpcUa_ReturnErrorIfInvalidStream
 *===========================================================================*/
/** @brief check instance */
#define OpcUa_ReturnErrorIfInvalidStream(xStream, xMethod) \
if (    (    (((OpcUa_HttpInputStream*)(xStream->Handle))->SanityCheck != OpcUa_HttpInputStream_SanityCheck) \
          && (((OpcUa_HttpOutputStream*)(xStream->Handle))->SanityCheck != OpcUa_HttpOutputStream_SanityCheck)) \
     || (xStream->xMethod != OpcUa_HttpStream_##xMethod)) \
{ \
    return OpcUa_BadInvalidArgument; \
}

/*============================================================================
 * OpcUa_Buffer_WriteString
 *===========================================================================*/
/** @brief writes string bytes into a buffer */
#define OpcUa_Buffer_WriteString(xBuffer, xString) \
    OpcUa_Buffer_Write(xBuffer, \
                       (OpcUa_Byte*)OpcUa_String_GetRawString(xString), \
                       OpcUa_String_StrSize(xString));

/*============================================================================
 * OpcUa_Buffer_ReadChar
 *===========================================================================*/
/** @brief reads the next available character */
static OpcUa_StatusCode OpcUa_Buffer_ReadChar(
    OpcUa_Buffer*   a_pBuffer,
    OpcUa_CharA*    a_pValue)
{
    OpcUa_ReturnErrorIfArgumentNull(a_pBuffer);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);

    if (a_pBuffer->EndOfData - a_pBuffer->Position == 0)
    {
        return OpcUa_BadEndOfStream;
    }

    *a_pValue = (OpcUa_CharA)(a_pBuffer->Data[a_pBuffer->Position++]);

    return OpcUa_Good;
}

/*============================================================================
 * OpcUa_Buffer_PeekChar
 *===========================================================================*/
/** @brief reads the next available character, does not advance the position */
static OpcUa_StatusCode OpcUa_Buffer_PeekChar(
    OpcUa_Buffer*   a_pBuffer,
    OpcUa_CharA*    a_pValue)
{
    OpcUa_ReturnErrorIfArgumentNull(a_pBuffer);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);

    if (a_pBuffer->EndOfData - a_pBuffer->Position == 0)
    {
        return OpcUa_BadEndOfStream;
    }

    *a_pValue = (OpcUa_CharA)(a_pBuffer->Data[a_pBuffer->Position]);

    return OpcUa_Good;
}

/*============================================================================
 * OpcUa_Buffer_SetLength
 *===========================================================================*/
/** @brief sets the length of the buffer */
static OpcUa_StatusCode OpcUa_Buffer_SetLength(
    OpcUa_Buffer*   a_pBuffer,
    OpcUa_UInt32    a_uLength)
{
    OpcUa_ReturnErrorIfArgumentNull(a_pBuffer);

    if(a_pBuffer->MaxSize != 0 &&  a_pBuffer->MaxSize < a_uLength)
    {
        return OpcUa_BadInvalidArgument;
    }

    /* check whether the length is not the same */
    if(a_pBuffer->EndOfData != a_uLength)
    {
        /* calculate the new buffer size taking the block size into account */
        OpcUa_UInt32 uBufferSize  = a_uLength / a_pBuffer->BlockSize * a_pBuffer->BlockSize;
                     uBufferSize += a_uLength % a_pBuffer->BlockSize?  a_pBuffer->BlockSize: 0;

        /* check whether the calculated size is not the same */
        if(uBufferSize != a_pBuffer->Size)
        {
            /* reallocate the buffer */
            OpcUa_Byte* pBufferData = (OpcUa_Byte*)OpcUa_ReAlloc(a_pBuffer->Data, uBufferSize);

            if(uBufferSize != 0 && a_pBuffer->Data == OpcUa_Null)
            {
                return OpcUa_BadOutOfMemory;
            }

            a_pBuffer->Data = pBufferData;
            a_pBuffer->Size = uBufferSize;            
        }

        a_pBuffer->EndOfData = a_uLength;
        
        /* update the current position if required */
        if(a_pBuffer->Position > a_pBuffer->EndOfData)
        {
            a_pBuffer->Position = a_pBuffer->EndOfData;
        }
    }

    return OpcUa_Good;
}

/*============================================================================
 * OpcUa_HttpStartLine_GetBytes
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_HttpStartLine_GetBytes(
    OpcUa_HttpStartLine*            a_pStartLine,
    OpcUa_HttpStream_MessageType    a_eMessageType,
    OpcUa_Buffer*                   a_pBuffer)
{
OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "OpcUa_HttpStatusLine_GetBytes");

    OpcUa_ReturnErrorIfArgumentNull(a_pStartLine);
    OpcUa_ReturnErrorIfArgumentNull(a_pBuffer);

    switch(a_eMessageType)
    {
        case OpcUa_HttpStream_MessageType_Request:
        {
            uStatus = OpcUa_HttpRequestLine_GetBytes(a_pStartLine->RequestLine, a_pBuffer);
            break;
        }

        case OpcUa_HttpStream_MessageType_Response:
        {
            uStatus = OpcUa_HttpStatusLine_GetBytes(a_pStartLine->StatusLine, a_pBuffer);
            break;
        }

        default:
        {
            uStatus = OpcUa_BadInvalidArgument;
            break;
        }
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpStream_DetachBuffer
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpStream_DetachBuffer(
    OpcUa_Stream*   a_pStream,
    OpcUa_Buffer*   a_pBuffer)
{
OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "DetachBuffer");

    OpcUa_ReturnErrorIfArgumentNull(a_pStream);
    OpcUa_ReturnErrorIfArgumentNull(a_pStream->Handle);
    OpcUa_ReturnErrorIfArgumentNull(a_pBuffer);

    switch(a_pStream->Type)
    {
        case OpcUa_StreamType_Output:
        {
            OpcUa_HttpOutputStream* pHttpOutputStream = (OpcUa_HttpOutputStream*)a_pStream->Handle;

            *a_pBuffer = pHttpOutputStream->Buffer;
            pHttpOutputStream->Buffer.Data = OpcUa_Null;
            OpcUa_Buffer_Clear(&pHttpOutputStream->Buffer);

            break;
        }
        case OpcUa_StreamType_Input:
        {
            OpcUa_HttpInputStream* pHttpInputStream = (OpcUa_HttpInputStream*)a_pStream->Handle;

            *a_pBuffer = pHttpInputStream->Buffer;
            pHttpInputStream->Buffer.Data = OpcUa_Null;
            OpcUa_Buffer_Clear(&pHttpInputStream->Buffer);

            pHttpInputStream->State = OpcUa_HttpStream_State_Empty;
            pHttpInputStream->Close((OpcUa_Stream*)pHttpInputStream);

            break;
        }
        default:
        {
            uStatus = OpcUa_BadInvalidArgument;
            OpcUa_GotoError;
        }
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpStream_AttachBuffer
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpStream_AttachBuffer(
    OpcUa_Stream*   a_pStream,
    OpcUa_Buffer*   a_pBuffer)
{
OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "AttachBuffer");

    OpcUa_ReturnErrorIfArgumentNull(a_pStream);
    OpcUa_ReturnErrorIfArgumentNull(a_pBuffer);
    OpcUa_ReturnErrorIfArgumentNull(a_pStream->Handle);

    switch(a_pStream->Type)
    {
        case OpcUa_StreamType_Output:
        {
            OpcUa_HttpOutputStream* pHttpOutputStream   = (OpcUa_HttpOutputStream*)a_pStream->Handle;
            OpcUa_Buffer            OldBuffer           = pHttpOutputStream->Buffer;

            pHttpOutputStream->Buffer = *a_pBuffer;

            /* create same state as data would have been written into the stream */
            uStatus = OpcUa_Buffer_SetPosition(&pHttpOutputStream->Buffer, OpcUa_BufferPosition_End);
            if(OpcUa_IsBad(uStatus))
            {
                /* restore old buffer */
                pHttpOutputStream->Buffer = OldBuffer;

            }

            a_pBuffer->Data = OpcUa_Null;
            OpcUa_Buffer_Clear(a_pBuffer);

            break;
        }
        case OpcUa_StreamType_Input:
        {
            OpcUa_HttpInputStream* pHttpInputStream = (OpcUa_HttpInputStream*)a_pStream->Handle;

            OpcUa_ReferenceParameter(pHttpInputStream);

            uStatus = OpcUa_BadNotSupported;
            
            break;
        }
        default:
        {
            uStatus = OpcUa_BadInvalidArgument;
            OpcUa_GotoError;
        }
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpStream_GetChunkLength
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpStream_GetChunkLength(
    OpcUa_Stream* a_pStream, 
    OpcUa_UInt32* a_puLength)
{
OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "GetChunkLength");

    OpcUa_ReturnErrorIfArgumentNull(a_pStream);
    OpcUa_ReturnErrorIfArgumentNull(a_pStream->Handle);
    OpcUa_ReturnErrorIfArgumentNull(a_puLength);

    /* http messages are regarded as indivisible */
    *a_puLength = 0;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpStream_Read
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpStream_Read(
    struct _OpcUa_InputStream*     a_pInputStream,  /* Stream with HttpStream handle */
    OpcUa_Byte*                    a_pTargetBuffer, /* The destination buffer. */
    OpcUa_UInt32*                  a_puCount,       /* How many bytes should be delivered. */
    OpcUa_Stream_PfnOnReadyToRead* a_pfnCallback,   /* The callack for later data ready events. */
    OpcUa_Void*                    a_pCallbackData) /* The data to give to the callback. */
{
    OpcUa_HttpInputStream* pHttpInputStream = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "Read");

    OpcUa_ReturnErrorIfArgumentNull(a_pInputStream);
    OpcUa_ReturnErrorIfArgumentNull(a_pInputStream->Handle);
    OpcUa_ReturnErrorIfArgumentNull(a_pTargetBuffer);
    OpcUa_ReturnErrorIfArgumentNull(a_puCount);
    OpcUa_ReturnErrorIfInvalidStream(a_pInputStream, Read);
   
    /* HINTS: we dont want to trigger a socket recv for every element, so the 
              data gets buffered internally in the stream to read "as much as
              possible" (well, not really, trying to predict message borders 
              implicitly through buffer sizes.) in one api call for performance 
              reasons.

              A read looks, if the requested amount of data is available in 
              the internal buffer and copies it into the target. The caller
              must swap into the right byte order afterwards.
              */

    /* resolve stream handle to http stream */
    pHttpInputStream = (OpcUa_HttpInputStream*)a_pInputStream->Handle;

    /* check for end of stream */
    /* (over)write the callback information */
    pHttpInputStream->Callback     = a_pfnCallback;
    pHttpInputStream->CallbackData = a_pCallbackData;

    uStatus = OpcUa_Buffer_Read(&(pHttpInputStream ->Buffer), a_pTargetBuffer, a_puCount);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

OpcUa_FinishErrorHandling;
}


/*============================================================================
 * OpcUa_HttpStream_Write
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpStream_Write(
    OpcUa_OutputStream* a_pOutputStream,    /* the stream to write the value into */
    OpcUa_Byte*         a_pBuffer,          /* the value to write */
    OpcUa_UInt32        a_uBufferSize)      /* the size of the value to write */
{
    OpcUa_HttpOutputStream*  pHttpOutputStream = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "Write");

    OpcUa_ReturnErrorIfArgumentNull(a_pOutputStream);
    OpcUa_ReturnErrorIfArgumentNull(a_pOutputStream->Handle);
    OpcUa_ReturnErrorIfArgumentNull(a_pBuffer);

    pHttpOutputStream = (OpcUa_HttpOutputStream*)a_pOutputStream->Handle;

    OpcUa_ReturnErrorIfInvalidStream(a_pOutputStream, Write);
    
    if(pHttpOutputStream->Closed)
    {
        return OpcUa_BadInvalidState;
    }

    uStatus = OpcUa_Buffer_Write(&(pHttpOutputStream->Buffer), a_pBuffer, a_uBufferSize);
    OpcUa_ReturnErrorIfBad(uStatus);
        
OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpStream_Flush
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpStream_Flush(
    OpcUa_OutputStream* a_pOutputStream,
    OpcUa_Boolean       a_bLastCall)
{
    OpcUa_HttpOutputStream* pHttpOutputStream = OpcUa_Null;
    OpcUa_Byte*             pContentData      = OpcUa_Null;
    OpcUa_UInt32            uContentLength    = 0;
    OpcUa_Byte*             pMessageData      = OpcUa_Null;
    OpcUa_UInt32            uMessageLength    = 0;
    OpcUa_Buffer*           pTemporaryBuffer  = OpcUa_Null;
    OpcUa_Int32             iBytesWritten     = 0;

OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "Flush");

    OpcUa_ReturnErrorIfArgumentNull(a_pOutputStream);
    OpcUa_ReturnErrorIfArgumentNull(a_pOutputStream->Handle);
    OpcUa_ReturnErrorIfInvalidStream(a_pOutputStream, Flush);

    OpcUa_ReferenceParameter(a_bLastCall);

    pHttpOutputStream = (OpcUa_HttpOutputStream*)a_pOutputStream->Handle;

    OpcUa_GotoErrorIfTrue((pHttpOutputStream->Closed), OpcUa_BadInvalidState);

    if(!OpcUa_Buffer_IsEmpty(&pHttpOutputStream->Buffer))
    {
        OpcUa_CharA chContentLength[0x20] = {'\x00'};

        /* check whether the message body is allowed */
        if(pHttpOutputStream->MessageType == OpcUa_HttpStream_MessageType_Request)
        {
            if(OpcUa_String_StrnCmp(&(pHttpOutputStream->StartLine.RequestLine->RequestMethod),
                                    OpcUa_String_FromCString("HEAD"),
                                    OPCUA_STRING_LENDONTCARE,
                                    OpcUa_False) == 0) 
            {
                return OpcUa_BadInvalidState;
            }
        }

        if(pHttpOutputStream->MessageType == OpcUa_HttpStream_MessageType_Response)
        {
            if((pHttpOutputStream->StartLine.StatusLine->StatusCode >= 100) && 
               (pHttpOutputStream->StartLine.StatusLine->StatusCode <  200))
            {
                return OpcUa_BadInvalidState;
            }

            if((pHttpOutputStream->StartLine.StatusLine->StatusCode == 204) ||
               (pHttpOutputStream->StartLine.StatusLine->StatusCode == 304))
            {
                return OpcUa_BadInvalidState;
            }
        }

        /* get content-length value */
        uStatus = OpcUa_Buffer_GetData(&pHttpOutputStream->Buffer, &pContentData, &uContentLength);
        OpcUa_GotoErrorIfBad(uStatus);

        /* format content-length value */
        OpcUa_SPrintfA(chContentLength,
#if OPCUA_USE_SAFE_FUNCTIONS
                       sizeof(chContentLength)/sizeof(chContentLength[0]), 
#endif 
                       "%u", 
                       uContentLength);

        /* update the "Content-Length" header */
        uStatus = OpcUa_HttpHeaderCollection_SetValue(pHttpOutputStream->Headers,
                                                      OpcUa_String_FromCString("Content-Length"),
                                                      OpcUa_String_FromCString(chContentLength));
        OpcUa_GotoErrorIfBad(uStatus);
    }

    if(!OpcUa_Buffer_IsEmpty(&pHttpOutputStream->Buffer))
    {
        OpcUa_Byte*     pTemporaryData      = OpcUa_Null;
        OpcUa_UInt32    uTemporaryLength    = 0;

        /* create a temporary buffer */
        uStatus = OpcUa_Buffer_Create(OpcUa_Null, 0, 0x100, 0, OpcUa_True, &pTemporaryBuffer);
        OpcUa_GotoErrorIfBad(uStatus);

        /* place the message start line into a temporary buffer */
        uStatus = OpcUa_HttpStartLine_GetBytes(&pHttpOutputStream->StartLine,
                                               pHttpOutputStream->MessageType,
                                               pTemporaryBuffer);
        OpcUa_GotoErrorIfBad(uStatus);

        /* place the message headers into a temporary buffer */
        uStatus = OpcUa_HttpHeaderCollection_GetBytes(pHttpOutputStream->Headers,
                                                      pTemporaryBuffer);
        OpcUa_GotoErrorIfBad(uStatus);

        /* get temporary data */
        uStatus = OpcUa_Buffer_GetData(pTemporaryBuffer, &pTemporaryData, &uTemporaryLength);
        OpcUa_GotoErrorIfBad(uStatus);

        /* get message content */
        uStatus = OpcUa_Buffer_GetData(&pHttpOutputStream->Buffer, &pContentData, &uContentLength);
        OpcUa_GotoErrorIfBad(uStatus);

        /* update the message length */
        uMessageLength = uTemporaryLength + uContentLength;        
        OpcUa_GotoErrorIfBad(uStatus);

        /* expand the buffer to store the start line and message headers */
        uStatus = OpcUa_Buffer_SetLength(&pHttpOutputStream->Buffer, uMessageLength);
        OpcUa_GotoErrorIfBad(uStatus);

        /* get message data */
        uStatus = OpcUa_Buffer_GetData(&pHttpOutputStream->Buffer, &pMessageData, &uMessageLength);
        OpcUa_GotoErrorIfBad(uStatus);

        /* shift the message content to provide the space for the start line and message headers */
        uStatus = OpcUa_MemCpy(&pMessageData[uTemporaryLength], uMessageLength, pMessageData, uContentLength);
        OpcUa_GotoErrorIfBad(uStatus);

        /* store the start line and message headers */
        uStatus = OpcUa_MemCpy(pMessageData, uMessageLength, pTemporaryData, uTemporaryLength);
        OpcUa_GotoErrorIfBad(uStatus);

        /* delete temporary buffer */
        OpcUa_Buffer_Delete(&pTemporaryBuffer);
        pTemporaryBuffer = OpcUa_Null;
    }
    else
    {
        /* place the message start line into an output buffer */
        uStatus = OpcUa_HttpStartLine_GetBytes(&pHttpOutputStream->StartLine,
                                               pHttpOutputStream->MessageType,
                                               &pHttpOutputStream->Buffer);
        OpcUa_GotoErrorIfBad(uStatus);

        /* place the message headers into an output buffer */
        uStatus = OpcUa_HttpHeaderCollection_GetBytes(pHttpOutputStream->Headers,
                                                      &pHttpOutputStream->Buffer);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    if(!OpcUa_Buffer_IsEmpty(&pHttpOutputStream->Buffer))
    {
        /* get message data */
        uStatus = OpcUa_Buffer_GetData(&pHttpOutputStream->Buffer, &pMessageData, &uMessageLength);
        OpcUa_GotoErrorIfBad(uStatus);

        OpcUa_Trace(OPCUA_TRACE_LEVEL_DEBUG, "OpcUa_HttpStream_Flush: Message length is %d!\n", uMessageLength);

        /* send to network */
        iBytesWritten = OPCUA_P_SOCKET_WRITE(pHttpOutputStream->Socket, 
                                             pMessageData,
                                             uMessageLength, 
                                             OpcUa_True);
        
        if(iBytesWritten < (OpcUa_Int32)uMessageLength)
        {
            if(iBytesWritten < (OpcUa_Int32)0)
            {
                uStatus = OPCUA_P_SOCKET_GETLASTERROR(pHttpOutputStream->Socket);
                OpcUa_Trace(OPCUA_TRACE_LEVEL_DEBUG, "OpcUa_HttpStream_Flush: Error writing to socket: 0x%08X!\n", uStatus);

                /* notify connection */
                if((pHttpOutputStream->NotifyDisconnect != OpcUa_Null) && (pHttpOutputStream->hConnection != OpcUa_Null))
                {
                    pHttpOutputStream->NotifyDisconnect(pHttpOutputStream->hConnection);
                }

                OpcUa_GotoErrorWithStatus(OpcUa_BadDisconnect);
            }
            else
            {
                /* keep as outgoing stream */
                OpcUa_Trace(OPCUA_TRACE_LEVEL_DEBUG, "OpcUa_HttpStream_Flush: Only %u bytes of %u written!\n", iBytesWritten, uMessageLength);
            }
        }

        /* empty an output buffer */
        OpcUa_Trace(OPCUA_TRACE_LEVEL_DEBUG, "OpcUa_HttpStream_Flush: Buffer emptied!\n");
        OpcUa_Buffer_SetEmpty(&pHttpOutputStream->Buffer);
    }


OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_Buffer_Delete(&pTemporaryBuffer);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpStream_Close
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpStream_Close(OpcUa_Stream* a_pStream)
{
OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "Close");

    OpcUa_ReturnErrorIfArgumentNull(a_pStream);
    OpcUa_ReturnErrorIfArgumentNull(a_pStream->Handle);
    OpcUa_ReturnErrorIfInvalidStream(a_pStream, Close);
    
    if(a_pStream->Type == OpcUa_StreamType_Output)
    {
        if(((OpcUa_HttpOutputStream*)(a_pStream->Handle))->Closed)
        {
            return OpcUa_BadInvalidState;
        }

        OpcUa_HttpStream_Flush((OpcUa_OutputStream*)a_pStream, OpcUa_True);

        ((OpcUa_HttpOutputStream*)(a_pStream->Handle))->Closed = OpcUa_True;
    }
    else if(a_pStream->Type == OpcUa_StreamType_Input)
    {
        if(((OpcUa_HttpInputStream*)(a_pStream->Handle))->Closed)
        {
            return OpcUa_BadInvalidState;
        }

        /* TODO: closing a stream before the end of the message could screw things up.
           Need to read rest of message from stream before closing. Thats complex, 
           because the rest of the message may be delayed, so we would have to block here,
           what we don't want. Handle this stream like before, but mark it as abandoned! 
           If the stream is complete, it will not be handled but deleted immediately. 
           Intermediary read events are not further processed. */

        ((OpcUa_HttpInputStream*)(a_pStream->Handle))->Closed = OpcUa_True;
    }
    else
    {
        return OpcUa_BadInvalidState;
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpStream_Delete
 *===========================================================================*/
OpcUa_Void OpcUa_HttpStream_Delete(OpcUa_Stream** a_ppStream)
{
    if(a_ppStream == OpcUa_Null)
    {
        /* error condition - should not happen */
        return;
    }

    if((*a_ppStream) == OpcUa_Null)
    {
        /* error condition - should not happen */
        return;
    }

    if((*a_ppStream)->Type == OpcUa_StreamType_Output)
    {
        OpcUa_HttpOutputStream* pOutputStream = (OpcUa_HttpOutputStream*)((*a_ppStream)->Handle);

        OpcUa_Buffer_Clear(&(pOutputStream->Buffer));
        
        if(pOutputStream->MessageType == OpcUa_HttpStream_MessageType_Request)
        {
            OpcUa_HttpRequestLine_Clear(pOutputStream->StartLine.RequestLine);
            OpcUa_Free(pOutputStream->StartLine.RequestLine);
        }
        
        else if(pOutputStream->MessageType == OpcUa_HttpStream_MessageType_Response)
        {
            OpcUa_HttpStatusLine_Clear(pOutputStream->StartLine.StatusLine);
            OpcUa_Free(pOutputStream->StartLine.StatusLine);
        }

        OpcUa_HttpHeaderCollection_Delete(&pOutputStream->Headers);

        OpcUa_Free(*a_ppStream);
        *a_ppStream = OpcUa_Null;
    }
    else if((*a_ppStream)->Type == OpcUa_StreamType_Input)
    {
        OpcUa_HttpInputStream* pInputStream = (OpcUa_HttpInputStream*)((*a_ppStream)->Handle);

        if(pInputStream->MessageType == OpcUa_HttpStream_MessageType_Request)
        {
            OpcUa_HttpRequestLine_Clear(pInputStream->StartLine.RequestLine);
            OpcUa_Free(pInputStream->StartLine.RequestLine);
        }

        else if(pInputStream->MessageType == OpcUa_HttpStream_MessageType_Response)
        {
            OpcUa_HttpStatusLine_Clear(pInputStream->StartLine.StatusLine);
            OpcUa_Free(pInputStream->StartLine.StatusLine);
        }

        OpcUa_HttpHeaderCollection_Delete(&pInputStream->Headers);

        /* clear buffer */
        /* Delete ignores OpcUa_Null, so if the buffer got detached by the upper layer, this works, too. */
        OpcUa_Buffer_Clear(&(pInputStream->Buffer));

        OpcUa_String_Clear(&(pInputStream->MessageLine));

        OpcUa_Free(*a_ppStream);
        *a_ppStream = OpcUa_Null;
    }
    else
    {
        /* error condition - should not happen */
        return;
    }
}

/*============================================================================
 * OpcUa_HttpStream_GetPosition
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpStream_GetPosition(
    OpcUa_Stream* a_pStream, 
    OpcUa_UInt32* a_pPosition)
{
OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "GetPosition");

    OpcUa_ReturnErrorIfArgumentNull(a_pStream);
    OpcUa_ReturnErrorIfArgumentNull(a_pStream->Handle);
    OpcUa_ReturnErrorIfInvalidStream(a_pStream, GetPosition);
    OpcUa_ReferenceParameter(a_pPosition);

    if(a_pStream->Type == OpcUa_StreamType_Output)
    {
        OpcUa_HttpOutputStream* pHttpOutputStream = (OpcUa_HttpOutputStream*)a_pStream->Handle;

        if(pHttpOutputStream->Closed)
        {
            return OpcUa_BadInvalidState;
        }

        *a_pPosition = pHttpOutputStream->Buffer.Position;
    }
    else if(a_pStream->Type == OpcUa_StreamType_Input)
    {
        OpcUa_HttpInputStream* pHttpInputStream = (OpcUa_HttpInputStream*)a_pStream->Handle;

        if(pHttpInputStream->Closed)
        {
            return OpcUa_BadInvalidState;
        }

        *a_pPosition = pHttpInputStream->Buffer.Position;
    }
    else
    {
        uStatus = OpcUa_BadInvalidArgument;
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpStream_SetPosition
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpStream_SetPosition(
    OpcUa_Stream* a_pStream, 
    OpcUa_UInt32  a_uPosition)
{
OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "SetPosition");

    OpcUa_ReturnErrorIfArgumentNull(a_pStream);
    OpcUa_ReturnErrorIfArgumentNull(a_pStream->Handle);
    OpcUa_ReturnErrorIfInvalidStream(a_pStream, SetPosition);
        
    if(a_pStream->Type == OpcUa_StreamType_Output)
    {
        OpcUa_HttpOutputStream* pHttpOutputStream = (OpcUa_HttpOutputStream*)a_pStream->Handle;

        if(pHttpOutputStream->Closed)
        {
            return OpcUa_BadInvalidState;
        }

        /* set the position */
        uStatus = OpcUa_Buffer_SetPosition(&(pHttpOutputStream->Buffer), a_uPosition);
    }
    else if(a_pStream->Type == OpcUa_StreamType_Input)
    {
        OpcUa_HttpInputStream* pHttpInputStream = (OpcUa_HttpInputStream*)a_pStream->Handle;

        if(pHttpInputStream->Closed)
        {
            return OpcUa_BadInvalidState;
        }

        uStatus = OpcUa_Buffer_SetPosition(&pHttpInputStream->Buffer, a_uPosition);
    }
    else
    {
        uStatus = OpcUa_BadInvalidArgument;
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpStream_GetMessageType
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpStream_GetMessageType(
    OpcUa_Stream*                   a_pStream,
    OpcUa_HttpStream_MessageType*   a_pMessageType)
{
OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "GetMessageType");

    OpcUa_ReturnErrorIfArgumentNull(a_pStream);
    OpcUa_ReturnErrorIfArgumentNull(a_pStream->Handle);
    OpcUa_ReturnErrorIfArgumentNull(a_pMessageType);

    if(a_pStream->Type == OpcUa_StreamType_Output)
    {
        *a_pMessageType = ((OpcUa_HttpOutputStream*)a_pStream->Handle)->MessageType;
    }
    else if(a_pStream->Type == OpcUa_StreamType_Input)
    {
        *a_pMessageType = ((OpcUa_HttpInputStream*)a_pStream->Handle)->MessageType;
    }
    else
    {
        *a_pMessageType = OpcUa_HttpStream_MessageType_Unknown;
        uStatus         = OpcUa_BadInvalidArgument;
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpStream_GetSocket
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpStream_GetSocket(
    OpcUa_Stream*   a_pStream,
    OpcUa_Socket*   a_pSocket)
{
OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "GetSocket");

    OpcUa_ReturnErrorIfArgumentNull(a_pStream);
    OpcUa_ReturnErrorIfArgumentNull(a_pStream->Handle);
    OpcUa_ReturnErrorIfArgumentNull(a_pSocket);

    if(a_pStream->Type == OpcUa_StreamType_Output)
    {
        *a_pSocket = ((OpcUa_HttpOutputStream*)a_pStream->Handle)->Socket;
    }
    else if(a_pStream->Type == OpcUa_StreamType_Input)
    {
        *a_pSocket = ((OpcUa_HttpInputStream*)a_pStream->Handle)->Socket;
    }
    else
    {
        *a_pSocket = OpcUa_Null;
        uStatus    = OpcUa_BadInvalidArgument;
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpStream_SetHeader
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpStream_SetHeader(
    OpcUa_Stream*   a_pStream,
    OpcUa_String*   a_pHeaderName,
    OpcUa_String*   a_pHeaderValue)
{
OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "SetHeader");

    OpcUa_ReturnErrorIfArgumentNull(a_pStream);
    OpcUa_ReturnErrorIfArgumentNull(a_pStream->Handle);
    OpcUa_ReturnErrorIfArgumentNull(a_pHeaderName);
    OpcUa_ReturnErrorIfArgumentNull(a_pHeaderValue);

    if(a_pStream->Type == OpcUa_StreamType_Output)
    {
        uStatus = OpcUa_HttpHeaderCollection_SetValue(((OpcUa_HttpOutputStream*)a_pStream->Handle)->Headers,
                                                      a_pHeaderName,
                                                      a_pHeaderValue);
    }
    else if(a_pStream->Type == OpcUa_StreamType_Input)
    {
        uStatus = OpcUa_HttpHeaderCollection_SetValue(((OpcUa_HttpInputStream*)a_pStream->Handle)->Headers,
                                                      a_pHeaderName,
                                                      a_pHeaderValue);
    }
    else
    {
        uStatus = OpcUa_BadInvalidArgument;
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpStream_GetHeader
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpStream_GetHeader(
    OpcUa_Stream*   a_pStream,
    OpcUa_String*   a_pHeaderName,
    OpcUa_String*   a_pHeaderValue)
{
OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "GetHeader");

    OpcUa_ReturnErrorIfArgumentNull(a_pStream);
    OpcUa_ReturnErrorIfArgumentNull(a_pStream->Handle);
    OpcUa_ReturnErrorIfArgumentNull(a_pHeaderName);
    OpcUa_ReturnErrorIfArgumentNull(a_pHeaderValue);

    if(a_pStream->Type == OpcUa_StreamType_Output)
    {
        uStatus = OpcUa_HttpHeaderCollection_GetValue(((OpcUa_HttpOutputStream*)a_pStream->Handle)->Headers,
                                                      a_pHeaderName,
                                                      a_pHeaderValue);
    }
    else if(a_pStream->Type == OpcUa_StreamType_Input)
    {
        uStatus = OpcUa_HttpHeaderCollection_GetValue(((OpcUa_HttpInputStream*)a_pStream->Handle)->Headers,
                                                      a_pHeaderName,
                                                      a_pHeaderValue);
    }
    else
    {
        uStatus = OpcUa_BadInvalidArgument;
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpStream_GetStatusCode
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpStream_GetStatusCode(
    OpcUa_Stream*   a_pStream,
    OpcUa_UInt32*   a_pStatusCode)
{
OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "GetStatusCode");

    OpcUa_ReturnErrorIfArgumentNull(a_pStream);
    OpcUa_ReturnErrorIfArgumentNull(a_pStream->Handle);
    OpcUa_ReturnErrorIfArgumentNull(a_pStatusCode);
    
    if(a_pStream->Type == OpcUa_StreamType_Output)
    {
        if(((OpcUa_HttpOutputStream*)a_pStream->Handle)->MessageType == OpcUa_HttpStream_MessageType_Response)
        {
            *a_pStatusCode = ((OpcUa_HttpOutputStream*)a_pStream->Handle)->StartLine.StatusLine->StatusCode;
        }
        else
        {
            uStatus = OpcUa_BadInvalidArgument;
        }
    }
    else if(a_pStream->Type == OpcUa_StreamType_Input)
    {
        if(((OpcUa_HttpInputStream*)a_pStream->Handle)->MessageType == OpcUa_HttpStream_MessageType_Response)
        {
            *a_pStatusCode = ((OpcUa_HttpInputStream*)a_pStream->Handle)->StartLine.StatusLine->StatusCode;
        }
        else
        {
            uStatus = OpcUa_BadInvalidArgument;
        }
    }
    else
    {
        uStatus = OpcUa_BadInvalidArgument;
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpStream_GetConnection
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpStream_GetConnection(
    OpcUa_OutputStream* a_pOutputStream,
    OpcUa_Handle*       a_pConnection)
{
OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "OpcUa_HttpStream_GetConnection");

    OpcUa_ReturnErrorIfArgumentNull(a_pOutputStream);
    OpcUa_ReturnErrorIfArgumentNull(a_pConnection);

    OpcUa_ReturnErrorIfTrue(a_pOutputStream->Type != OpcUa_StreamType_Output,
                            OpcUa_BadInvalidArgument);

    *a_pConnection = ((OpcUa_HttpOutputStream*)a_pOutputStream->Handle)->hConnection;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
    
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpStream_GetState
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpStream_GetState(
    OpcUa_InputStream*      a_pInputStream,
    OpcUa_HttpStream_State* a_pStreamState)
{
OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "OpcUa_HttpStream_GetState");
    
    OpcUa_ReturnErrorIfArgumentNull(a_pInputStream);
    OpcUa_ReturnErrorIfArgumentNull(a_pStreamState);

    OpcUa_ReturnErrorIfTrue(a_pInputStream->Type != OpcUa_StreamType_Input,
                            OpcUa_BadInvalidArgument);

    *a_pStreamState = ((OpcUa_HttpInputStream*)a_pInputStream->Handle)->State;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

OpcUa_FinishErrorHandling;
}


/*============================================================================
 * OpcUa_HttpStream_CreateInput
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpStream_CreateInput(
    OpcUa_Socket                    a_hSocket,
    OpcUa_HttpStream_MessageType    a_eMessageType,
    OpcUa_InputStream**             a_ppInputStream)
{
    OpcUa_HttpInputStream* pHttpInputStream = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "CreateInput");

    OpcUa_ReturnErrorIfArgumentNull(a_hSocket);
    OpcUa_ReturnErrorIfArgumentNull(a_ppInputStream);
    
    OpcUa_ReturnErrorIfTrue(    a_eMessageType != OpcUa_HttpStream_MessageType_Request
                             && a_eMessageType != OpcUa_HttpStream_MessageType_Response,
                            OpcUa_BadInvalidArgument);

    *a_ppInputStream = OpcUa_Null;

    pHttpInputStream = (OpcUa_HttpInputStream*)OpcUa_Alloc(sizeof(OpcUa_HttpInputStream));
    OpcUa_GotoErrorIfAllocFailed(pHttpInputStream);
    OpcUa_MemSet(pHttpInputStream, 0, sizeof(OpcUa_HttpInputStream));

    pHttpInputStream->SanityCheck   = OpcUa_HttpInputStream_SanityCheck;
    pHttpInputStream->MessageType   = a_eMessageType;
    pHttpInputStream->Closed        = OpcUa_False;
    pHttpInputStream->Socket        = a_hSocket;
    pHttpInputStream->Callback      = OpcUa_Null;
    pHttpInputStream->CallbackData  = OpcUa_Null;
    pHttpInputStream->State         = OpcUa_HttpStream_State_Empty;
    
    OpcUa_String_Initialize(&pHttpInputStream->MessageLine);
    pHttpInputStream->LineStart     = OpcUa_BufferPosition_End;
    pHttpInputStream->LineEnd       = OpcUa_BufferPosition_End;

    *a_ppInputStream = (OpcUa_InputStream*)pHttpInputStream;

    (*a_ppInputStream)->Type            = OpcUa_StreamType_Input;
    (*a_ppInputStream)->Handle          = pHttpInputStream;
    (*a_ppInputStream)->GetPosition     = OpcUa_HttpStream_GetPosition;
    (*a_ppInputStream)->SetPosition     = OpcUa_HttpStream_SetPosition;
    (*a_ppInputStream)->GetChunkLength  = OpcUa_HttpStream_GetChunkLength;
    (*a_ppInputStream)->DetachBuffer    = OpcUa_HttpStream_DetachBuffer;
    (*a_ppInputStream)->AttachBuffer    = OpcUa_HttpStream_AttachBuffer;
    (*a_ppInputStream)->Close           = OpcUa_HttpStream_Close;
    (*a_ppInputStream)->Delete          = OpcUa_HttpStream_Delete;
    (*a_ppInputStream)->Read            = OpcUa_HttpStream_Read;
    (*a_ppInputStream)->NonBlocking     = OpcUa_False;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_Free(pHttpInputStream);
    OpcUa_Free(*a_ppInputStream);

    *a_ppInputStream = OpcUa_Null;

OpcUa_FinishErrorHandling;
}


/*============================================================================
 * OpcUa_HttpStream_CreateOutput
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_HttpStream_CreateOutput(  
    OpcUa_Socket                         a_hSocket,
    OpcUa_HttpStream_MessageType         a_eMessageType,
    OpcUa_StringA                        a_sMessageHeaders,
    OpcUa_HttpStream_PfnNotifyDisconnect a_pfnDisconnectCB,
    OpcUa_OutputStream**                 a_ppOutputStream)
{
    OpcUa_HttpOutputStream* pHttpOutputStream = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "CreateOutput");

    OpcUa_ReturnErrorIfTrue(    a_eMessageType != OpcUa_HttpStream_MessageType_Request 
                             && a_eMessageType != OpcUa_HttpStream_MessageType_Response,
                            OpcUa_BadInvalidArgument);

    OpcUa_ReturnErrorIfArgumentNull(a_ppOutputStream);

    *a_ppOutputStream = OpcUa_Null;

    /* allocate http out stream */
    pHttpOutputStream = (OpcUa_HttpOutputStream*)OpcUa_Alloc(sizeof(OpcUa_HttpOutputStream));
    OpcUa_GotoErrorIfAllocFailed(pHttpOutputStream);
    OpcUa_MemSet(pHttpOutputStream, 0, sizeof(OpcUa_HttpOutputStream));

    pHttpOutputStream->SanityCheck        = OpcUa_HttpOutputStream_SanityCheck;
    pHttpOutputStream->MessageType        = a_eMessageType; 
    pHttpOutputStream->Closed             = OpcUa_False;
    pHttpOutputStream->Socket             = a_hSocket;
    pHttpOutputStream->Callback           = OpcUa_Null;
    pHttpOutputStream->CallbackData       = OpcUa_Null;
    pHttpOutputStream->NotifyDisconnect   = a_pfnDisconnectCB;

    /* create message start line */
    if(a_eMessageType == OpcUa_HttpStream_MessageType_Request)
    {
        pHttpOutputStream->StartLine.RequestLine = (OpcUa_HttpRequestLine*)OpcUa_Alloc(sizeof(OpcUa_HttpRequestLine));
        OpcUa_GotoErrorIfAllocFailed(pHttpOutputStream->StartLine.RequestLine);
        OpcUa_HttpRequestLine_Initialize(pHttpOutputStream->StartLine.RequestLine);
    }
    else if(a_eMessageType == OpcUa_HttpStream_MessageType_Response)
    {
        pHttpOutputStream->StartLine.StatusLine = (OpcUa_HttpStatusLine*)OpcUa_Alloc(sizeof(OpcUa_HttpStatusLine));
        OpcUa_GotoErrorIfAllocFailed(pHttpOutputStream->StartLine.StatusLine);
        OpcUa_HttpStatusLine_Initialize(pHttpOutputStream->StartLine.StatusLine);
    }

    /* create header collection */

    if(a_sMessageHeaders != OpcUa_Null)
    {
        uStatus = OpcUa_HttpHeaderCollection_Parse(OpcUa_String_FromCString(a_sMessageHeaders),
                                                   &(pHttpOutputStream->Headers));
        OpcUa_GotoErrorIfBad(uStatus);
    }
    else
    {
        OpcUa_HttpHeaderCollection_Create(&(pHttpOutputStream->Headers));
        OpcUa_GotoErrorIfAllocFailed(pHttpOutputStream->Headers);        
    }

    /* create an internal buffer . */
    uStatus = OpcUa_Buffer_Initialize(&(pHttpOutputStream->Buffer),     /* instance        */
                                      OpcUa_Null,                       /* bufferdata      */
                                      0,                                /* buffersize      */
                                      OpcUa_HttpOutputStream_BlockSize, /* blocksize       */
                                      0,                                /* maxsize         */
                                      OpcUa_True);                      /* should be freed */
    OpcUa_GotoErrorIfBad(uStatus);

    *a_ppOutputStream                      = (OpcUa_OutputStream*)pHttpOutputStream;

    /* now initialize superclass members */
    (*a_ppOutputStream)->Type              = OpcUa_StreamType_Output;
    (*a_ppOutputStream)->Handle            = pHttpOutputStream;
    (*a_ppOutputStream)->GetPosition       = OpcUa_HttpStream_GetPosition;
    (*a_ppOutputStream)->SetPosition       = OpcUa_HttpStream_SetPosition;
    (*a_ppOutputStream)->GetChunkLength    = OpcUa_HttpStream_GetChunkLength;
    (*a_ppOutputStream)->DetachBuffer      = OpcUa_HttpStream_DetachBuffer;
    (*a_ppOutputStream)->AttachBuffer      = OpcUa_HttpStream_AttachBuffer;
    (*a_ppOutputStream)->Close             = OpcUa_HttpStream_Close;
    (*a_ppOutputStream)->Delete            = OpcUa_HttpStream_Delete;
    (*a_ppOutputStream)->Write             = OpcUa_HttpStream_Write;
    (*a_ppOutputStream)->Flush             = OpcUa_HttpStream_Flush;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_HttpStream_Delete((OpcUa_Stream**)&pHttpOutputStream);

    *a_ppOutputStream = OpcUa_Null;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpStream_CreateRequest
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpStream_CreateRequest(
    OpcUa_Socket                            a_hSocket,
    OpcUa_StringA                           a_sRequestMethod,
    OpcUa_StringA                           a_sRequestUri,
    OpcUa_StringA                           a_sRequestHeaders,
    OpcUa_HttpStream_PfnNotifyDisconnect    a_pfnDisconnectCB,
    OpcUa_OutputStream**                    a_ppOutputStream)
{
    OpcUa_HttpRequestLine* pHttpRequestLine = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "CreateRequest");

    OpcUa_ReturnErrorIfArgumentNull(a_sRequestMethod);
    OpcUa_ReturnErrorIfArgumentNull(a_sRequestUri);
    OpcUa_ReturnErrorIfArgumentNull(a_ppOutputStream);
    
    uStatus = OpcUa_HttpStream_CreateOutput(a_hSocket,
                                            OpcUa_HttpStream_MessageType_Request,
                                            a_sRequestHeaders,
                                            a_pfnDisconnectCB,
                                            a_ppOutputStream);
    OpcUa_GotoErrorIfBad(uStatus);

    pHttpRequestLine = ((OpcUa_HttpOutputStream*)((*a_ppOutputStream)->Handle))->StartLine.RequestLine;
    OpcUa_GotoErrorIfNull(pHttpRequestLine, OpcUa_BadInternalError);

    uStatus = OpcUa_String_AttachCopy(&(pHttpRequestLine->RequestMethod), a_sRequestMethod);
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = OpcUa_String_AttachCopy(&(pHttpRequestLine->RequestUri), a_sRequestUri);
    OpcUa_GotoErrorIfBad(uStatus);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
    
    OpcUa_HttpStream_Delete((OpcUa_Stream**)a_ppOutputStream);

    *a_ppOutputStream = OpcUa_Null;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpStream_CreateResponse
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpStream_CreateResponse(
    OpcUa_Socket                            a_hSocket,
    OpcUa_UInt32                            a_uStatusCode,
    OpcUa_StringA                           a_sReasonPhrase,
    OpcUa_StringA                           a_sResponseHeaders,
    OpcUa_Handle                            a_hConnection,
    OpcUa_HttpStream_PfnNotifyDisconnect    a_pfnDisconnectCB,
    OpcUa_OutputStream**                    a_ppOutputStream)
{
    OpcUa_HttpStatusLine* pHttpStatusLine = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "CreateResponse");

    OpcUa_ReturnErrorIfArgumentNull(a_ppOutputStream);

    /* only three-digit status codes are allowed */
    OpcUa_ReturnErrorIfTrue(a_uStatusCode < 100 || a_uStatusCode > 999, OpcUa_BadInvalidArgument);

    uStatus = OpcUa_HttpStream_CreateOutput(a_hSocket,
                                            OpcUa_HttpStream_MessageType_Response,
                                            a_sResponseHeaders,
                                            a_pfnDisconnectCB,
                                            a_ppOutputStream);
    OpcUa_GotoErrorIfBad(uStatus);

    pHttpStatusLine = ((OpcUa_HttpOutputStream*)((*a_ppOutputStream)->Handle))->StartLine.StatusLine;
    OpcUa_GotoErrorIfNull(pHttpStatusLine, OpcUa_BadInternalError);

    pHttpStatusLine->StatusCode = a_uStatusCode;

    if(a_sReasonPhrase != OpcUa_Null)
    {
        uStatus = OpcUa_String_AttachCopy(&(pHttpStatusLine->ReasonPhrase), a_sReasonPhrase);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    ((OpcUa_HttpOutputStream*)((*a_ppOutputStream)->Handle))->hConnection = a_hConnection;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_HttpStream_Delete((OpcUa_Stream**)a_ppOutputStream);

    *a_ppOutputStream = OpcUa_Null;

OpcUa_FinishErrorHandling;
}

static OpcUa_StatusCode OpcUa_HttpStream_Receive(
    OpcUa_InputStream*  a_pInputStream,
    OpcUa_UInt32        a_uCountExpected,
    OpcUa_UInt32*       a_pCountActual)
{
    OpcUa_HttpInputStream*  pHttpInputStream    = OpcUa_Null;
    OpcUa_Byte*             pBufferedData       = OpcUa_Null;
    OpcUa_UInt32            uBytesBuffered      = 0;
    OpcUa_UInt32            uBufferSize         = 0;

OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "Receive");

    OpcUa_ReturnErrorIfArgumentNull(a_pInputStream);
    OpcUa_ReturnErrorIfArgumentNull(a_pInputStream->Handle);
    OpcUa_ReturnErrorIfArgumentNull(a_pCountActual);

    pHttpInputStream = (OpcUa_HttpInputStream*)a_pInputStream->Handle;

    /* get the buffer markers */
    uBytesBuffered = pHttpInputStream->Buffer.EndOfData;
    uBufferSize    = pHttpInputStream->Buffer.Size;

    /* expand the buffer if required */
    if(uBytesBuffered + a_uCountExpected > uBufferSize)
    {
        uStatus = OpcUa_Buffer_SetLength(&pHttpInputStream->Buffer, 
                                         uBytesBuffered + a_uCountExpected);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    pBufferedData = pHttpInputStream->Buffer.Data;

    /* read data from the network */
    uStatus = OPCUA_P_SOCKET_READ(pHttpInputStream->Socket, 
                                  &(pBufferedData[uBytesBuffered]), 
                                  a_uCountExpected, 
                                  a_pCountActual);
    
    /* adjust the status code if necessary */
    if(OpcUa_IsBad(uStatus))
    {
        uStatus = (uStatus == OpcUa_BadWouldBlock)? OpcUa_GoodCallAgain: uStatus;
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* update the end of data marker */
    pHttpInputStream->Buffer.EndOfData = uBytesBuffered + *a_pCountActual;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    *a_pCountActual = 0;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpStream_ReadLine
 *===========================================================================*/
/** @brief reads a line of characters */
static OpcUa_StatusCode OpcUa_HttpStream_ReadLine(
    OpcUa_InputStream*  a_pInputStream,
    OpcUa_String*       a_pMessageLine)
{
    OpcUa_HttpInputStream*  pHttpInputStream    = OpcUa_Null;
    OpcUa_CharA*            pLineContent        = OpcUa_Null;
    OpcUa_CharA             chAsciiChar         = '\x00';
    OpcUa_UInt32            uCharCount          = 0;
    
OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "DataReady");

    OpcUa_ReturnErrorIfArgumentNull(a_pInputStream);
    OpcUa_ReturnErrorIfArgumentNull(a_pInputStream->Handle);
    OpcUa_ReturnErrorIfArgumentNull(a_pMessageLine);

    pHttpInputStream = (OpcUa_HttpInputStream*)a_pInputStream->Handle;    

    if(pHttpInputStream->LineStart == OpcUa_BufferPosition_End)
    {
        uStatus = OpcUa_Buffer_GetPosition(&pHttpInputStream->Buffer, 
                                           &pHttpInputStream->LineStart);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    for( ; ; )
    {
        uStatus = OpcUa_Buffer_ReadChar(&pHttpInputStream->Buffer, &chAsciiChar);
        
        if(uStatus == OpcUa_BadEndOfStream)
        {
            return OpcUa_GoodCallAgain;
        }

        OpcUa_GotoErrorIfBad(uStatus);

        if(chAsciiChar == '\r')
        {
            uStatus = OpcUa_Buffer_PeekChar(&pHttpInputStream->Buffer, &chAsciiChar);
            
            if(uStatus == OpcUa_BadEndOfStream)
            {
                OpcUa_UInt32 uCharPosition = 0;

                /* get a position of the current character */
                uStatus = OpcUa_Buffer_GetPosition(&pHttpInputStream->Buffer, &uCharPosition);
                OpcUa_GotoErrorIfBad(uStatus);

                /* move one character back */
                uStatus = OpcUa_Buffer_SetPosition(&pHttpInputStream->Buffer, uCharPosition - 1);
                OpcUa_GotoErrorIfBad(uStatus);

                return OpcUa_GoodCallAgain;
            }

            if(chAsciiChar == '\n')
            {
                uStatus = OpcUa_Buffer_ReadChar(&pHttpInputStream->Buffer, &chAsciiChar);
                OpcUa_GotoErrorIfBad(uStatus);

                break;
            }
        }
    }

    uStatus = OpcUa_Buffer_GetPosition(&pHttpInputStream->Buffer, 
                                       &pHttpInputStream->LineEnd);
    OpcUa_GotoErrorIfBad(uStatus);

    uCharCount = pHttpInputStream->LineEnd - pHttpInputStream->LineStart;
    OpcUa_GotoErrorIfTrue(uCharCount < 2, OpcUa_BadInternalError);

    /* do not include CR and LF characters into the resulting string */
    uCharCount -= 2;

    pLineContent = (OpcUa_CharA*)&(pHttpInputStream->Buffer.Data[pHttpInputStream->LineStart]);

    uStatus = OpcUa_String_AttachToString(pLineContent, 
                                          uCharCount, 
                                          uCharCount, 
                                          OpcUa_True,
                                          OpcUa_True,
                                          a_pMessageLine);
    OpcUa_GotoErrorIfBad(uStatus);

    pHttpInputStream->LineStart = OpcUa_BufferPosition_End;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    if(pHttpInputStream != OpcUa_Null)
    {
        pHttpInputStream->LineStart = OpcUa_BufferPosition_End;
        pHttpInputStream->LineEnd   = OpcUa_BufferPosition_End;

        OpcUa_String_Clear(&pHttpInputStream->MessageLine);
    }

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpStream_ProcessHeaders
 *===========================================================================*/
/** @brief processes message headers */
static OpcUa_StatusCode OpcUa_HttpStream_ProcessHeaders(
    OpcUa_InputStream*  a_pInputStream)
{
    OpcUa_HttpInputStream*  pHttpInputStream    = OpcUa_Null;
    OpcUa_HttpHeader*       pHttpHeader         = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "ProcessHeaders");
    
    OpcUa_ReturnErrorIfArgumentNull(a_pInputStream);
    OpcUa_ReturnErrorIfArgumentNull(a_pInputStream->Handle);

    pHttpInputStream = (OpcUa_HttpInputStream*)a_pInputStream->Handle;

    pHttpInputStream->ContentLength = 0;

    OpcUa_List_ResetCurrent(pHttpInputStream->Headers);
    pHttpHeader = (OpcUa_HttpHeader*)OpcUa_List_GetCurrentElement(pHttpInputStream->Headers);

    while(pHttpHeader != OpcUa_Null)
    {
        if(OpcUa_StrCmpA(OpcUa_String_GetRawString(&pHttpHeader->Name), "Content-Length") == 0)
        {
            if(OpcUa_String_IsEmpty(&pHttpHeader->Name))
            {
                uStatus = OpcUa_BadInternalError;
                break;
            }

            pHttpInputStream->ContentLength = (OpcUa_UInt32)OpcUa_CharAToInt(OpcUa_String_GetRawString(&pHttpHeader->Value));
        }

        if(pHttpInputStream->MessageType == OpcUa_HttpStream_MessageType_Request)
        {
            if(OpcUa_StrCmpA(OpcUa_String_GetRawString(&pHttpHeader->Name), "Expect") == 0)
            {
                if(!OpcUa_String_IsEmpty(&pHttpHeader->Name))
                {
                    if(OpcUa_StrCmpA(OpcUa_String_GetRawString(&pHttpHeader->Value), "100-continue") == 0)
                    {
                        OpcUa_CharA* sHttpResponse   = "HTTP/1.1 100 Continue\r\n\r\n";
                        OpcUa_UInt32 uResponseLength = OpcUa_StrLenA(sHttpResponse);

                        OPCUA_P_SOCKET_WRITE(pHttpInputStream->Socket, 
                                             (OpcUa_Byte*)sHttpResponse,
                                             uResponseLength, 
                                             OpcUa_True);
                    }
                }
            }
        }

        pHttpHeader = (OpcUa_HttpHeader*)OpcUa_List_GetNextElement(pHttpInputStream->Headers);
    }


OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpStream_DataReady
 *===========================================================================*/
/** @brief Called if data is available for reading on a socket attached to a stream.
  *
  * This is kind of a read event handler of the pHttpInputStream. The Listener
  * calls this function, if new data is available on the socket. Dependend of
  * the stream state, it starts handling the http stream relevant data and
  * gives feedback to the listener, which takes further action, ie. calls
  * the handler.
  *
  * @param a_pIstrm [ in] The stream for which data is ready to be received.
  *
  * @return StatusCode
  */
OpcUa_StatusCode OpcUa_HttpStream_DataReady(OpcUa_InputStream* a_pInputStream)
{
    OpcUa_HttpInputStream*  pHttpInputStream    = OpcUa_Null;
    OpcUa_UInt32            uExpectedLength     = OpcUa_HttpInputStream_MaxLineLength;
    OpcUa_UInt32            uActualLength       = 0;
    OpcUa_String            sMessageLine        = OPCUA_STRING_STATICINITIALIZER;
    OpcUa_HttpHeader*       pMessageHeader      = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_HttpStream, "DataReady");

    OpcUa_ReturnErrorIfArgumentNull(a_pInputStream);
    OpcUa_ReturnErrorIfArgumentNull(a_pInputStream->Handle);

    pHttpInputStream = (OpcUa_HttpInputStream*)a_pInputStream->Handle;

    if(pHttpInputStream->State == OpcUa_HttpStream_State_Empty)
    {
        /* This is a new stream and a new message. */
        OpcUa_Byte* pBufferData = (OpcUa_Byte*)OpcUa_Alloc(OpcUa_HttpInputStream_BlockSize);
        OpcUa_ReturnErrorIfAllocFailed(pBufferData);
        
        uStatus = OpcUa_Buffer_Initialize(&pHttpInputStream->Buffer,
                                          pBufferData,
                                          OpcUa_HttpInputStream_BlockSize,
                                          OpcUa_HttpInputStream_BlockSize,
                                          0,
                                          OpcUa_True);
        if(OpcUa_IsBad(uStatus))
        {
            OpcUa_Buffer_Clear(&pHttpInputStream->Buffer);
            OpcUa_ReturnStatusCode;
        }

        OpcUa_Buffer_SetEmpty(&pHttpInputStream->Buffer);

        uStatus = OpcUa_HttpHeaderCollection_Create(&pHttpInputStream->Headers);
        OpcUa_GotoErrorIfBad(uStatus);
    }
    else if(pHttpInputStream->State == OpcUa_HttpStream_State_Body)
    {
        /* data has already been received into the stream buffer
           calculate length of data to read */        
        uActualLength = pHttpInputStream->Buffer.EndOfData - pHttpInputStream->Buffer.Position;
        uExpectedLength = pHttpInputStream->ContentLength - uActualLength;
    }

    /* receive data from the network */
    uStatus = OpcUa_HttpStream_Receive(a_pInputStream, uExpectedLength, &uActualLength);
    OpcUa_GotoErrorIfBad(uStatus);

    switch(pHttpInputStream->State)
    {
        case OpcUa_HttpStream_State_Empty:
        {
            pHttpInputStream->State = OpcUa_HttpStream_State_StartLine;
        }

        case OpcUa_HttpStream_State_StartLine:
        {
            /* read message start line */
            uStatus = OpcUa_HttpStream_ReadLine(a_pInputStream, &sMessageLine);
            OpcUa_GotoErrorIfBad(uStatus);

            uStatus = OpcUa_String_IsEmpty(&sMessageLine)? OpcUa_GoodCallAgain: uStatus;

            if(uStatus == OpcUa_GoodCallAgain)
            {
                break;
            }

            if(pHttpInputStream->MessageType == OpcUa_HttpStream_MessageType_Request)
            {
                uStatus = OpcUa_HttpRequestLine_Parse(&sMessageLine, &(pHttpInputStream->StartLine.RequestLine));
                OpcUa_GotoErrorIfBad(uStatus);
            }
            else if(pHttpInputStream->MessageType == OpcUa_HttpStream_MessageType_Response)
            {
                uStatus = OpcUa_HttpStatusLine_Parse(&sMessageLine, &(pHttpInputStream->StartLine.StatusLine));
                OpcUa_GotoErrorIfBad(uStatus);
            }

            pHttpInputStream->State = OpcUa_HttpStream_State_Headers;
        }

        case OpcUa_HttpStream_State_Headers:
        {
            for( ; ; )
            {
                uStatus = OpcUa_HttpStream_ReadLine(a_pInputStream, &sMessageLine);
                OpcUa_GotoErrorIfBad(uStatus);

                if(uStatus == OpcUa_GoodCallAgain || OpcUa_String_IsEmpty(&sMessageLine))
                {
                    break;
                }
                
                uStatus = OpcUa_HttpHeader_Parse(&sMessageLine, &pMessageHeader);
                OpcUa_GotoErrorIfBad(uStatus);

                uStatus = OpcUa_HttpHeaderCollection_AddHeader(pHttpInputStream->Headers, pMessageHeader);
                OpcUa_GotoErrorIfBad(uStatus);
            }

            if(uStatus == OpcUa_GoodCallAgain)
            {
                break;
            }

            uStatus = OpcUa_HttpStream_ProcessHeaders(a_pInputStream);
            OpcUa_GotoErrorIfBad(uStatus);
            
            if(pHttpInputStream->MessageType == OpcUa_HttpStream_MessageType_Response)
            {
                /* response is being received, check whether the message body is allowed */
                if((pHttpInputStream->StartLine.StatusLine->StatusCode >= 100) && 
                   (pHttpInputStream->StartLine.StatusLine->StatusCode <  200))
                {
                    /* informational response (1xx) */
                    pHttpInputStream->State = OpcUa_HttpStream_State_MessageComplete;
                }

                if((pHttpInputStream->StartLine.StatusLine->StatusCode == 204) ||
                   (pHttpInputStream->StartLine.StatusLine->StatusCode == 304))
                {
                    /* no content (204) or not modified response (304) */
                    pHttpInputStream->State = OpcUa_HttpStream_State_MessageComplete;
                }
            }

            if(pHttpInputStream->State == OpcUa_HttpStream_State_MessageComplete)
            {
                pHttpInputStream->ContentLength = 0;    
            }

            if(pHttpInputStream->ContentLength > 0)
            {
                pHttpInputStream->State = OpcUa_HttpStream_State_Body;
            }
            else
            {
                pHttpInputStream->State = OpcUa_HttpStream_State_MessageComplete;
            }
        }

        case OpcUa_HttpStream_State_Body:
        {
            if(pHttpInputStream->ContentLength > 0)
            {
                uActualLength = pHttpInputStream->Buffer.EndOfData - pHttpInputStream->Buffer.Position;

                if(uActualLength < pHttpInputStream->ContentLength)
                {
                    uExpectedLength = pHttpInputStream->ContentLength - uActualLength;

                    uStatus = OpcUa_HttpStream_Receive(a_pInputStream, uExpectedLength, &uActualLength);
                    OpcUa_GotoErrorIfBad(uStatus);
                }

                uActualLength = pHttpInputStream->Buffer.EndOfData - pHttpInputStream->Buffer.Position;

                if(uStatus == OpcUa_GoodCallAgain)
                {
                    break;
                }

                if(uActualLength < pHttpInputStream->ContentLength)
                {
                    uStatus = OpcUa_GoodCallAgain;
                    break;
                }
            }

            pHttpInputStream->State = OpcUa_HttpStream_State_MessageComplete;
        }

        case OpcUa_HttpStream_State_MessageComplete:
        {
            break;
        }

        default:
        {
            uStatus = OpcUa_BadInternalError;
            break;
        }
    }

    OpcUa_String_Clear(&sMessageLine);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
    
    OpcUa_HttpHeader_Clear(pMessageHeader);
    OpcUa_String_Clear(&sMessageLine);

OpcUa_FinishErrorHandling;
}

#endif /* OPCUA_HAVE_HTTPAPI */
