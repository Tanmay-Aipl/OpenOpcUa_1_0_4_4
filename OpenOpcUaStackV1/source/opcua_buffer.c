/*****************************************************************************
      Author
        �. Michel Condemine, 4CE Industry (2010-2012)
      
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
#include <opcua_memory.h>
#include <opcua_buffer.h>




/*============================================================================
 * OpcUa_Buffer_SanityCheck
 *===========================================================================*/
#define OpcUa_Buffer_SanityCheck 0x43824B55

/*============================================================================
 * OpcUa_ValidateBuffer
 *===========================================================================*/
#define OpcUa_ReturnErrorIfInvalidBuffer() \
if (((OpcUa_Buffer*)a_Handle)->SanityCheck != OpcUa_Buffer_SanityCheck) \
{ \
    return OpcUa_BadInvalidArgument; \
}

/*============================================================================
 * OpcUa_Buffer_Initialize
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Buffer_Initialize(   OpcUa_Buffer* a_pBuffer,
                                            OpcUa_Byte*   a_pbyData,
                                            OpcUa_UInt32  a_uDataSize,
                                            OpcUa_UInt32  a_uBlockSize,
                                            OpcUa_UInt32  a_uMaxSize,
                                            OpcUa_Boolean a_bFreeBuffer)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Buffer);

    OpcUa_ReturnErrorIfArgumentNull(a_pBuffer);

    OpcUa_MemSet(a_pBuffer, 0, sizeof(OpcUa_Buffer));

    a_pBuffer->SanityCheck = OpcUa_Buffer_SanityCheck;
    a_pBuffer->Size        = (a_pbyData != OpcUa_Null)?a_uBlockSize:0;
    a_pBuffer->EndOfData   = a_uDataSize;
    a_pBuffer->Position    = 0;
    a_pBuffer->BlockSize   = (a_uBlockSize > 0)? a_uBlockSize: 1;
    a_pBuffer->MaxSize     = a_uMaxSize;
    /* position must not behind buffer */
    OpcUa_ReturnErrorIfTrue((a_pBuffer->Position > a_pBuffer->Size), OpcUa_BadInvalidArgument);

    /* EndOfData must not behind buffer */
    OpcUa_ReturnErrorIfTrue((a_pBuffer->EndOfData > a_pBuffer->Size), OpcUa_BadInvalidArgument);

    a_pBuffer->Data        = a_pbyData;

    a_pBuffer->FreeBuffer  = a_bFreeBuffer;

    return OpcUa_Good; /* nothing can go wrong beside invalid arguments */
}


/*============================================================================
 * OpcUa_Buffer_Create
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Buffer_Create(   OpcUa_Byte*    a_pbyData,
                                        OpcUa_UInt32   a_uDataSize,
                                        OpcUa_UInt32   a_uBlockSize,
                                        OpcUa_UInt32   a_uMaxSize,
                                        OpcUa_Boolean  a_bFreeBuffer,
                                        OpcUa_Buffer** a_ppBuffer)
{
    OpcUa_StatusCode    uStatus = OpcUa_Good;
    OpcUa_Buffer*       pBuffer = OpcUa_Null;

    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Buffer);

    OpcUa_ReturnErrorIfArgumentNull(a_ppBuffer);

    *a_ppBuffer = OpcUa_Null;

    pBuffer = (OpcUa_Buffer*)OpcUa_Alloc(sizeof(OpcUa_Buffer));
    OpcUa_ReturnErrorIfAllocFailed(pBuffer);

    uStatus = OpcUa_Buffer_Initialize(  pBuffer,
                                        a_pbyData,
                                        a_uDataSize,
                                        a_uBlockSize,
                                        a_uMaxSize,
                                        a_bFreeBuffer);

    *a_ppBuffer = pBuffer;

    return uStatus;
}



/*============================================================================
 * OpcUa_Buffer_Delete
 *===========================================================================*/
OpcUa_Void OpcUa_Buffer_Delete(OpcUa_Buffer** handle)
{   
    if (handle != OpcUa_Null && *handle != OpcUa_Null)
    {
        OpcUa_Buffer* buffer = (OpcUa_Buffer*)*handle;

        if(buffer->FreeBuffer)
        {
            OpcUa_Free(buffer->Data);
        }

        OpcUa_Free(buffer);

        *handle = OpcUa_Null;
    }
}

/*============================================================================
 * OpcUa_Buffer_Clear
 *===========================================================================*/
OpcUa_Void OpcUa_Buffer_Clear(OpcUa_Buffer* a_pBuffer)
{   
    if(a_pBuffer != OpcUa_Null)
    {
        if(a_pBuffer->FreeBuffer)
        {
            OpcUa_Free(a_pBuffer->Data);
        }

        OpcUa_MemSet(a_pBuffer, 0, sizeof(OpcUa_Buffer));
    }
}

/*============================================================================
 * OpcUa_Buffer_Read
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Buffer_Read(
    OpcUa_Handle  a_Handle,
    OpcUa_Byte*   data,
    OpcUa_UInt32* count)
{
    OpcUa_UInt32 bytesToRead = 0;   
    OpcUa_Buffer* buffer = OpcUa_Null;
    OpcUa_StatusCode uStatus = OpcUa_Good;

    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Buffer);

    OpcUa_ReturnErrorIfArgumentNull(a_Handle);
    OpcUa_ReturnErrorIfArgumentNull(data);
    OpcUa_ReturnErrorIfArgumentNull(count);

    OpcUa_ReturnErrorIfInvalidBuffer();

    buffer = (OpcUa_Buffer*)a_Handle;

    if (*count == 0)
    {
        return OpcUa_Good;
    }

    bytesToRead = buffer->EndOfData - buffer->Position;

    if (bytesToRead == 0)
    {
        *count = 0;
        return OpcUa_BadEndOfStream;
    }

    if (bytesToRead > *count)
    {
        bytesToRead = *count;
    }

    OpcUa_MemCpy(data, *count, buffer->Data+buffer->Position, bytesToRead);
    buffer->Position += bytesToRead;

    *count = bytesToRead;

    return uStatus;
}

/*============================================================================
 * OpcUa_Buffer_Skip
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Buffer_Skip(
    OpcUa_Handle  a_Handle,
    OpcUa_UInt32  a_uLength)
{
    OpcUa_UInt32    uBytesAvailable = 0;   
    OpcUa_Buffer*   pBuffer         = OpcUa_Null;

    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Buffer);
    OpcUa_ReturnErrorIfArgumentNull(a_Handle);
    OpcUa_ReturnErrorIfInvalidBuffer();

    /* we're done skip length is zero */
    if(a_uLength == 0)
    {
        return OpcUa_Good;
    }

    pBuffer = (OpcUa_Buffer*)a_Handle;

    /* check if remaining buffer content is larger than amount of bytes to skip */
    uBytesAvailable = pBuffer->EndOfData - pBuffer->Position;

    if(uBytesAvailable == 0 || uBytesAvailable < a_uLength)
    {
        return OpcUa_BadEndOfStream;
    }

    /* set current position to new value */
    pBuffer->Position += a_uLength;

    return OpcUa_Good;
}

/*============================================================================
 * OpcUa_Buffer_Write
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Buffer_Write(
    OpcUa_Handle a_Handle,
    OpcUa_Byte*  a_pData,
    OpcUa_UInt32 a_uCount)
{
    OpcUa_UInt32        bytesAvailable = 0; 
    OpcUa_Buffer*       buffer = OpcUa_Null;
    OpcUa_StatusCode    uStatus = OpcUa_Good;

    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Buffer);

    OpcUa_ReturnErrorIfArgumentNull(a_Handle);
    OpcUa_ReturnErrorIfArgumentNull(a_pData);

    OpcUa_ReturnErrorIfInvalidBuffer();

    buffer = (OpcUa_Buffer*)a_Handle;

    bytesAvailable = buffer->Size - buffer->Position;

    if(bytesAvailable < a_uCount)
    {
        /* Try to prevent to get into this codepath by setting the buffer big enough. */

        OpcUa_Byte*  newData = OpcUa_Null;
        OpcUa_UInt32 newSize = buffer->Size;

        /* all or nothing write - fail if the entire block can't be written */
        if(     buffer->MaxSize != 0 
            &&  buffer->Size + a_uCount > buffer->MaxSize)
        {
            return OpcUa_BadEndOfStream;
        }

        newSize += (buffer->BlockSize * (((a_uCount - bytesAvailable-1)/buffer->BlockSize)+1));

        /* Reallocate new buffer */
        newData = (OpcUa_Byte *)OpcUa_ReAlloc(buffer->Data, newSize);

        if (newData == OpcUa_Null)
        {
            return OpcUa_BadOutOfMemory;
        }

        buffer->Data = newData;
        buffer->Size = newSize;
        
        bytesAvailable = buffer->Size - buffer->Position;
    }

    /* copy new data into buffer */
    OpcUa_MemCpy(   buffer->Data+buffer->Position, 
                    bytesAvailable, 
                    a_pData, 
                    a_uCount);

    buffer->Position += a_uCount;

    /* update end of data marker */
    if (buffer->EndOfData < buffer->Position)
    {
        buffer->EndOfData = buffer->Position;
    }


    return uStatus;
}

/*============================================================================
 * OpcUa_Buffer_GetPosition
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Buffer_GetPosition(
    OpcUa_Handle  a_Handle,
    OpcUa_UInt32* position)
{
    OpcUa_Buffer* buffer = OpcUa_Null;
    OpcUa_StatusCode uStatus = OpcUa_Good;

    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Buffer);

    OpcUa_ReturnErrorIfArgumentNull(a_Handle);
    OpcUa_ReturnErrorIfArgumentNull(position);
    OpcUa_ReturnErrorIfInvalidBuffer();

    buffer = (OpcUa_Buffer*)a_Handle;
    
    *position = buffer->Position;

    return uStatus;
}

/*============================================================================
 * OpcUa_Buffer_SetPosition
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Buffer_SetPosition(
    OpcUa_Handle a_Handle,
    OpcUa_UInt32 a_uPosition)
{
    OpcUa_Buffer* pBuffer = OpcUa_Null;

    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Buffer);

    OpcUa_ReturnErrorIfArgumentNull(a_Handle);
    OpcUa_ReturnErrorIfInvalidBuffer();

    pBuffer = (OpcUa_Buffer*)a_Handle;
    
    if (a_uPosition == OpcUa_BufferPosition_Start)
    {
        pBuffer->Position = 0;
        return OpcUa_Good;
    }

    if (a_uPosition == OpcUa_BufferPosition_End)
    {
        pBuffer->Position = pBuffer->EndOfData;
        return OpcUa_Good;
    }

    if (a_uPosition > pBuffer->EndOfData)
    {
        return OpcUa_BadEndOfStream;
    }

    pBuffer->Position = a_uPosition;
    return OpcUa_Good;
}

/*============================================================================
 * OpcUa_Buffer_SetEmpty
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Buffer_SetEmpty(OpcUa_Buffer* a_pBuffer)
{
    OpcUa_ReturnErrorIfArgumentNull(a_pBuffer);

    a_pBuffer->EndOfData = 0;
    a_pBuffer->Position  = 0;

    return OpcUa_Good;
}

/*============================================================================
 * OpcUa_Buffer_IsEmpty
 *===========================================================================*/
OpcUa_Boolean OpcUa_Buffer_IsEmpty(OpcUa_Buffer* a_pBuffer)
{
    return (((a_pBuffer == OpcUa_Null)||(a_pBuffer->EndOfData==0))?OpcUa_True:OpcUa_False); 
}

/*============================================================================
 * OpcUa_Buffer_SetEndOfData
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Buffer_SetEndOfData(
    OpcUa_Handle a_Handle,
    OpcUa_UInt32 a_uEndOfData)
{
    OpcUa_Buffer* pBuffer = OpcUa_Null;

    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Buffer);

    OpcUa_ReturnErrorIfArgumentNull(a_Handle);
    OpcUa_ReturnErrorIfInvalidBuffer();

    pBuffer = (OpcUa_Buffer*)a_Handle;
    
    if(a_uEndOfData > pBuffer->Size)
    {
        return OpcUa_BadEndOfStream;
    }

    pBuffer->EndOfData = a_uEndOfData;

    return OpcUa_Good;
}
/*============================================================================
 * OpcUa_Buffer_GetData
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Buffer_GetData(
    OpcUa_Handle  a_Handle,
    OpcUa_Byte**  a_ppData,
    OpcUa_UInt32* a_uLength)
{
    OpcUa_Buffer* pBuffer = OpcUa_Null;
    OpcUa_StatusCode uStatus = OpcUa_Good;

    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Buffer);

    OpcUa_ReturnErrorIfArgumentNull(a_Handle);

    OpcUa_ReturnErrorIfInvalidBuffer();

    pBuffer = (OpcUa_Buffer*)a_Handle;
    
    if(a_ppData != OpcUa_Null)
    {
        *a_ppData = pBuffer->Data;
    }

    if(a_uLength != OpcUa_Null)
    {
        *a_uLength = pBuffer->EndOfData;
    }

    return uStatus;
}
