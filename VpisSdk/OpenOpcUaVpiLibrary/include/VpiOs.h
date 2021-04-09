/* INCLUDE OS SPECIFIC DEFINITIONS HERE */

#ifndef OS_SPECIFIC_INCLUDED
#define OS_SPECIFIC_INCLUDED

#if defined(WIN32) || defined(_WIN32_WCE)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <Wincrypt.h>
typedef int socklen_t;
#endif

#ifdef _GNUC_
#define VPI_IMPORT 
#define NO_ERROR 0L     
/* PUT HERE SOME SYSTEM INCLUDE */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */
#include <sys/socket.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <dirent.h>
#include <wchar.h>
#include <wctype.h>
#include <fnmatch.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <semaphore.h>
#include <VpiPlatformdefs.h>
#include <sys/select.h>

#define stricmp strcasecmp
#define WIN32_FIND_DATA struct dirent*
#define WIN32_FIND_DATAA struct dirent*

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define ERROR_NO_MORE_FILES 18
#define closesocket(s) close(s)
#define SOCKET int
#define HRESULT OpcUa_Int32
#define WSAEWOULDBLOCK EWOULDBLOCK
#define WSAEINPROGRESS EINPROGRESS
#define WSAECONNABORTED ECONNABORTED
#define WSAECONNRESET ECONNRESET
#define MAX_PATH FILENAME_MAX
#define INVALID_HANDLE_VALUE -1
#define WAIT_TIMEOUT 258
#define WAIT_OBJECT_0 0
#define ERROR_TOO_MANY_POSTS 298

#define S_OK 0
#define S_FALSE 1
#define E_FAIL 0x80004005L

#define FALSE 0
#define TRUE 1
#define INFINITE 0xFFFFFFFF
#define E_INVALIDARG 0x80070057

#define EPOCH_DIFF 11644473600LL

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;
typedef struct addrinfo * LPADDRINFO;
typedef const char *LPCSTR;
typedef char *LPSTR;
typedef wchar_t* LPWSTR;
typedef unsigned char       BYTE;
typedef char CHAR;
typedef wchar_t     _TCHAR;

#ifdef UNICODE
 typedef LPCWSTR LPCTSTR;
#else
 typedef LPCSTR LPCTSTR;
#endif

#define PATH_PATERN '/'
/* PUT HERE SOME SPECIFIC DECLARATIONS FOR LINUX OS */
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef int BOOL;
typedef unsigned long long UINT64;
typedef int HANDLE;
typedef ushort USHORT;
typedef unsigned char UCHAR;
typedef uint UINT;

typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

#define ULONGLONG unsigned long long
typedef union _ULARGE_INTEGER {
  struct {
    DWORD LowPart;
    DWORD HighPart;
  } ;
  struct {
    DWORD LowPart;
    DWORD HighPart;
  } u;
  ULONGLONG QuadPart;
} ULARGE_INTEGER, *PULARGE_INTEGER;

/* Well-known GUIDs that are defined by COM... */


typedef void *HCERTSTORE;

#define ZeroMemory(arg0,arg1) memset(arg0,0,arg1);

/*************************************************************************************/
/* Time Functions */
#define LONG long
#define LONGLONG long long
#define Int32x32To64(a, b) ((LONGLONG)((LONG)(a)) * (LONGLONG)((LONG)(b)))

VPI_BEGIN_EXTERN_C

VPILIBRARY_EXPORT DWORD GetTickCount(void);

void GetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime);

BOOL FileTimeToSystemTime(const FILETIME *lpFileTime,LPSYSTEMTIME lpSystemTime);

void GetLocalTime(LPSYSTEMTIME lpSystemTime);

BOOL SystemTimeToFileTime(const LPSYSTEMTIME lpSystemTime, FILETIME *lpFileTime);

VPI_END_EXTERN_C

/* we don't use the linux internal generator */
#define _GUID_CREATE_NOT_AVAILABLE
/* force definition for PTHREAD compatibility */

#ifndef __cdecl
#define __cdecl
#endif

#ifndef __stdcall
#define __stdcall
#endif

#endif

#endif
