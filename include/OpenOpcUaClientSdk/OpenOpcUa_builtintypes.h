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
	 * The NodeId type
	 *===========================================================================*/

	/* The set of known node identifier types */
	typedef enum _OpcUa_IdentifierType
	{
		OpcUa_IdentifierType_Numeric = 0x00,
		OpcUa_IdentifierType_String = 0x01,
		OpcUa_IdentifierType_Guid = 0x02,
		OpcUa_IdentifierType_Opaque = 0x03
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
	OPCUA_BEGIN_EXTERN_C
	OpcUa_Void OpcUa_NodeId_Initialize(OpcUa_NodeId* pValue);
	OpcUa_Void OpcUa_NodeId_Clear(OpcUa_NodeId* pValue);
	OpcUa_StatusCode OpcUa_NodeId_CopyTo(const OpcUa_NodeId* pSource, OpcUa_NodeId* pDestination);
	OpcUa_Int OpcUa_NodeId_Compare(const OpcUa_NodeId* pValue1, const OpcUa_NodeId* pValue2);
	OPCUA_END_EXTERN_C
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

	/*============================================================================
	 * The LocalizedText structure.
	 *===========================================================================*/
	typedef struct _OpcUa_LocalizedText
	{
		OpcUa_String Locale;
		OpcUa_String Text;
	}
	OpcUa_LocalizedText;
	OPCUA_BEGIN_EXTERN_C
	OpcUa_Void OpcUa_LocalizedText_Initialize(OpcUa_LocalizedText* value);
	OpcUa_Void OpcUa_LocalizedText_Clear(OpcUa_LocalizedText* value);
	OpcUa_StatusCode OpcUa_LocalizedText_CopyTo(const OpcUa_LocalizedText* a_pSource, OpcUa_LocalizedText* a_pDestination);
	OPCUA_END_EXTERN_C
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

	typedef OpcUa_ByteString OpcUa_XmlElement;
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

	struct _OpcUa_DataValue;

	/*============================================================================
	 * The Variant type
	 *===========================================================================*/
	struct _OpcUa_Variant;

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
	OPCUA_BEGIN_EXTERN_C
	OpcUa_StatusCode OpcUa_Variant_CopyTo(const OpcUa_Variant* pSource, OpcUa_Variant* pDestination);
	OpcUa_Void OpcUa_Variant_Initialize(OpcUa_Variant* value);
	OpcUa_Void OpcUa_Variant_Clear(OpcUa_Variant* value);
	OPCUA_END_EXTERN_C
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
	OPCUA_BEGIN_EXTERN_C
	OpcUa_Void OpcUa_DataValue_Initialize(OpcUa_DataValue* value);
	OpcUa_Void OpcUa_DataValue_Clear(OpcUa_DataValue* value);
	OpcUa_StatusCode OpcUa_DataValue_CopyTo(const OpcUa_DataValue* pSource, OpcUa_DataValue* pDestination);
	OPCUA_END_EXTERN_C
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
