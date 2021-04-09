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

#ifdef XML_TOK_NS_C

const ENCODING *
NS(XmlGetUtf8InternalEncoding)(void)
{
  return &ns(internal_utf8_encoding).enc;
}

const ENCODING *
NS(XmlGetUtf16InternalEncoding)(void)
{
#if BYTEORDER == 1234
  return &ns(internal_little2_encoding).enc;
#elif BYTEORDER == 4321
  return &ns(internal_big2_encoding).enc;
#else
  const short n = 1;
  return (*(const char *)&n
		  ? &ns(internal_little2_encoding).enc
		  : &ns(internal_big2_encoding).enc);
#endif
}

static const ENCODING * const NS(encodings)[] = {
  &ns(latin1_encoding).enc,
  &ns(ascii_encoding).enc,
  &ns(utf8_encoding).enc,
  &ns(big2_encoding).enc,
  &ns(big2_encoding).enc,
  &ns(little2_encoding).enc,
  &ns(utf8_encoding).enc /* NO_ENC */
};

static int PTRCALL
NS(initScanProlog)(const ENCODING *enc, const char *ptr, const char *end,
				   const char **nextTokPtr)
{
  return initScan(NS(encodings), (const INIT_ENCODING *)enc,
				  XML_PROLOG_STATE, ptr, end, nextTokPtr);
}

static int PTRCALL
NS(initScanContent)(const ENCODING *enc, const char *ptr, const char *end,
					const char **nextTokPtr)
{
  return initScan(NS(encodings), (const INIT_ENCODING *)enc,
				  XML_CONTENT_STATE, ptr, end, nextTokPtr);
}

int
NS(XmlInitEncoding)(INIT_ENCODING *p, const ENCODING **encPtr,
					const char *name)
{
  int i = getEncodingIndex(name);
  if (i == UNKNOWN_ENC)
	return 0;
  SET_INIT_ENC_INDEX(p, i);
  p->initEnc.scanners[XML_PROLOG_STATE] = NS(initScanProlog);
  p->initEnc.scanners[XML_CONTENT_STATE] = NS(initScanContent);
  p->initEnc.updatePosition = initUpdatePosition;
  p->encPtr = encPtr;
  *encPtr = &(p->initEnc);
  return 1;
}

static const ENCODING *
NS(findEncoding)(const ENCODING *enc, const char *ptr, const char *end)
{
#define ENCODING_MAX 128
  char buf[ENCODING_MAX];
  char *p = buf;
  int i;
  XmlUtf8Convert(enc, &ptr, end, &p, p + ENCODING_MAX - 1);
  if (ptr != end)
	return 0;
  *p = 0;
  if (streqci(buf, KW_UTF_16) && enc->minBytesPerChar == 2)
	return enc;
  i = getEncodingIndex(buf);
  if (i == UNKNOWN_ENC)
	return 0;
  return NS(encodings)[i];
}

int
NS(XmlParseXmlDecl)(int isGeneralTextEntity,
					const ENCODING *enc,
					const char *ptr,
					const char *end,
					const char **badPtr,
					const char **versionPtr,
					const char **versionEndPtr,
					const char **encodingName,
					const ENCODING **encoding,
					int *standalone)
{
  return doParseXmlDecl(NS(findEncoding),
						isGeneralTextEntity,
						enc,
						ptr,
						end,
						badPtr,
						versionPtr,
						versionEndPtr,
						encodingName,
						encoding,
						standalone);
}

#endif /* XML_TOK_NS_C */