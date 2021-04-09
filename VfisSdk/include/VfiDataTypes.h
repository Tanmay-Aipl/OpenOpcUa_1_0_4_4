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
#pragma once
namespace UABuiltinType
{	
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
	* Type definitions for basic data types.
	*===========================================================================*/
	typedef int                 OpcUa_Int;
	typedef unsigned int        OpcUa_UInt;
	typedef void                OpcUa_Void;
	typedef void*               OpcUa_Handle;
	typedef unsigned char       OpcUa_Boolean;
	typedef char                OpcUa_SByte;
	typedef unsigned char       OpcUa_Byte;
	typedef short               OpcUa_Int16;
	typedef unsigned short      OpcUa_UInt16;
	typedef long                OpcUa_Int32;
	typedef unsigned long       OpcUa_UInt32;
	typedef float               OpcUa_Float;
	typedef double              OpcUa_Double;
	typedef char                OpcUa_CharA;
	typedef unsigned char       OpcUa_UCharA;
	typedef OpcUa_CharA*        OpcUa_StringA;
	typedef unsigned short      OpcUa_Char;


	typedef OpcUa_UInt32    OpcUa_StatusCode;

	typedef struct _OpcUa_ByteString
	{
		OpcUa_Int32 Length;
		OpcUa_Byte* Data;
	} OpcUa_ByteString;

	typedef OpcUa_ByteString OpcUa_XmlElement;
	// 64 bits type
	#ifdef OPCUA_P_NATIVE64
	/**
	 * @brief Convert OpcUa_DateTime to OpcUa_Int64. (i64 = OpcUa_DateTime_ToInt64(xDT))
	 */
	  #define OpcUa_DateTime_ToInt64(xDT)     (*((OpcUa_Int64*)&xDT))

	/**
	 * @brief Convert OpcUa_Int64 to OpcUa_DateTime. (DT = OpcUa_DateTime_FromInt64(x64))
	 */
	  #define OpcUa_DateTime_FromInt64(x64)   *((OpcUa_DateTime*)&x64)

	  #ifdef _MSC_VER
		typedef __int64             OpcUa_Int64;
		typedef unsigned __int64    OpcUa_UInt64;
	  #else /* _MSC_VER */
		typedef long long           OpcUa_Int64;
		typedef unsigned long long  OpcUa_UInt64;
	  #endif /* _MSC_VER */
	#else
	struct _OpcUa_Int64 {
		OpcUa_UInt32 dwLowQuad;
		OpcUa_UInt32 dwHighQuad;
	};
	typedef struct _OpcUa_Int64 OpcUa_Int64;
	struct _OpcUa_UInt64 {
		OpcUa_UInt32 dwLowQuad;
		OpcUa_UInt32 dwHighQuad;
	};
	typedef struct _OpcUa_UInt64 OpcUa_UInt64;
	#endif

	struct _OpcUa_DateTime
	{
		OpcUa_UInt32 dwLowDateTime;
		OpcUa_UInt32 dwHighDateTime;
	};
	typedef struct _OpcUa_DateTime OpcUa_DateTime;

	//#ifdef _DEBUG
	typedef struct _OpcUa_String
	{
		OpcUa_UInt16 flags;
		OpcUa_UInt32 uLength;
		OpcUa_CharA* strContent;
	} OpcUa_String, *OpcUa_pString;
	////#else
	//typedef struct _OpcUa_String
	//{
	//	OpcUa_UInt16        uReserved1;     /* Content is private to String Implementation */
	//	OpcUa_UInt32        uReserved2;     /* Content is private to String Implementation */
	//	OpcUa_Void*         uReserved4;     /* Content is private to String Implementation */
	//} OpcUa_String, *OpcUa_pString;
	//#endif
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

	/*============================================================================
	 * The LocalizedText structure.
	 *===========================================================================*/
	typedef struct _OpcUa_LocalizedText
	{
		OpcUa_String Locale;
		OpcUa_String Text;
	}
	OpcUa_LocalizedText;

	typedef struct _OpcUa_Guid
	{
		OpcUa_UInt32    Data1;
		OpcUa_UInt16    Data2;
		OpcUa_UInt16    Data3;
		OpcUa_UCharA    Data4[8];
	} OpcUa_Guid;
	/*============================================================================
	 * The NodeId type
	 *===========================================================================*/
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
	} OpcUa_NodeId;
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
		OpcUa_VariantMatrixValue Matrix;
	}
	OpcUa_VariantUnion;
	//
	#define OpcUa_VariantArrayType_Scalar 0x00
	#define OpcUa_VariantArrayType_Array  0x01
	#define OpcUa_VariantArrayType_Matrix 0x02


	//
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


	/*============================================================================
	 * The DataValue type
	 *===========================================================================*/
	typedef struct _OpcUa_DataValue
	{
		OpcUa_Variant    Value;
		OpcUa_StatusCode StatusCode;
		OpcUa_DateTime   SourceTimestamp;
		OpcUa_DateTime   ServerTimestamp;
		OpcUa_UInt16     SourcePicoseconds;
		OpcUa_UInt16     ServerPicoseconds;
	}
	OpcUa_DataValue;
	typedef enum _OpcUa_IdentifierType
	{
		OpcUa_IdentifierType_Numeric = 0x00,
		OpcUa_IdentifierType_String  = 0x01,
		OpcUa_IdentifierType_Guid    = 0x02,
		OpcUa_IdentifierType_Opaque  = 0x03
	}
	OpcUa_IdentifierType;
	// other types
	#ifdef __cplusplus
	#define OpcUa_Null           0
	#else
	#define OpcUa_Null          (OpcUa_Void*)0
	#endif
	#define OpcUa_False         0
	#define OpcUa_True          (!OpcUa_False)
}