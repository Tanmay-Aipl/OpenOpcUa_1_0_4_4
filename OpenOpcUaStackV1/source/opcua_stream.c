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
#include <opcua_stream.h>


/*============================================================================
 * OpcUa_Stream_Read
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Stream_Read(
    struct _OpcUa_InputStream*     istrm, 
    OpcUa_Byte*                    buffer, 
    OpcUa_UInt32*                  count, 
    OpcUa_Stream_PfnOnReadyToRead* callback, 
    OpcUa_Void*                    callbackData)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Stream);
    OpcUa_ReturnErrorIfArgumentNull(istrm);
    OpcUa_ReturnErrorIfArgumentNull(istrm->Read);

    return istrm->Read(istrm, buffer, count, callback, callbackData);
}

/*============================================================================
 * OpcUa_Stream_Write
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Stream_Write(
    struct _OpcUa_OutputStream* ostrm, 
    OpcUa_Byte*                 buffer, 
    OpcUa_UInt32                count)
{
    /*OpcUa_StatusCode uStatus = OpcUa_Good;*/
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Stream);

    OpcUa_ReturnErrorIfArgumentNull(ostrm);
    OpcUa_ReturnErrorIfArgumentNull(ostrm->Write);

    return ostrm->Write(ostrm, buffer, count);
}

/*============================================================================
 * OpcUa_Stream_Flush
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Stream_Flush(
    struct _OpcUa_OutputStream* ostrm,
    OpcUa_Boolean               lastCall)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Stream);
    OpcUa_ReturnErrorIfArgumentNull(ostrm);
    OpcUa_ReturnErrorIfArgumentNull(ostrm->Flush);

    return ostrm->Flush(ostrm, lastCall);
}

/*============================================================================
 * OpcUa_Stream_Close
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Stream_Close(
    struct _OpcUa_Stream* strm)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Stream);
    OpcUa_ReturnErrorIfArgumentNull(strm);
    OpcUa_ReturnErrorIfArgumentNull(strm->Close);

    return strm->Close(strm);
}

/*============================================================================
 * OpcUa_Stream_GetChunkLength
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Stream_GetChunkLength(
    struct _OpcUa_Stream* strm, 
    OpcUa_UInt32*         length)
{
    OpcUa_ReturnErrorIfArgumentNull(strm);
    OpcUa_ReturnErrorIfArgumentNull(length);

    return strm->GetChunkLength(strm, length);
}

/*============================================================================
 * OpcUa_Stream_AttachBuffer
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Stream_AttachBuffer(
    struct _OpcUa_Stream*   strm,
    OpcUa_Buffer*           buffer)
{
    OpcUa_ReturnErrorIfArgumentNull(strm);
    OpcUa_ReturnErrorIfArgumentNull(buffer);

    return strm->AttachBuffer(strm, buffer);
}


/*============================================================================
 * OpcUa_Stream_DetachBuffer
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Stream_DetachBuffer(
    struct _OpcUa_Stream*   strm,
    OpcUa_Buffer*           buffer,
    OpcUa_Boolean*          moreData)
{
    OpcUa_ReturnErrorIfArgumentNull(strm);
    OpcUa_ReturnErrorIfArgumentNull(buffer);

    return strm->DetachBuffer(strm, buffer, moreData);
}

/*============================================================================
 * OpcUa_Stream_Delete
 *===========================================================================*/
OpcUa_Void OpcUa_Stream_Delete(
    struct _OpcUa_Stream** strm)
{
    if (strm != OpcUa_Null && *strm != OpcUa_Null)
    {
        (*strm)->Delete(strm);
    }
}

/*============================================================================
 * OpcUa_Stream_GetPosition
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Stream_GetPosition(
    struct _OpcUa_Stream* strm, 
    OpcUa_UInt32*         position)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Stream);
    OpcUa_ReturnErrorIfArgumentNull(strm);
    OpcUa_ReturnErrorIfArgumentNull(strm->GetPosition);

    return strm->GetPosition(strm, position);
}

/*============================================================================
 * OpcUa_Stream_SetPosition
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Stream_SetPosition(
    struct _OpcUa_Stream* strm, 
    OpcUa_UInt32          position)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Stream);
    OpcUa_ReturnErrorIfArgumentNull(strm);
    OpcUa_ReturnErrorIfArgumentNull(strm->SetPosition);

    return strm->SetPosition(strm, position);
}
