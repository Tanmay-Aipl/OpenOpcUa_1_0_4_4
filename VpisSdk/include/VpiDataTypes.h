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
namespace VpiBuiltinType
{	
	/*============================================================================
	 * The Vpi_BuiltInType enumeration
	 *===========================================================================*/
	typedef enum _Vpi_BuiltInType
	{
		VpiType_Null = 0,
		VpiType_Boolean = 1,
		VpiType_SByte = 2,
		VpiType_Byte = 3,
		VpiType_Int16 = 4,
		VpiType_UInt16 = 5,
		VpiType_Int32 = 6,
		VpiType_UInt32 = 7,
		VpiType_Int64 = 8,
		VpiType_UInt64 = 9,
		VpiType_Float = 10,
		VpiType_Double = 11,
		VpiType_String = 12,
		VpiType_DateTime = 13,
		VpiType_Guid = 14,
		VpiType_ByteString = 15,
		VpiType_XmlElement = 16,
		VpiType_NodeId = 17,
		VpiType_ExpandedNodeId = 18,
		VpiType_StatusCode = 19,
		VpiType_QualifiedName = 20,
		VpiType_LocalizedText = 21,
		VpiType_ExtensionObject = 22,
		VpiType_DataValue = 23,
		VpiType_Variant = 24,
		VpiType_DiagnosticInfo = 25
	}
	Vpi_BuiltInType;
	/*============================================================================
	* Type definitions for basic data types.
	*===========================================================================*/
	typedef int                 Vpi_Int;
	typedef unsigned int        Vpi_UInt;
	typedef void                Vpi_Void;
	typedef void*               Vpi_Handle;
	typedef Vpi_Void*			Vpi_Mutex;
	typedef Vpi_Void*			Vpi_Semaphore;
	typedef unsigned char       Vpi_Boolean;
	typedef char                Vpi_SByte;
	typedef unsigned char       Vpi_Byte;
	typedef short               Vpi_Int16;
	typedef unsigned short      Vpi_UInt16;
	typedef long                Vpi_Int32;
	typedef unsigned long       Vpi_UInt32;
	typedef float               Vpi_Float;
	typedef double              Vpi_Double;
	typedef char                Vpi_CharA;
	typedef unsigned char       Vpi_UCharA;
	typedef Vpi_CharA*			Vpi_StringA;
	typedef unsigned short      Vpi_CharW;
	typedef unsigned short      Vpi_Char;


	typedef Vpi_UInt32    Vpi_StatusCode;

	typedef struct _Vpi_ByteString
	{
		Vpi_Int32 Length;
		Vpi_Byte* Data;
	} Vpi_ByteString;

	typedef Vpi_ByteString Vpi_XmlElement;
	// 64 bits type
	#ifdef OPCUA_P_NATIVE64
	/**
	 * @brief Convert Vpi_DateTime to Vpi_Int64. (i64 = Vpi_DateTime_ToInt64(xDT))
	 */
	  #define Vpi_DateTime_ToInt64(xDT)     (*((Vpi_Int64*)&xDT))

	/**
	 * @brief Convert Vpi_Int64 to Vpi_DateTime. (DT = Vpi_DateTime_FromInt64(x64))
	 */
	  #define Vpi_DateTime_FromInt64(x64)   *((Vpi_DateTime*)&x64)

	  #ifdef _MSC_VER
		typedef __int64             Vpi_Int64;
		typedef unsigned __int64    Vpi_UInt64;
	  #else /* _MSC_VER */
		typedef long long           Vpi_Int64;
		typedef unsigned long long  Vpi_UInt64;
	  #endif /* _MSC_VER */
	#else
	struct _Vpi_Int64 {
		Vpi_UInt32 dwLowQuad;
		Vpi_UInt32 dwHighQuad;
	};
	typedef struct _Vpi_Int64 Vpi_Int64;
	struct _Vpi_UInt64 {
		Vpi_UInt32 dwLowQuad;
		Vpi_UInt32 dwHighQuad;
	};
	typedef struct _Vpi_UInt64 Vpi_UInt64;
	#endif

	struct _Vpi_DateTime
	{
		Vpi_UInt32 dwLowDateTime;
		Vpi_UInt32 dwHighDateTime;
	};
	typedef struct _Vpi_DateTime Vpi_DateTime;

	//#ifdef _DEBUG
	typedef struct _Vpi_String
	{
		Vpi_UInt16 flags;
		Vpi_UInt32 uLength;
		Vpi_CharA* strContent;
	} Vpi_String, *Vpi_pString;
	////#else
	//typedef struct _Vpi_String
	//{
	//	Vpi_UInt16        uReserved1;     /* Content is private to String Implementation */
	//	Vpi_UInt32        uReserved2;     /* Content is private to String Implementation */
	//	Vpi_Void*         uReserved4;     /* Content is private to String Implementation */
	//} Vpi_String, *Vpi_pString;
	//#endif
	/*============================================================================
	 * The QualifiedName structure.
	 *===========================================================================*/
	typedef struct _Vpi_QualifiedName
	{
		Vpi_UInt16 NamespaceIndex;
		Vpi_UInt16 Reserved;
		Vpi_String Name;
	}
	Vpi_QualifiedName;

	/*============================================================================
	 * The LocalizedText structure.
	 *===========================================================================*/
	typedef struct _Vpi_LocalizedText
	{
		Vpi_String Locale;
		Vpi_String Text;
	}
	Vpi_LocalizedText;

	typedef struct _Vpi_Guid
	{
		Vpi_UInt32    Data1;
		Vpi_UInt16    Data2;
		Vpi_UInt16    Data3;
		Vpi_UCharA    Data4[8];
	} Vpi_Guid;
	/*============================================================================
	 * The NodeId type
	 *===========================================================================*/
	typedef struct _Vpi_NodeId
	{
		Vpi_UInt16 IdentifierType;
		Vpi_UInt16 NamespaceIndex;

		union
		{
			Vpi_UInt32     Numeric;
			Vpi_String     String;
			Vpi_Guid*      Guid;
			Vpi_ByteString ByteString;
		} 
		Identifier;
	} Vpi_NodeId;
	/*============================================================================
	 * The ExpandedNodeId type
	 *===========================================================================*/
	typedef struct _Vpi_ExpandedNodeId
	{
		Vpi_NodeId NodeId;
		Vpi_String NamespaceUri;
		Vpi_UInt32 ServerIndex;
	}
	Vpi_ExpandedNodeId;

	typedef enum _Vpi_ExtensionObjectEncoding
	{
		Vpi_ExtensionObjectEncoding_None = 0,
		Vpi_ExtensionObjectEncoding_Binary = 1,
		Vpi_ExtensionObjectEncoding_Xml = 2,
		Vpi_ExtensionObjectEncoding_EncodeableObject = 3
	}
	Vpi_ExtensionObjectEncoding;

	typedef struct _Vpi_ExtensionObject
	{
		/*! @brief The full data type identifier. */
		Vpi_ExpandedNodeId TypeId;
		
		/*! @brief The encoding used for the body. */
		Vpi_ExtensionObjectEncoding Encoding;

		/*! @brief The body of the extension object. */
		union _Vpi_ExtensionObject_Body
		{
			/*! @brief A pre-encoded binary body. */
			Vpi_ByteString Binary;

			/*! @brief A pre-encoded XML body. */
			Vpi_XmlElement Xml;

			struct _Vpi_EncodeableObjectBody
			{           
				/*! @brief The object contained in the extension object. */
				Vpi_Void* Object;
				
				/*! @brief Provides information necessary to encode/decode the object. */
				struct _Vpi_EncodeableType* Type;
			}
			EncodeableObject;
		}
		Body;

		/*! @brief The length of the encoded body in bytes (updated automatically when GetSize is called). */   
		Vpi_Int32 BodySize;
	}
	Vpi_ExtensionObject;

	/* A union that contains arrays of one of the built in types. */
	typedef union _Vpi_VariantArrayUnion
	{
		Vpi_Void*              Array;
		Vpi_Boolean*           BooleanArray;
		Vpi_SByte*             SByteArray;
		Vpi_Byte*              ByteArray;
		Vpi_Int16*             Int16Array;
		Vpi_UInt16*            UInt16Array;
		Vpi_Int32*             Int32Array;
		Vpi_UInt32*            UInt32Array;
		Vpi_Int64*             Int64Array;
		Vpi_UInt64*            UInt64Array;
		Vpi_Float*             FloatArray;
		Vpi_Double*            DoubleArray;
		Vpi_String*            StringArray;
		Vpi_DateTime*          DateTimeArray;
		Vpi_Guid*              GuidArray;
		Vpi_ByteString*        ByteStringArray;
		Vpi_ByteString*        XmlElementArray;
		Vpi_NodeId*            NodeIdArray;
		Vpi_ExpandedNodeId*    ExpandedNodeIdArray;
		Vpi_StatusCode*        StatusCodeArray;
		Vpi_QualifiedName*     QualifiedNameArray;
		Vpi_LocalizedText*     LocalizedTextArray;
		Vpi_ExtensionObject*   ExtensionObjectArray;
		struct _Vpi_DataValue* DataValueArray;
		struct _Vpi_Variant*   VariantArray;
	}
	Vpi_VariantArrayUnion;

	/* A union that contains a one dimensional array of one of the built in types. */
	typedef struct _Vpi_VariantArrayValue
	{
		/* The total number of elements in all dimensions. */
		Vpi_Int32  Length;

		/* The data stored in the array. */
		Vpi_VariantArrayUnion Value;
	}
	Vpi_VariantArrayValue;

	/* A union that contains a multi-dimensional array of one of the built in types. */
	typedef struct _Vpi_VariantMatrixValue
	{
		/* The number of dimensions in the array. */
		Vpi_Int32 NoOfDimensions;

		/* The length of each dimension. */
		Vpi_Int32* Dimensions;

		/* The data stored in the array. 
		
		   The higher rank dimensions appear in the array first.
		   e.g. a array with dimensions [2,2,2] is written in this order: 
		   [0,0,0], [0,0,1], [0,1,0], [0,1,1], [1,0,0], [1,0,1], [1,1,0], [1,1,1]

		   Using [3] to access the pointer stored in this field would return element [0,1,1] */
		Vpi_VariantArrayUnion Value;
	}
	Vpi_VariantMatrixValue;

	/* A union that contains one of the built in types. */
	typedef union _Vpi_VariantUnion
	{
		Vpi_Boolean            Boolean;
		Vpi_SByte              SByte;
		Vpi_Byte               Byte;
		Vpi_Int16              Int16;
		Vpi_UInt16             UInt16;
		Vpi_Int32              Int32;
		Vpi_UInt32             UInt32;
		Vpi_Int64              Int64;
		Vpi_UInt64             UInt64;
		Vpi_Float              Float;
		Vpi_Double             Double;
		Vpi_DateTime           DateTime;
		Vpi_String             String;
		Vpi_Guid*              Guid;
		Vpi_ByteString         ByteString;
		Vpi_XmlElement         XmlElement;
		Vpi_NodeId*            NodeId;
		Vpi_ExpandedNodeId*    ExpandedNodeId;
		Vpi_StatusCode         StatusCode;
		Vpi_QualifiedName*     QualifiedName;
		Vpi_LocalizedText*     LocalizedText;
		Vpi_ExtensionObject*   ExtensionObject;
		struct _Vpi_DataValue* DataValue;
		Vpi_VariantArrayValue  Array;
		Vpi_VariantMatrixValue Matrix;
	}
	Vpi_VariantUnion;
	//
	#define Vpi_VariantArrayType_Scalar 0x00
	#define Vpi_VariantArrayType_Array  0x01
	#define Vpi_VariantArrayType_Matrix 0x02


	//
	typedef struct _Vpi_Variant
	{
		/* Indicates the datatype stored in the Variant. This is always one of the Vpi_BuiltInType values. */
		/* This is the datatype of a single element if the Variant contains an array. */
		Vpi_Byte          Datatype;

		/* A flag indicating that an array with one or more dimensions is stored in the Variant. */
		Vpi_Byte          ArrayType;

		/* Not used. Must be ignored. */
		Vpi_UInt16        Reserved;

		/* The value stored in the Variant. */
		Vpi_VariantUnion  Value;
	}
	Vpi_Variant;


	/*============================================================================
	 * The DataValue type
	 *===========================================================================*/
	typedef struct _Vpi_DataValue
	{
		Vpi_Variant    Value;
		Vpi_StatusCode StatusCode;
		Vpi_DateTime   SourceTimestamp;
		Vpi_DateTime   ServerTimestamp;
		Vpi_UInt16     SourcePicoseconds;
		Vpi_UInt16     ServerPicoseconds;
	}
	Vpi_DataValue;
	typedef enum _Vpi_IdentifierType
	{
		Vpi_IdentifierType_Numeric = 0x00,
		Vpi_IdentifierType_String  = 0x01,
		Vpi_IdentifierType_Guid    = 0x02,
		Vpi_IdentifierType_Opaque  = 0x03
	}
	Vpi_IdentifierType;
	// other types
	#ifdef __cplusplus
	#define Vpi_Null           0
	#else
	#define Vpi_Null          (Vpi_Void*)0
	#endif
	#define Vpi_False         0
	#define Vpi_True          (!Vpi_False)
}