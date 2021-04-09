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
#include <opcua_p_os.h>
/* UA platform definitions */
#include <opcua_p_internal.h>

#ifdef OPCUA_HAVE_XMLAPI

/* Libxml2 headers */
#include <libxml/parser.h>

/* own header */
#include <opcua_p_libxml2.h>

/*============================================================================
 * OpcUa_P_Libxml2_Initialize
 *===========================================================================*/
OpcUa_Void OpcUa_P_Libxml2_Initialize()
{
    LIBXML_TEST_VERSION
}

/*============================================================================
 * OpcUa_P_Libxml2_Cleanup
 *===========================================================================*/
OpcUa_Void OpcUa_P_Libxml2_Cleanup()
{
    xmlCleanupParser();
}
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_Libxml2_XmlReader_Create(
    OpcUa_Void*                         a_pReadContext,
    OpcUa_XmlReader_PfnReadCallback*    a_pReadCallback,
    OpcUa_XmlReader_PfnCloseCallback*   a_pCloseCallback,
    struct _OpcUa_XmlReader*            a_pXmlReader)
{
	(void)a_pReadContext;
	(void)a_pReadCallback;
	(void)a_pCloseCallback;
	(void)a_pXmlReader;
	return OpcUa_Good;
}
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_Libxml2_XmlReader_Delete(struct _OpcUa_XmlReader*  a_pXmlReader)
{
	(void)a_pXmlReader;
	return OpcUa_Good;
}
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_Libxml2_XmlWriter_Create(
    OpcUa_Void*                         a_pWriteContext,
    OpcUa_XmlWriter_PfnWriteCallback*   a_pWriteCallback,
    OpcUa_XmlWriter_PfnCloseCallback*   a_pCloseCallback,
    struct _OpcUa_XmlWriter*            a_pXmlWriter)
{
	(void)a_pWriteCallback;
	(void)a_pCloseCallback;
	(void)a_pXmlWriter;
	(void)a_pWriteContext;
	return OpcUa_Good;
}
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_Libxml2_XmlWriter_Delete(
    struct _OpcUa_XmlWriter*            a_pXmlWriter)
{
	(void)a_pXmlWriter;
	return OpcUa_Good;
}

#endif /* OPCUA_HAVE_XMLAPI */