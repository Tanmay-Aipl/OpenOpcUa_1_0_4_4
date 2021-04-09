/* ========================================================================
 * Copyright (c) 2005-2011 The OPC Foundation, Inc. All rights reserved.
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

#ifndef _OpcUa_BuiltInTypes_H_
#define _OpcUa_BuiltInTypes_H_ 1

OPCUA_BEGIN_EXTERN_C

/*============================================================================
 * The OpcUa_BuiltInType enumeration
 *===========================================================================*/
typedef enum _OpcUa_BuiltInType
{
    OpcUaType_Null = 0,
    OpcUaType_Boolean = 1,
    OpcUaType_SByte = 2,
    OpcUaType_Byte = 3,
    OpcUaType_Int16 = 4,
    OpcUaType_UInt16 = 5,
    OpcUaType_Int32 = 6,
    OpcUaType_UInt32 = 7,
    OpcUaType_Int64 = 8,
    OpcUaType_UInt64 = 9,
    OpcUaType_Float = 10,
    OpcUaType_Double = 11,
    OpcUaType_String = 12,
    OpcUaType_DateTime = 13,
    OpcUaType_Guid = 14,
    OpcUaType_ByteString = 15,
    OpcUaType_XmlElement = 16,
    OpcUaType_NodeId = 17,
    OpcUaType_ExpandedNodeId = 18,
    OpcUaType_StatusCode = 19,
    OpcUaType_QualifiedName = 20,
    OpcUaType_LocalizedText = 21,
    OpcUaType_ExtensionObject = 22,
    OpcUaType_DataValue = 23,
    OpcUaType_Variant = 24,
    OpcUaType_DiagnosticInfo = 25
}
OpcUa_BuiltInType;

/*============================================================================
 * Helper for compare functions.
 *===========================================================================*/
#define OPCUA_EQUAL     ((OpcUa_Int)0)
#define OPCUA_NOT_EQUAL ((OpcUa_Int)!OPCUA_EQUAL)

/*============================================================================
 * Scalar value macros (for Code Generator output)
 *===========================================================================*/
#define OpcUa_Scalar_CopyTo(xSource, xDestination) ((*(xDestination) = *(xSource)) != 0)?(OpcUa_Good):(OpcUa_Good)
#define OpcUa_Scalar_Compare(xValue1, xValue2) ((OpcUa_Int)(*(xValue1) - *(xValue2)))

/*============================================================================
 * The Boolean type
 *===========================================================================*/

#define OpcUa_Boolean_Initialize(xValue) *(xValue) = OpcUa_False;

#define OpcUa_Boolean_Clear(xValue) *(xValue) = OpcUa_False;

#define OpcUa_Boolean_Compare(xValue1, xValue2) OpcUa_Scalar_Compare(xValue1, xValue2)

#define OpcUa_Boolean_CopyTo(xSource,xDestination) *(xDestination)=(OpcUa_Boolean)*(xSource);
/*============================================================================
 * The SByte type
 *===========================================================================*/

#define OpcUa_SByte_Initialize(xValue) *(xValue) = (OpcUa_SByte)0;

#define OpcUa_SByte_Clear(xValue) *(xValue) = (OpcUa_SByte)0;

#define OpcUa_SByte_Compare(xValue1, xValue2) OpcUa_Scalar_Compare(xValue1, xValue2)

#define OpcUa_SByte_CopyTo(xSource,xDestination) *(xDestination)=(OpcUa_SByte)*(xSource);
/*============================================================================
 * The Byte type
 *===========================================================================*/

#define OpcUa_Byte_Initialize(xValue) *(xValue) = (OpcUa_Byte)0;

#define OpcUa_Byte_Clear(xValue) *(xValue) = (OpcUa_Byte)0;

#define OpcUa_Byte_Compare(xValue1, xValue2) OpcUa_Scalar_Compare(xValue1, xValue2)

#define OpcUa_Byte_CopyTo(xSource,xDestination) *(xDestination)=(OpcUa_Byte)*(xSource);
/*============================================================================
 * The Int16 type
 *===========================================================================*/

#define OpcUa_Int16_Initialize(xValue) *(xValue) = (OpcUa_Int16)0;

#define OpcUa_Int16_Clear(xValue) *(xValue) = (OpcUa_Int16)0;

#define OpcUa_Int16_Compare(xValue1, xValue2) OpcUa_Scalar_Compare(xValue1, xValue2)

#define OpcUa_Int16_CopyTo(xSource,xDestination) *(xDestination)=(OpcUa_Int16)*(xSource);
/*============================================================================
 * The UInt16 type
 *===========================================================================*/

#define OpcUa_UInt16_Initialize(xValue) *(xValue) = (OpcUa_UInt16)0;

#define OpcUa_UInt16_Clear(xValue) *(xValue) = (OpcUa_UInt16)0;

#define OpcUa_UInt16_Compare(xValue1, xValue2) OpcUa_Scalar_Compare(xValue1, xValue2)

#define OpcUa_UInt16_CopyTo(xSource,xDestination) *(xDestination)=(OpcUa_UInt16)*(xSource);
/*============================================================================
 * The Int32 type
 *===========================================================================*/

#define OpcUa_Int32_Initialize(xValue) *(xValue) = (OpcUa_Int32)0;

#define OpcUa_Int32_Clear(xValue) *(xValue) = (OpcUa_Int32)0;

#define OpcUa_Int32_Compare(xValue1, xValue2) OpcUa_Scalar_Compare(xValue1, xValue2)

#define OpcUa_Int32_CopyTo(xSource,xDestination) *(xDestination)=(OpcUa_Int32)*(xSource);
/*============================================================================
 * The UInt32 type
 *===========================================================================*/

#define OpcUa_UInt32_Initialize(xValue) *(xValue) = (OpcUa_UInt32)0;

#define OpcUa_UInt32_Clear(xValue) *(xValue) = (OpcUa_UInt32)0;

#define OpcUa_UInt32_Compare(xValue1, xValue2) OpcUa_Scalar_Compare(xValue1, xValue2)

#define OpcUa_UInt32_CopyTo(xSource,xDestination) *(xDestination)=(OpcUa_UInt32)*(xSource);
/*============================================================================
 * The Int64 type
 *===========================================================================*/

#if OPCUA_USE_NATIVE_64BIT_INTEGERS

#define OpcUa_Int64_Initialize(xValue) *(xValue) = (OpcUa_Int64)0;

#define OpcUa_Int64_Clear(xValue) *(xValue) = (OpcUa_Int64)0;

#define OpcUa_Int64_Compare(xValue1, xValue2) OpcUa_Scalar_Compare(xValue1, xValue2)

#else /* OPCUA_USE_NATIVE_64BIT_INTEGERS */

#define OpcUa_Int64_Initialize(xValue) OpcUa_MemSet(xValue, 0, sizeof(OpcUa_Int64));

#define OpcUa_Int64_Clear(xValue) OpcUa_MemSet(xValue, 0, sizeof(OpcUa_Int64));

#define OpcUa_Int64_Compare(xValue1, xValue2) (OpcUa_MemCmp(xValue1, xValue2, sizeof(OpcUa_Int64)))

#define OpcUa_Int64_CopyTo(xSource,xDestination) *(xDestination)=(OpcUa_Int64)*(xSource);
#endif /* OPCUA_USE_NATIVE_64BIT_INTEGERS */

/*============================================================================
 * The UInt64 type
 *===========================================================================*/

#if OPCUA_USE_NATIVE_64BIT_INTEGERS

#define OpcUa_UInt64_Initialize(xValue) *(xValue) = (OpcUa_UInt64)0;

#define OpcUa_UInt64_Clear(xValue) *(xValue) = (OpcUa_UInt64)0;

#define OpcUa_UInt64_Compare(xValue1, xValue2) OpcUa_Scalar_Compare(xValue1, xValue2)

#else /* OPCUA_USE_NATIVE_64BIT_INTEGERS */

#define OpcUa_UInt64_Initialize(xValue) OpcUa_MemSet(xValue, 0, sizeof(OpcUa_UInt64));

#define OpcUa_UInt64_Clear(xValue) OpcUa_MemSet(xValue, 0, sizeof(OpcUa_UInt64));

#define OpcUa_UInt64_Compare(xValue1, xValue2) (OpcUa_MemCmp(xValue1, xValue2, sizeof(OpcUa_UInt64)))

#define OpcUa_UInt64_CopyTo(xSource,xDestination) *(xDestination)=(OpcUa_UInt64)*(xSource);
#endif /* OPCUA_USE_NATIVE_64BIT_INTEGERS */

/*============================================================================
 * The Float type
 *===========================================================================*/

#define OpcUa_Float_Initialize(xValue) *(xValue) = (OpcUa_Float)0.0;

#define OpcUa_Float_Clear(xValue) *(xValue) = (OpcUa_Float)0.0;

#define OpcUa_Float_Compare(xValue1, xValue2) ((OpcUa_Int)((*(xValue1) > *(xValue2))?1:((*(xValue1) < *(xValue2))?-1:0)))

#define OpcUa_Float_CopyTo(xSource,xDestination) *(xDestination)=(OpcUa_Float)*(xSource);
/*============================================================================
 * The Double type
 *===========================================================================*/

#define OpcUa_Double_Initialize(xValue) *(xValue) = (OpcUa_Double)0.0;

#define OpcUa_Double_Clear(xValue) *(xValue) = (OpcUa_Double)0.0;

#define OpcUa_Double_Compare(xValue1, xValue2) ((OpcUa_Int)((*(xValue1) > *(xValue2))?1:((*(xValue1) < *(xValue2))?-1:0)))

#define OpcUa_Double_CopyTo(xSource,xDestination) *(xDestination)=(OpcUa_Double)*(xSource);
/*============================================================================
 * The String type
 *===========================================================================*/
/* see opcua_string.h */

/*============================================================================
 * The DateTime type
 *===========================================================================*/

#define OpcUa_DateTime_Initialize(xValue) OpcUa_MemSet(xValue, 0, sizeof(OpcUa_DateTime));

#define OpcUa_DateTime_Clear(xValue) OpcUa_MemSet(xValue, 0, sizeof(OpcUa_DateTime));

#define OpcUa_DateTime_Compare(xValue1, xValue2) (OpcUa_MemCmp(xValue1, xValue2, sizeof(OpcUa_DateTime)))

#define OpcUa_DateTime_CopyTo(xSource, xDestination) (*(xDestination) = *(xSource))

/*============================================================================
 * The Guid type
 *===========================================================================*/

#define OpcUa_Guid_Initialize(xValue) *(xValue) = OpcUa_Guid_Null;

#define OpcUa_Guid_Clear(xValue) *(xValue) = OpcUa_Guid_Null;

#define OpcUa_Guid_Compare(xValue1, xValue2) (OpcUa_MemCmp(xValue1, xValue2, sizeof(OpcUa_Guid)))

#define OpcUa_Guid_CopyTo(xSource, xDestination) OpcUa_MemCpy(xDestination, sizeof(OpcUa_Guid), (OpcUa_Void*)xSource, sizeof(OpcUa_Guid))

/*============================================================================
 * The ByteString type
 *===========================================================================*/

OPCUA_EXPORT OpcUa_Void OpcUa_ByteString_Initialize(OpcUa_ByteString* value);

OPCUA_EXPORT OpcUa_Void OpcUa_ByteString_Clear(OpcUa_ByteString* value);

OPCUA_EXPORT OpcUa_Int OpcUa_ByteString_Compare(const OpcUa_ByteString* value1, const OpcUa_ByteString* value2);

OPCUA_EXPORT OpcUa_StatusCode OpcUa_ByteString_CopyTo(const OpcUa_ByteString* source, OpcUa_ByteString* destination);

OPCUA_EXPORT OpcUa_StatusCode OpcUa_ByteString_Concatenate(const OpcUa_ByteString* source, OpcUa_ByteString* destination, OpcUa_Int len);

/*============================================================================
 * The XmlElement type
 *===========================================================================*/

typedef OpcUa_ByteString OpcUa_XmlElement;

#define OpcUa_XmlElement_Initialize(xValue) OpcUa_ByteString_Initialize((OpcUa_ByteString*)xValue);

#define OpcUa_XmlElement_Clear(xValue) OpcUa_ByteString_Clear((OpcUa_ByteString*)xValue);

#define OpcUa_XmlElement_Compare(xValue1, xValue2) OpcUa_ByteString_Compare((OpcUa_ByteString*)xValue1, (OpcUa_ByteString*)xValue2)

#define OpcUa_XmlElement_CopyTo(xSource, xDestination) OpcUa_ByteString_CopyTo((OpcUa_ByteString*)xSource, (OpcUa_ByteString*)xDestination)

/*============================================================================
 * The NodeId type
 *===========================================================================*/

/* The set of known node identifier types */
typedef enum _OpcUa_IdentifierType
{
    OpcUa_IdentifierType_Numeric = 0x00,
    OpcUa_IdentifierType_String  = 0x01,
    OpcUa_IdentifierType_Guid    = 0x02,
    OpcUa_IdentifierType_Opaque  = 0x03
}
OpcUa_IdentifierType;

typedef struct _OpcUa_NodeId
{
    OpcUa_UInt16 IdentifierType;
    OpcUa_UInt16 NamespaceIndex;

    union
    {
        OpcUa_UInt32     Numeric;
        OpcUa_String     String;
        OpcUa_Guid*      Guid;
        OpcUa_ByteString ByteString;
    } 
    Identifier;
}
OpcUa_NodeId;

OPCUA_EXPORT OpcUa_Void OpcUa_NodeId_Initialize(OpcUa_NodeId* pValue);

OPCUA_EXPORT OpcUa_Void OpcUa_NodeId_Clear(OpcUa_NodeId* pValue);

OPCUA_EXPORT OpcUa_Int OpcUa_NodeId_Compare(const OpcUa_NodeId* pValue1, const OpcUa_NodeId* pValue2);

OPCUA_EXPORT OpcUa_StatusCode OpcUa_NodeId_CopyTo(const OpcUa_NodeId* pSource, OpcUa_NodeId* pDestination);

OPCUA_EXPORT OpcUa_Boolean OpcUa_NodeId_IsNull(OpcUa_NodeId* pValue);

/*============================================================================
 * The ExpandedNodeId type
 *===========================================================================*/
typedef struct _OpcUa_ExpandedNodeId
{
    OpcUa_NodeId NodeId;
    OpcUa_String NamespaceUri;
    OpcUa_UInt32 ServerIndex;
}
OpcUa_ExpandedNodeId;

OPCUA_EXPORT OpcUa_Void OpcUa_ExpandedNodeId_Initialize(OpcUa_ExpandedNodeId* pValue);

OPCUA_EXPORT OpcUa_Void OpcUa_ExpandedNodeId_Clear(OpcUa_ExpandedNodeId* pValue);

OPCUA_EXPORT OpcUa_Int OpcUa_ExpandedNodeId_Compare(const OpcUa_ExpandedNodeId* pValue1, const OpcUa_ExpandedNodeId* pValue2);

OPCUA_EXPORT OpcUa_StatusCode OpcUa_ExpandedNodeId_CopyTo(const OpcUa_ExpandedNodeId* pSource, OpcUa_ExpandedNodeId* pDestination);

OPCUA_EXPORT OpcUa_Boolean OpcUa_ExpandedNodeId_IsNull(OpcUa_ExpandedNodeId* pValue);

/*============================================================================
 * The StatusCode type
 *===========================================================================*/

#define OpcUa_StatusCode_Initialize(xValue) *(xValue) = (OpcUa_StatusCode)0;

#define OpcUa_StatusCode_Clear(xValue) *(xValue) = (OpcUa_StatusCode)0;

#define OpcUa_StatusCode_Compare(xValue1, xValue2) OpcUa_UInt32_Compare(xValue1, xValue2)

#define OpcUa_StatusCode_CopyTo(xSource, xDestination) OpcUa_UInt32_CopyTo(xSource, xDestination)

/*============================================================================
 * The DiagnosticsInfo type
 *===========================================================================*/
typedef struct _OpcUa_DiagnosticInfo
{
    OpcUa_Int32                   SymbolicId;
    OpcUa_Int32                   NamespaceUri;
    OpcUa_Int32                   Locale;
    OpcUa_Int32                   LocalizedText;
    OpcUa_String                  AdditionalInfo;
    OpcUa_StatusCode              InnerStatusCode;
    struct _OpcUa_DiagnosticInfo* InnerDiagnosticInfo;
}
OpcUa_DiagnosticInfo;

OPCUA_EXPORT OpcUa_Void OpcUa_DiagnosticInfo_Initialize(OpcUa_DiagnosticInfo* value);

OPCUA_EXPORT OpcUa_Void OpcUa_DiagnosticInfo_Clear(OpcUa_DiagnosticInfo* value);

OPCUA_EXPORT OpcUa_Int OpcUa_DiagnosticInfo_Compare(const OpcUa_DiagnosticInfo* pValue1, const OpcUa_DiagnosticInfo* pValue2);

OPCUA_EXPORT OpcUa_StatusCode OpcUa_DiagnosticInfo_CopyTo(const OpcUa_DiagnosticInfo* pSource, OpcUa_DiagnosticInfo* pDestination);

/*============================================================================
 * The LocalizedText structure.
 *===========================================================================*/
typedef struct _OpcUa_LocalizedText
{
    OpcUa_String Locale;
    OpcUa_String Text;
}
OpcUa_LocalizedText;

OPCUA_EXPORT OpcUa_Void OpcUa_LocalizedText_Initialize(OpcUa_LocalizedText* pValue);

OPCUA_EXPORT OpcUa_Void OpcUa_LocalizedText_Clear(OpcUa_LocalizedText* pValue);

OPCUA_EXPORT OpcUa_Int OpcUa_LocalizedText_Compare(const OpcUa_LocalizedText* pValue1, const OpcUa_LocalizedText* pValue2);

OPCUA_EXPORT OpcUa_StatusCode OpcUa_LocalizedText_CopyTo(const OpcUa_LocalizedText* pSource, OpcUa_LocalizedText* pDestination);

/*============================================================================
 * The QualifiedName structure.
 *===========================================================================*/
typedef struct _OpcUa_QualifiedName
{
    OpcUa_UInt16 NamespaceIndex;
    OpcUa_UInt16 Reserved;
    OpcUa_String Name;
}
OpcUa_QualifiedName;

OPCUA_EXPORT OpcUa_Void OpcUa_QualifiedName_Initialize(OpcUa_QualifiedName* pValue);

OPCUA_EXPORT OpcUa_Void OpcUa_QualifiedName_Clear(OpcUa_QualifiedName* pValue);

OPCUA_EXPORT OpcUa_Int OpcUa_QualifiedName_Compare(const OpcUa_QualifiedName* pValue1, const OpcUa_QualifiedName* pValue2);

OPCUA_EXPORT OpcUa_StatusCode OpcUa_QualifiedName_CopyTo(const OpcUa_QualifiedName* pSource, OpcUa_QualifiedName* pDestination);

/*============================================================================
 * String extensions
 *===========================================================================*/
#define OpcUa_String_Compare(xValue1, xValue2) OpcUa_String_StrnCmp(xValue1, xValue2, OPCUA_STRING_LENDONTCARE, OpcUa_False)
#define OpcUa_String_CopyTo(xSource, xDestination) OpcUa_String_StrnCpy(xDestination, xSource, OPCUA_STRING_LENDONTCARE)

/*============================================================================
 * The OpcUa_ExtensionObject type
 *===========================================================================*/
struct _OpcUa_EncodeableType;

typedef enum _OpcUa_ExtensionObjectEncoding
{
    OpcUa_ExtensionObjectEncoding_None = 0,
    OpcUa_ExtensionObjectEncoding_Binary = 1,
    OpcUa_ExtensionObjectEncoding_Xml = 2,
    OpcUa_ExtensionObjectEncoding_EncodeableObject = 3
}
OpcUa_ExtensionObjectEncoding;

typedef struct _OpcUa_ExtensionObject
{
    /*! @brief The full data type identifier. */
    OpcUa_ExpandedNodeId TypeId;
    
    /*! @brief The encoding used for the body. */
    OpcUa_ExtensionObjectEncoding Encoding;

    /*! @brief The body of the extension object. */
    union _OpcUa_ExtensionObject_Body
    {
        /*! @brief A pre-encoded binary body. */
        OpcUa_ByteString Binary;

        /*! @brief A pre-encoded XML body. */
        OpcUa_XmlElement Xml;

        struct _OpcUa_EncodeableObjectBody
        {           
            /*! @brief The object contained in the extension object. */
            OpcUa_Void* Object;
            
            /*! @brief Provides information necessary to encode/decode the object. */
            struct _OpcUa_EncodeableType* Type;
        }
        EncodeableObject;
    }
    Body;

    /*! @brief The length of the encoded body in bytes (updated automatically when GetSize is called). */   
    OpcUa_Int32 BodySize;
}
OpcUa_ExtensionObject;

OPCUA_EXPORT OpcUa_Void OpcUa_ExtensionObject_Create(OpcUa_ExtensionObject** value);

OPCUA_EXPORT OpcUa_Void OpcUa_ExtensionObject_Initialize(OpcUa_ExtensionObject* value);

OPCUA_EXPORT OpcUa_Void OpcUa_ExtensionObject_Clear(OpcUa_ExtensionObject* value);

OPCUA_EXPORT OpcUa_Void OpcUa_ExtensionObject_Delete(OpcUa_ExtensionObject** value);

OPCUA_EXPORT OpcUa_Int OpcUa_ExtensionObject_Compare(const OpcUa_ExtensionObject* pValue1, const OpcUa_ExtensionObject* pValue2);

OPCUA_EXPORT OpcUa_StatusCode OpcUa_ExtensionObject_CopyTo(const OpcUa_ExtensionObject* pSource, OpcUa_ExtensionObject* pDestination);

/*============================================================================
 * The Variant type
 *===========================================================================*/
struct _OpcUa_Variant;
struct _OpcUa_DataValue;

/* A union that contains arrays of one of the built in types. */
typedef union _OpcUa_VariantArrayUnion
{
    OpcUa_Void*              Array;
    OpcUa_Boolean*           BooleanArray;
    OpcUa_SByte*             SByteArray;
    OpcUa_Byte*              ByteArray;
    OpcUa_Int16*             Int16Array;
    OpcUa_UInt16*            UInt16Array;
    OpcUa_Int32*             Int32Array;
    OpcUa_UInt32*            UInt32Array;
    OpcUa_Int64*             Int64Array;
    OpcUa_UInt64*            UInt64Array;
    OpcUa_Float*             FloatArray;
    OpcUa_Double*            DoubleArray;
    OpcUa_String*            StringArray;
    OpcUa_DateTime*          DateTimeArray;
    OpcUa_Guid*              GuidArray;
    OpcUa_ByteString*        ByteStringArray;
    OpcUa_ByteString*        XmlElementArray;
    OpcUa_NodeId*            NodeIdArray;
    OpcUa_ExpandedNodeId*    ExpandedNodeIdArray;
    OpcUa_StatusCode*        StatusCodeArray;
    OpcUa_QualifiedName*     QualifiedNameArray;
    OpcUa_LocalizedText*     LocalizedTextArray;
    OpcUa_ExtensionObject*   ExtensionObjectArray;
    struct _OpcUa_DataValue* DataValueArray;
    struct _OpcUa_Variant*   VariantArray;
}
OpcUa_VariantArrayUnion;

/* A union that contains a one dimensional array of one of the built in types. */
typedef struct _OpcUa_VariantArrayValue
{
    /* The total number of elements in all dimensions. */
    OpcUa_Int32  Length;

    /* The data stored in the array. */
    OpcUa_VariantArrayUnion Value;
}
OpcUa_VariantArrayValue;

/* A union that contains a multi-dimensional array of one of the built in types. */
typedef struct _OpcUa_VariantMatrixValue
{
    /* The number of dimensions in the array. */
    OpcUa_Int32 NoOfDimensions;

    /* The length of each dimension. */
    OpcUa_Int32* Dimensions;

    /* The data stored in the array. 
    
       The higher rank dimensions appear in the array first.
       e.g. a array with dimensions [2,2,2] is written in this order: 
       [0,0,0], [0,0,1], [0,1,0], [0,1,1], [1,0,0], [1,0,1], [1,1,0], [1,1,1]

       Using [3] to access the pointer stored in this field would return element [0,1,1] */
    OpcUa_VariantArrayUnion Value;
}
OpcUa_VariantMatrixValue;

/* Returns the total number of elements stored in a matrix value. */
OPCUA_EXPORT OpcUa_Int32 OpcUa_VariantMatrix_GetElementCount(const OpcUa_VariantMatrixValue* pValue);

/* A union that contains one of the built in types. */
typedef union _OpcUa_VariantUnion
{
    OpcUa_Boolean            Boolean;
    OpcUa_SByte              SByte;
    OpcUa_Byte               Byte;
    OpcUa_Int16              Int16;
    OpcUa_UInt16             UInt16;
    OpcUa_Int32              Int32;
    OpcUa_UInt32             UInt32;
    OpcUa_Int64              Int64;
    OpcUa_UInt64             UInt64;
    OpcUa_Float              Float;
    OpcUa_Double             Double;
    OpcUa_DateTime           DateTime;
    OpcUa_String             String;
    OpcUa_Guid*              Guid;
    OpcUa_ByteString         ByteString;
    OpcUa_XmlElement         XmlElement;
    OpcUa_NodeId*            NodeId;
    OpcUa_ExpandedNodeId*    ExpandedNodeId;
    OpcUa_StatusCode         StatusCode;
    OpcUa_QualifiedName*     QualifiedName;
    OpcUa_LocalizedText*     LocalizedText;
    OpcUa_ExtensionObject*   ExtensionObject;
    struct _OpcUa_DataValue* DataValue;
    OpcUa_VariantArrayValue  Array;
#if !OPCUA_VARIANT_OMIT_MATRIX
    OpcUa_VariantMatrixValue Matrix;
#endif /* !OPCUA_VARIANT_OMIT_MATRIX */
}
OpcUa_VariantUnion;

#define OpcUa_VariantArrayType_Scalar 0x00
#define OpcUa_VariantArrayType_Array  0x01
#define OpcUa_VariantArrayType_Matrix 0x02

typedef struct _OpcUa_Variant
{
    /* Indicates the datatype stored in the Variant. This is always one of the OpcUa_BuiltInType values. */
    /* This is the datatype of a single element if the Variant contains an array. */
    OpcUa_Byte          Datatype;

    /* A flag indicating that an array with one or more dimensions is stored in the Variant. */
    OpcUa_Byte          ArrayType;

    /* Not used. Must be ignored. */
    OpcUa_UInt16        Reserved;

    /* The value stored in the Variant. */
    OpcUa_VariantUnion  Value;
}
OpcUa_Variant;

OPCUA_EXPORT OpcUa_Void OpcUa_Variant_Initialize(OpcUa_Variant* value);

OPCUA_EXPORT OpcUa_Void OpcUa_Variant_Clear(OpcUa_Variant* value);

OPCUA_EXPORT OpcUa_Int OpcUa_Variant_Compare(const OpcUa_Variant* pValue1, const OpcUa_Variant* pValue2);

OPCUA_EXPORT OpcUa_StatusCode OpcUa_Variant_CopyTo(const OpcUa_Variant* pSource, OpcUa_Variant* pDestination);

#define OpcUa_Variant_InitializeArray(xValue, xLength) OpcUa_MemSet(xValue, 0, (xLength)*sizeof(OpcUa_Variant));

#define OpcUa_Variant_ClearArray(xValue, xLength) OpcUa_ClearArray(xValue, xLength, OpcUa_Variant);

/*============================================================================
 * The DataValue type
 *===========================================================================*/
typedef struct _OpcUa_DataValue
{
    OpcUa_Variant    Value;
    OpcUa_StatusCode StatusCode;
    OpcUa_DateTime   SourceTimestamp;
    OpcUa_DateTime   ServerTimestamp;
#if !OPCUA_DATAVALUE_OMIT_PICOSECONDS
    OpcUa_UInt16     SourcePicoseconds;
    OpcUa_UInt16     ServerPicoseconds;
#endif /* !OPCUA_DATAVALUE_OMIT_PICOSECONDS */
}
OpcUa_DataValue;

OPCUA_EXPORT OpcUa_Void OpcUa_DataValue_Initialize(OpcUa_DataValue* value);

OPCUA_EXPORT OpcUa_Void OpcUa_DataValue_Clear(OpcUa_DataValue* value);

OPCUA_EXPORT OpcUa_Int OpcUa_DataValue_Compare(const OpcUa_DataValue* pValue1, const OpcUa_DataValue* pValue2);

OPCUA_EXPORT OpcUa_StatusCode OpcUa_DataValue_CopyTo(const OpcUa_DataValue* pSource, OpcUa_DataValue* pDestination);

#define OpcUa_DataValue_InitializeArray(xValue, xLength) OpcUa_MemSet(xValue, 0, (xLength)*sizeof(OpcUa_DataValue));

#define OpcUa_DataValue_ClearArray(xValue, xLength) OpcUa_ClearArray(xValue, xLength, OpcUa_DataValue);

/*============================================================================
 * OpcUa_Field_Initialize
 *===========================================================================*/
#define OpcUa_Field_Initialize(xType, xName) OpcUa_##xType##_Initialize(&a_pValue->xName);

/*============================================================================
 * OpcUa_Field_InitializeEncodeable
 *===========================================================================*/
#define OpcUa_Field_InitializeEncodeable(xType, xName) xType##_Initialize(&a_pValue->xName);

/*============================================================================
 * OpcUa_Field_InitializeEnumerated
 *===========================================================================*/
#define OpcUa_Field_InitializeEnumerated(xType, xName) xType##_Initialize(&a_pValue->xName);

/*============================================================================
 * OpcUa_Field_InitializeArray
 *===========================================================================*/
#define OpcUa_Field_InitializeArray(xType, xName) \
{ \
    a_pValue->xName = OpcUa_Null; \
    a_pValue->NoOf##xName = 0; \
}

/*============================================================================
 * OpcUa_Field_InitializeEncodeableArray
 *===========================================================================*/
#define OpcUa_Field_InitializeEncodeableArray(xType, xName) OpcUa_Field_InitializeArray(xType, xName)

/*============================================================================
 * OpcUa_Field_InitializeEnumeratedArray
 *===========================================================================*/
#define OpcUa_Field_InitializeEnumeratedArray(xType, xName) OpcUa_Field_InitializeArray(xType, xName)

/*============================================================================
 * OpcUa_Field_Clear
 *===========================================================================*/
#define OpcUa_Field_Clear(xType, xName) OpcUa_##xType##_Clear(&a_pValue->xName);

/*============================================================================
 * OpcUa_Field_ClearEncodeable
 *===========================================================================*/
#define OpcUa_Field_ClearEncodeable(xType, xName) xType##_Clear(&a_pValue->xName);

/*============================================================================
 * OpcUa_Field_ClearEnumerated
 *===========================================================================*/
#define OpcUa_Field_ClearEnumerated(xType, xName) xType##_Clear(&a_pValue->xName);

/*============================================================================
 * OpcUa_Field_ClearArray
 *===========================================================================*/
#define OpcUa_Field_ClearArray(xType, xName)\
{ \
    int ii; \
\
    for (ii = 0; ii < a_pValue->NoOf##xName && a_pValue->xName != OpcUa_Null; ii++) \
    { \
        OpcUa_##xType##_Clear(&(a_pValue->xName[ii])); \
    } \
\
    OpcUa_Free(a_pValue->xName); \
\
    a_pValue->xName = OpcUa_Null; \
    a_pValue->NoOf##xName = 0; \
}

/*============================================================================
 * OpcUa_Field_ClearEncodeableArray
 *===========================================================================*/
#define OpcUa_Field_ClearEncodeableArray(xType, xName) \
{ \
    int ii; \
\
    for (ii = 0; ii < a_pValue->NoOf##xName && a_pValue->xName != OpcUa_Null; ii++) \
    { \
        xType##_Clear(&(a_pValue->xName[ii])); \
    } \
\
    OpcUa_Free(a_pValue->xName); \
\
    a_pValue->xName = OpcUa_Null; \
    a_pValue->NoOf##xName = 0; \
}

/*============================================================================
 * OpcUa_Field_ClearEnumeratedArray
 *===========================================================================*/
#define OpcUa_Field_ClearEnumeratedArray(xType, xName) \
{ \
    OpcUa_Free(a_pValue->xName); \
    a_pValue->xName = OpcUa_Null; \
    a_pValue->NoOf##xName = 0; \
}

/*============================================================================
 * OpcUa_Field_Compare
 *===========================================================================*/
#define OpcUa_Field_Compare(xType, xName) {OpcUa_Int i = OpcUa_##xType##_Compare(&a_pValue1->xName, &a_pValue2->xName); if(i) return i;}

/*============================================================================
 * OpcUa_Field_CompareEncodeable
 *===========================================================================*/
#define OpcUa_Field_CompareEncodeable(xType, xName) {OpcUa_Int i = xType##_Compare(&a_pValue1->xName, &a_pValue2->xName); if(i) return i;}

/*============================================================================
 * OpcUa_Field_CompareEnumerated
 *===========================================================================*/
#define OpcUa_Field_CompareEnumerated(xType, xName) {OpcUa_Int i = OpcUa_Int32_Compare(&a_pValue1->xName, &a_pValue2->xName); if(i) return i;}

/*============================================================================
 * OpcUa_Field_CompareArray
 *===========================================================================*/
#define OpcUa_Field_CompareArray(xType, xName)\
{ \
    int ii; \
\
    if(a_pValue1->NoOf##xName != a_pValue2->NoOf##xName) \
    { \
        return 1; \
    } \
\
    for (ii = 0; ii < a_pValue1->NoOf##xName && a_pValue1->xName != OpcUa_Null; ii++) \
    { \
        if(OpcUa_##xType##_Compare(&(a_pValue1->xName[ii]), &(a_pValue2->xName[ii]))){return 1;}; \
    } \
}

/*============================================================================
 * OpcUa_Field_CompareEncodeableArray
 *===========================================================================*/
#define OpcUa_Field_CompareEncodeableArray(xType, xName) \
{ \
    int ii; \
\
    for (ii = 0; ii < a_pValue1->NoOf##xName && a_pValue1->xName != OpcUa_Null; ii++) \
    { \
        if(xType##_Compare(&(a_pValue1->xName[ii]), &(a_pValue2->xName[ii]))){return 1;}; \
    } \
}

/*============================================================================
 * OpcUa_Field_CompareEnumeratedArray
 *===========================================================================*/
#define OpcUa_Field_CompareEnumeratedArray(xType, xName) \
{ \
    int ii; \
\
    for (ii = 0; ii < a_pValue1->NoOf##xName && a_pValue1->xName != OpcUa_Null; ii++) \
    { \
        if(a_pValue1->xName[ii] != a_pValue2->xName[ii]){return 1;}; \
    } \
}

/*============================================================================
 * OpcUa_Field_Copy. This one Alloc, initialize and copy
 *===========================================================================*/
#define OpcUa_Field_Copy(xType, xName) \
{ \
    a_pDestination->xName = (OpcUa_##xType*)OpcUa_Alloc(sizeof(OpcUa_##xType)); \
    OpcUa_GotoErrorIfAllocFailed(a_pDestination->xName); \
    OpcUa_##xType##_Initialize(a_pDestination->xName); \
    uStatus = OpcUa_##xType##_CopyTo(a_pSource->xName, a_pDestination->xName); \
    OpcUa_GotoErrorIfBad(uStatus); \
}

/*============================================================================
 * OpcUa_Field_CopyTo
 *===========================================================================*/
#define OpcUa_Field_CopyTo(xType, xName) uStatus = OpcUa_##xType##_CopyTo(&(a_pSource->xName), &(a_pDestination->xName)); OpcUa_GotoErrorIfBad(uStatus);

/*============================================================================
 * OpcUa_Field_CopyToScalar
 *===========================================================================*/
#define OpcUa_Field_CopyToScalar(xType, xName) (a_pDestination->xName = a_pSource->xName)

/*============================================================================
 * OpcUa_Field_CopyToEncodeable
 *===========================================================================*/
#define OpcUa_Field_CopyToEncodeable(xType, xName) xType##_CopyTo(&a_pSource->xName, &a_pDestination->xName)

/*============================================================================
 * OpcUa_Field_CopyToEnumerated
 *===========================================================================*/
#define OpcUa_Field_CopyToEnumerated(xType, xName) a_pDestination->xName = a_pSource->xName

/*============================================================================
 * OpcUa_Field_CopyToArray
 *===========================================================================*/
#define OpcUa_Field_CopyToArray(xType, xName)\
{ \
    int ii; \
\
    for (ii = 0; ii < a_pSource->NoOf##xName && a_pSource->xName != OpcUa_Null; ii++) \
    { \
        uStatus = OpcUa_##xType##_CopyTo(&(a_pSource->xName[ii]), &(a_pDestination->xName[ii])); \
        OpcUa_GotoErrorIfBad(uStatus); \
    } \
}

/*============================================================================
 * OpcUa_Field_CopyArray
 *===========================================================================*/
#define OpcUa_Field_CopyArray(xType, xName)\
if(a_pSource->NoOf##xName > 0 && a_pSource->xName != OpcUa_Null) \
{ \
    int ii; \
\
    a_pDestination->xName = (OpcUa_##xType*)OpcUa_Alloc(sizeof(OpcUa_##xType) * a_pSource->NoOf##xName); \
    OpcUa_GotoErrorIfAllocFailed(a_pDestination->xName); \
    OpcUa_MemSet(a_pDestination->xName, 0, sizeof(OpcUa_##xType) * a_pSource->NoOf##xName); \
\
    for (ii = 0; ii < a_pSource->NoOf##xName && a_pSource->xName != OpcUa_Null; ii++) \
    { \
        uStatus = OpcUa_##xType##_CopyTo(&(a_pSource->xName[ii]), &(a_pDestination->xName[ii])); \
        OpcUa_GotoErrorIfBad(uStatus); \
    } \
    a_pDestination->NoOf##xName = a_pSource->NoOf##xName; \
} \
else \
{ \
    a_pDestination->NoOf##xName = 0; \
    a_pDestination->xName = OpcUa_Null; \
}

/*============================================================================
 * OpcUa_Field_CopyToEncodeableArray
 *===========================================================================*/
#define OpcUa_Field_CopyToEncodeableArray(xType, xName) \
{ \
    int ii; \
\
    for (ii = 0; ii < a_pSource->NoOf##xName && a_pSource->xName != OpcUa_Null; ii++) \
    { \
        xType##_CopyTo(&(a_pSource->xName[ii]), &(a_pDestination->xName[ii])); \
    } \
}

/*============================================================================
 * OpcUa_Field_CopyEncodeableArray
 *===========================================================================*/
#define OpcUa_Field_CopyEncodeableArray(xType, xName) \
if(a_pSource->NoOf##xName > 0 && a_pSource->xName != OpcUa_Null) \
{ \
    int ii; \
\
    a_pDestination->xName = (xType*)OpcUa_Alloc(sizeof(xType) * a_pSource->NoOf##xName); \
    OpcUa_GotoErrorIfAllocFailed(a_pDestination->xName); \
    OpcUa_MemSet(a_pDestination->xName, 0, sizeof(xType) * a_pSource->NoOf##xName); \
\
    for (ii = 0; ii < a_pSource->NoOf##xName && a_pSource->xName != OpcUa_Null; ii++) \
    { \
        xType##_CopyTo(&(a_pSource->xName[ii]), &(a_pDestination->xName[ii])); \
    } \
    a_pDestination->NoOf##xName = a_pSource->NoOf##xName; \
} \
else \
{ \
    a_pDestination->NoOf##xName = 0; \
    a_pDestination->xName = OpcUa_Null; \
}

/*============================================================================
 * OpcUa_Field_CopyEnumeratedArray
 *===========================================================================*/
#define OpcUa_Field_CopyEnumeratedArray(xType, xName) \
if(a_pSource->NoOf##xName > 0 && a_pSource->xName != OpcUa_Null) \
{ \
    a_pDestination->xName = (xType*)OpcUa_Alloc(a_pSource->NoOf##xName * sizeof(xType)); \
    OpcUa_GotoErrorIfAllocFailed(a_pDestination->xName); \
    OpcUa_MemCpy(a_pDestination->xName, a_pSource->NoOf##xName * sizeof(xType), a_pSource->xName, a_pSource->NoOf##xName * sizeof(xType)); \
    a_pDestination->NoOf##xName = a_pSource->NoOf##xName; \
} \
else \
{ \
    a_pDestination->NoOf##xName = 0; \
    a_pDestination->xName = OpcUa_Null; \
}

/*============================================================================
 * OpcUa_Field_CopyScalarArray
 *===========================================================================*/
#define OpcUa_Field_CopyScalarArray(xType, xName) \
if(a_pSource->NoOf##xName > 0 && a_pSource->xName != OpcUa_Null) \
{ \
    a_pDestination->xName = (OpcUa_##xType*)OpcUa_Alloc(a_pSource->NoOf##xName * sizeof(OpcUa_##xType)); \
    OpcUa_GotoErrorIfAllocFailed(a_pDestination->xName); \
    OpcUa_MemCpy(a_pDestination->xName, a_pSource->NoOf##xName * sizeof(OpcUa_##xType), a_pSource->xName, a_pSource->NoOf##xName * sizeof(OpcUa_##xType)); \
    a_pDestination->NoOf##xName = a_pSource->NoOf##xName; \
} \
else \
{ \
    a_pDestination->NoOf##xName = 0; \
    a_pDestination->xName = OpcUa_Null; \
}

/*============================================================================
 * Flags that can be set for the EventNotifier attribute.
 *===========================================================================*/
  
/* The Object or View produces no event and has no event history. */
#define OpcUa_EventNotifiers_None 0x0

/* The Object or View produces event notifications. */
#define OpcUa_EventNotifiers_SubscribeToEvents 0x1

/* The Object has an event history which may be read. */
#define OpcUa_EventNotifiers_HistoryRead 0x4

/* The Object has an event history which may be updated. */
#define OpcUa_EventNotifiers_HistoryWrite 0x8

/*============================================================================
 * Flags that can be set for the AccessLevel attribute.
 *===========================================================================*/

/* The Variable value cannot be accessed and has no event history. */
#define OpcUa_AccessLevels_None 0x0

/* The current value of the Variable may be read.*/
#define OpcUa_AccessLevels_CurrentRead 0x1

/* The current value of the Variable may be written.*/
#define OpcUa_AccessLevels_CurrentWrite 0x2

/* The current value of the Variable may be read or written.*/
#define OpcUa_AccessLevels_CurrentReadOrWrite 0x3

/* The history for the Variable may be read.*/
#define OpcUa_AccessLevels_HistoryRead 0x4

/* The history for the Variable may be updated.*/
#define OpcUa_AccessLevels_HistoryWrite 0x8

/* The history value of the Variable may be read or updated. */
#define OpcUa_AccessLevels_HistoryReadOrWrite 0xC

/*============================================================================
 * Constants defined for the ValueRank attribute.
 *===========================================================================*/

/* The variable may be a scalar or a one dimensional array. */
#define OpcUa_ValueRanks_ScalarOrOneDimension -3
        
/* The variable may be a scalar or an array of any dimension. */
#define OpcUa_ValueRanks_Any -2

/* The variable is always a scalar. */
#define OpcUa_ValueRanks_Scalar -1

/* The variable is always an array with one or more dimensions. */
#define OpcUa_ValueRanks_OneOrMoreDimensions 0

/* The variable is always one dimensional array. */
#define OpcUa_ValueRanks_OneDimension 1

/* The variable is always an array with two or more dimensions. */
#define OpcUa_ValueRanks_TwoDimensions 2

/*============================================================================
 * Constants defined for the MinimumSamplingInterval attribute.
 *===========================================================================*/

/* The server does not know how fast the value can be sampled. */
#define OpcUa_MinimumSamplingIntervals_Indeterminate -1

/* The server can sample the variable continuously. */
#define OpcUa_MinimumSamplingIntervals_Continuous 0

/*============================================================================
 * Constants defined for the DiagnosticsMasks parameter.
 *===========================================================================*/

#define OpcUa_DiagnosticsMasks_ServiceSymbolicId 1
#define OpcUa_DiagnosticsMasks_ServiceLocalizedText 2
#define OpcUa_DiagnosticsMasks_ServiceAdditionalInfo 4
#define OpcUa_DiagnosticsMasks_ServiceInnerStatusCode 8
#define OpcUa_DiagnosticsMasks_ServiceInnerDiagnostics 16
#define OpcUa_DiagnosticsMasks_ServiceSymbolicIdAndText 3
#define OpcUa_DiagnosticsMasks_ServiceNoInnerStatus 15
#define OpcUa_DiagnosticsMasks_ServiceAll 31
#define OpcUa_DiagnosticsMasks_OperationSymbolicId 32
#define OpcUa_DiagnosticsMasks_OperationLocalizedText 64
#define OpcUa_DiagnosticsMasks_OperationAdditionalInfo 128
#define OpcUa_DiagnosticsMasks_OperationInnerStatusCode 256
#define OpcUa_DiagnosticsMasks_OperationInnerDiagnostics 512
#define OpcUa_DiagnosticsMasks_OperationSymbolicIdAndText 96
#define OpcUa_DiagnosticsMasks_OperationNoInnerStatus 224
#define OpcUa_DiagnosticsMasks_OperationAll 992
#define OpcUa_DiagnosticsMasks_SymbolicId 33
#define OpcUa_DiagnosticsMasks_LocalizedText 66
#define OpcUa_DiagnosticsMasks_AdditionalInfo 132
#define OpcUa_DiagnosticsMasks_InnerStatusCode 264
#define OpcUa_DiagnosticsMasks_InnerDiagnostics 528
#define OpcUa_DiagnosticsMasks_SymbolicIdAndText 99
#define OpcUa_DiagnosticsMasks_NoInnerStatus 239
#define OpcUa_DiagnosticsMasks_All 1023

OPCUA_END_EXTERN_C

#endif /* _OpcUa_BuiltInTypes_H_ */
