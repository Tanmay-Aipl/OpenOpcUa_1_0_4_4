#ifndef VPISTRING
#define VPISTRING
#define OPCUA_STRINGLENZEROTERMINATED   0xffffffffL
#define OPCUA_STRING_LENDONTCARE        OPCUA_STRINGLENZEROTERMINATED
#define OpcUa_uiMagic       0

/////////////////////////////////////////////////////////////////////////////
typedef struct _OpcUa_StringInternal
{
    OpcUa_UInt          uMagic          : 8;    /* ==0x00 -> Ua String, != 0x00 -> C-String, an empty string result in a null pointer! */
    OpcUa_UInt          bFreeSecondMem  : 1;    /* strContent is a Pointer to the string */
    OpcUa_UInt          uReserved       : 7;
    OpcUa_UInt32        uLength;                /* Length without terminating '\0' */
    OpcUa_CharA*        strContent;             /* Pointer or start of the string or 4 first unsignend chars (mind bFreeSecondMem) */
} OpcUa_StringInternal, *OpcUa_pStringInternal;
/////////////////////////////////////////////////////////////////////////////

#define OpcUa_Vpi_String_CopyTo(xSource, xDestination) OpcUa_Vpi_String_StrnCpy(xDestination, xSource, OPCUA_STRING_LENDONTCARE)

OpcUa_Vpi_StatusCode OPCUA_DLLCALL OpcUa_Vpi_String_Initialize( /*  bi */ OpcUa_String* pString);

OpcUa_Vpi_StatusCode OPCUA_DLLCALL OpcUa_Vpi_String_Clear(OpcUa_String* a_pString);

OpcUa_Vpi_StatusCode OPCUA_DLLCALL OpcUa_Vpi_String_StrnCpy(  OpcUa_String*       pDestString, 
                                        const OpcUa_String* pSrcString, 
                                        OpcUa_UInt32        uLength);

OpcUa_Vpi_StatusCode   OPCUA_DLLCALL  OpcUa_Vpi_String_AttachToString(  /*  in */ OpcUa_StringA strSource,
                                                  /*  in */ OpcUa_UInt32  uLength,
                                                  /*  in */ OpcUa_UInt32  uBufferSize,
                                                  /*  in */ OpcUa_Boolean bDoCopy,
                                                  /*  in */ OpcUa_Boolean bFreeOnClear,
                                                  /*  bi */ OpcUa_String* pString);

OpcUa_Vpi_StatusCode OPCUA_DLLCALL OpcUa_Vpi_String_AttachCopy(OpcUa_String* a_pDst, const OpcUa_CharA* a_pSrc);


OpcUa_UInt32 OPCUA_DLLCALL OpcUa_Vpi_String_StrLen(const OpcUa_String* pString);


OpcUa_UInt32 OPCUA_DLLCALL OpcUa_Vpi_String_StrSize(const OpcUa_String* pString);


OpcUa_CharA* OPCUA_DLLCALL OpcUa_Vpi_String_GetRawString(const OpcUa_String* pString);

OpcUa_Boolean OPCUA_DLLCALL OpcUa_Vpi_String_IsNull(const OpcUa_String* pString);

////////////////////////////////////////////////////////////////
// Internal function
//#define _Vpi_String_GetRawString(x)   ( (((OpcUa_StringA)x)[0]=='\0')?(OpcUa_StringA)(((OpcUa_pStringInternal)x)->strContent):((OpcUa_StringA)x))
//OpcUa_Boolean _Vpi_IsUaString(const OpcUa_Void* a_strCString);


#endif