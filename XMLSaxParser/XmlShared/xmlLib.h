/*****************************************************************************
      Author
        ?. Michel Condemine, 4CE Industry (2010-2012)
      
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

/*
modification history
--------------------
01b,20dec04,pas  added 1.95.7, 1.95.8 error codes
01a, 01dec02,dt  written.

*/

#ifndef __INCxmlLibh
#define __INCxmlLibh

#ifdef __cplusplus
extern "C" {
#endif




/* module number for XML */
#define M_xmlLib (850 << 16)  


/* status codes */
/*!!! NOTE: This MUST be kept in sync with enum XML_Error in expat.h !!!*/
#define S_xmlLib_NONE 					(M_xmlLib | 1)
#define S_xmlLib_NO_MEMORY 				(M_xmlLib | 2)
#define S_xmlLib_SYNTAX 				(M_xmlLib | 3)
#define S_xmlLib_NO_ELEMENTS 				(M_xmlLib | 4)
#define S_xmlLib_INVALID_TOKEN 				(M_xmlLib | 5)
#define S_xmlLib_UNCLOSED_TOKEN 			(M_xmlLib | 6)
#define S_xmlLib_PARTIAL_CHAR 				(M_xmlLib | 7)
#define S_xmlLib_TAG_MISMATCH 				(M_xmlLib | 8)
#define S_xmlLib_DUPLICATE_ATTRIBUTE 			(M_xmlLib | 9)
#define S_xmlLib_JUNK_AFTER_DOC_ELEMENT 		(M_xmlLib | 10)
#define S_xmlLib_PARAM_ENTITY_REF 			(M_xmlLib | 11)
#define S_xmlLib_UNDEFINED_ENTITY 			(M_xmlLib | 12)
#define S_xmlLib_RECURSIVE_ENTITY_REF 			(M_xmlLib | 13)
#define S_xmlLib_ASYNC_ENTITY 				(M_xmlLib | 14)
#define S_xmlLib_BAD_CHAR_REF				(M_xmlLib | 15)
#define S_xmlLib_BINARY_ENTITY_REF 			(M_xmlLib | 16)
#define S_xmlLib_ATTRIBUTE_EXTERNAL_ENTITY_REF 		(M_xmlLib | 17)
#define S_xmlLib_MISPLACED_XML_PI 			(M_xmlLib | 18)
#define S_xmlLib_UNKNOWN_ENCODING 			(M_xmlLib | 19)
#define S_xmlLib_INCORRECT_ENCODING 			(M_xmlLib | 20)
#define S_xmlLib_UNCLOSED_CDATA_SECTION 		(M_xmlLib | 21)
#define S_xmlLib_EXTERNAL_ENTITY_HANDLING	 	(M_xmlLib | 22)
#define S_xmlLib_NOT_STANDALONE 			(M_xmlLib | 23)
#define S_xmlLib_UNEXPECTED_STATE 			(M_xmlLib | 24)
#define S_xmlLib_ENTITY_DECLARED_IN_PE 			(M_xmlLib | 25)
#define S_xmlLib_FEATURE_REQUIRES_XML_DTD 		(M_xmlLib | 26)
#define S_xmlLib_CANT_CHANGE_FEATURE_ONCE_PARSING 	(M_xmlLib | 27)
/* Added in 1.95.7. */
#define S_xmlLib_UNBOUND_PREFIX				(M_xmlLib | 28)
/* Added in 1.95.8. */
#define S_xmlLib_UNDECLARING_PREFIX			(M_xmlLib | 29)
#define S_xmlLib_INCOMPLETE_PE				(M_xmlLib | 30)
#define S_xmlLib_XML_DECL				(M_xmlLib | 31)
#define S_xmlLib_TEXT_DECL				(M_xmlLib | 32)
#define S_xmlLib_PUBLICID				(M_xmlLib | 33)
#define S_xmlLib_SUSPENDED				(M_xmlLib | 34)
#define S_xmlLib_NOT_SUSPENDED				(M_xmlLib | 35)
#define S_xmlLib_ABORTED				(M_xmlLib | 36)
#define S_xmlLib_FINISHED				(M_xmlLib | 37)
#define S_xmlLib_SUSPEND_PE				(M_xmlLib | 38)
/* Added by Wind River */
#define S_xmlLib_ILLEGAL_PI_TARGET			(M_xmlLib | 39)

/* used for one pointer validation in xmltok_impl.c */
#define S_xmlLib_ILLEGAL_MEMORY_ACCESS			EFAULT



#ifdef __cplusplus
}
#endif

#endif /* __INCxmlLibh */
