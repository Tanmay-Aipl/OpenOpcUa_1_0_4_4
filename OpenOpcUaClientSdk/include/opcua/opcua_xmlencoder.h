/* ========================================================================
 * Copyright (c) 2005-2009 The OPC Foundation, Inc. All rights reserved.
 *
 * OPC Foundation MIT License 1.00
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * The complete license agreement can be found here:
 * http://opcfoundation.org/License/MIT/1.00/
 * ======================================================================*/
// New file from RCL project
#include <opcua_builtintypes.h>
#include <opcua_stream.h>
#include <opcua_messagecontext.h>
#include <opcua_encoder.h>
#include <opcua_decoder.h>

#ifndef _OpcUa_XmlEncoder_H_
#define _OpcUa_XmlEncoder_H_ 1

OPCUA_BEGIN_EXTERN_C

/** 
  @brief Used to create an instance of a Xml encoder.
 
  @param ppEncoder [out] The encoder.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlEncoder_Create(
    OpcUa_Encoder**       ppEncoder);


/** 
  @brief Used to create an instance of a Xml decoder.
 
  @param ppDecoder [out] The decoder.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_XmlDecoder_Create(
    OpcUa_Decoder** ppDecoder);

/** 
  @brief Writes a Boolean value.
 
  @param bValue [in] The value to encode.
  @param pOstrm [in] The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Boolean_XmlEncode(
    OpcUa_Boolean       bValue,
    OpcUa_OutputStream* pOstrm);

/** 
  @brief Reads a Boolean value.
 
  @param pValue [out] The decoded value.
  @param pIstrm [in]  The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Boolean_XmlDecode(
    OpcUa_Boolean*     pValue,
    OpcUa_InputStream* pIstrm);

/** 
  @brief Writes a SByte value.
 
  @param nValue [in] The value to encode.
  @param pOstrm [in] The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_SByte_XmlEncode(
    OpcUa_SByte         nValue,
    OpcUa_OutputStream* pOstrm);

/** 
  @brief Reads a SByte value.
 
  @param pValue [out] The decoded value.
  @param pIstrm [in]  The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_SByte_XmlDecode(
    OpcUa_SByte*       pValue,
    OpcUa_InputStream* pIstrm);

/** 
  @brief Writes a Byte value.
 
  @param nValue [in] The value to encode.
  @param pOstrm [in] The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Byte_XmlEncode(
    OpcUa_Byte          nValue,
    OpcUa_OutputStream* pOstrm);
/** 
  @brief Reads a Byte value.
 
  @param pValue [out] The decoded value.
  @param pIstrm [in]  The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Byte_XmlDecode(
    OpcUa_Byte*        pValue,
    OpcUa_InputStream* pIstrm);

/** 
  @brief Writes a Int16 value.
 
  @param nValue [in] The value to encode.
  @param pOstrm [in] The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Int16_XmlEncode(
    OpcUa_Int16         nValue,
    OpcUa_OutputStream* pOstrm);
/** 
  @brief Reads a Int16 value.
 
  @param pValue [out] The decoded value.
  @param pIstrm [in]  The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Int16_XmlDecode(
    OpcUa_Int16*       pValue,
    OpcUa_InputStream* pIstrm);

/** 
  @brief Writes a UInt16 value.
 
  @param nValue [in] The value to encode.
  @param pOstrm [in] The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_UInt16_XmlEncode(
    OpcUa_UInt16        nValue,
    OpcUa_OutputStream* pOstrm);

/** 
  @brief Reads a UInt16 value.
 
  @param pValue [out] The decoded value.
  @param pIstrm [in]  The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_UInt16_XmlDecode(
    OpcUa_UInt16*      pValue,
    OpcUa_InputStream* pIstrm);
/** 
  @brief Writes a Int32 value.
 
  @param nValue [in] The value to encode.
  @param pOstrm [in] The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Int32_XmlEncode(
    OpcUa_Int32         nValue,
    OpcUa_OutputStream* pOstrm);

/** 
  @brief Reads a Int32 value.
 
  @param pValue [out] The decoded value.
  @param pIstrm [in]  The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Int32_XmlDecode(
    OpcUa_Int32*       pValue,
    OpcUa_InputStream* pIstrm);

/** 
  @brief Writes a UInt32 value.
 
  @param nValue [in] The value to encode.
  @param pOstrm [in] The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_UInt32_XmlEncode(
    OpcUa_UInt32        nValue,
    OpcUa_OutputStream* pOstrm);

/** 
  @brief Reads a UInt32 value.
 
  @param pValue [out] The decoded value.
  @param pIstrm [in]  The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_UInt32_XmlDecode(
    OpcUa_UInt32*      pValue,
    OpcUa_InputStream* pIstrm);

/** 
  @brief Writes a Int64 value.
 
  @param nValue [in] The value to encode.
  @param pOstrm [in] The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Int64_XmlEncode(
    OpcUa_Int64         nValue,
    OpcUa_OutputStream* pOstrm);

/** 
  @brief Reads a Int64 value.
 
  @param pValue [out] The decoded value.
  @param pIstrm [in]  The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Int64_XmlDecode(
    OpcUa_Int64*       pValue,
    OpcUa_InputStream* pIstrm);

/** 
  @brief Writes a UInt64 value.
 
  @param nValue [in] The value to encode.
  @param pOstrm [in] The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_UInt64_XmlEncode(
    OpcUa_UInt64        nValue,
    OpcUa_OutputStream* pOstrm);

/** 
  @brief Reads a UInt64 value.
 
  @param pValue [out] The decoded value.
  @param pIstrm [in]  The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_UInt64_XmlDecode(
    OpcUa_UInt64*      pValue,
    OpcUa_InputStream* pIstrm);

/** 
  @brief Writes a Float value.
 
  @param nValue [in] The value to encode.
  @param pOstrm [in] The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Float_XmlEncode(
    OpcUa_Float         nValue,
    OpcUa_OutputStream* pOstrm);

/** 
  @brief Reads a Float value.
 
  @param pValue [out] The decoded value.
  @param pIstrm [in]  The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Float_XmlDecode(
    OpcUa_Float*       pValue,
    OpcUa_InputStream* pIstrm);

/** 
  @brief Writes a Double value.
 
  @param nValue [in] The value to encode.
  @param pOstrm [in] The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Double_XmlEncode(
    OpcUa_Double        nValue,
    OpcUa_OutputStream* pOstrm);

/** 
  @brief Reads a Double value.
 
  @param pValue [out] The decoded value.
  @param pIstrm [in]  The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Double_XmlDecode(
    OpcUa_Double*      pValue,
    OpcUa_InputStream* pIstrm);

/** 
  @brief Writes a String value.
 
  @param pValue [in] The value to encode.
  @param pOstrm [in] The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_String_XmlEncode(
    OpcUa_String*       pValue,
    OpcUa_OutputStream* pOstrm);

/** 
  @brief Reads a String value.
 
  @param pValue     [out] The decoded value.
  @param nMaxLength [in]  The maximum length for the decoded string (0 means no limit).
  @param pIstrm     [in]  The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_String_XmlDecode(
    OpcUa_String*      pValue,
    OpcUa_UInt32       nMaxLength,
    OpcUa_InputStream* pIstrm);

/** 
  @brief Writes a DateTime value.
 
  @param pValue [in] The value to encode.
  @param pOstrm [in] The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_DateTime_XmlEncode(
    OpcUa_DateTime*     pValue,
    OpcUa_OutputStream* pOstrm); 

/** 
  @brief Reads a DateTime value.
 
  @param pValue [out] The decoded value.
  @param pIstrm [in]  The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_DateTime_XmlDecode(
    OpcUa_DateTime*    pValue,
    OpcUa_InputStream* pIstrm);

/** 
  @brief Writes a Guid value.
 
  @param pValue [in] The value to encode.
  @param pOstrm [in] The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Guid_XmlEncode(
    OpcUa_Guid*         pValue,
    OpcUa_OutputStream* pOstrm);

/** 
  @brief Reads a Guid value.
 
  @param pValue [out] The decoded value.
  @param pIstrm [in]  The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Guid_XmlDecode(
    OpcUa_Guid*        pValue,
    OpcUa_InputStream* pIstrm);

/** 
  @brief Writes a ByteString value.
 
  @param pValue [in] The value to encode.
  @param pOstrm [in] The stream.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_ByteString_XmlEncode(
    OpcUa_ByteString*   pValue,
    OpcUa_OutputStream* pOstrm);

/** 
  @brief Reads a ByteString value.
 
  @param pValue     [out] The decoded value.
  @param nMaxLength [in]  The maximum length for the decoded byte string (0 means no limit).
  @param pIstrm     [in]  The stream.
*/
OpcUa_StatusCode OpcUa_ByteString_XmlDecode(
    OpcUa_ByteString*  pValue,
    OpcUa_UInt32       nMaxLength,
    OpcUa_InputStream* pIstrm);

OPCUA_END_EXTERN_C

#endif /* _OpcUa_XmlEncoder_H_ */
