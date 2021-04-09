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

/*
DESCRIPTION

Defines and function prototypes for the XML parser Canonicalization module
*/

#ifndef __INCxmlCanonh
#define __INCxmlCanonh

#ifdef __cplusplus
extern "C" {
#endif

/*includes */

#include "expat.h"
#include "xmlop.h"

/* defines */

#undef CANON_DEBUG

#ifdef CANON_DEBUG
#define XML_CANON_DEBUG(msg, x1, x2, x3, x4) \
		fprintf(stderr, msg, __FILE__, __LINE__, x1, x2, x3, x4)
#else
#define XML_CANON_DEBUG(msg, x1, x2, x3, x4) 
#endif

#define NSSEP T('\001')

#define XMLCANON_DOCTYPEBUF_SIZE 1024
#define XMLCANON_NOTATIONLIST_SIZE 100
#define XMLCANON_NOTATIONNAME_SIZE 50

/* function declarartions */
void xmlCanonParamInit();

extern void xmlCanonFile(const char *canonizeDocName, const char *outputDocName);

#ifdef __cplusplus
}
#endif

#endif /* __INCxmlCanonh */

