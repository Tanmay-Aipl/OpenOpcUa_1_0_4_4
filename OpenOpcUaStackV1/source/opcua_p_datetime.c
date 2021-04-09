/* ========================================================================
 * Copyright (c) 2005-2009 The OPC Foundation, Inc. All rights reserved.
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

/******************************************************************************************************/
/* Platform Portability Layer                                                                         */
/* Modify the content of this file according to the event implementation on your system.              */
/* This is the win32 implementation                                                                   */
/******************************************************************************************************/

#ifdef _MSC_VER
/* Disables warning for non secure functions in visual studio 2005. Debug only! */
#pragma warning (disable:4996)
#pragma warning(disable:4748) /* suppress /GS can not protect parameters and local variables from local buffer overrun because optimizations are disabled in function */
#endif /* _MSC_VER */

/* System Headers */
#include <opcua_p_os.h>
#include <time.h>

/* UA platform definitions */
#include <opcua_p_internal.h>

/* additional UA dependencies */
/* reference to stack; string is needed for conversion */
#include <opcua_string.h>

/* own headers */
#include <opcua_datetime.h>
#include <opcua_p_datetime.h>

#ifdef __GNUC__
#include <sys/time.h>
#endif

/*============================================================================
 * The OpcUa_GetTimeOfDay function (returns the time in OpcUa_TimeVal format)
 *===========================================================================*/
OpcUa_Void OPCUA_DLLCALL OpcUa_P_DateTime_GetTimeOfDay(OpcUa_TimeVal* a_pTimeVal)
{
#if 0

    static const OpcUa_Int64    SECS_BETWEEN_EPOCHS = 11644473600;
    static const OpcUa_Int64    SECS_TO_100NS       = 10000000; /* 10^7 */

    OpcUa_DateTime              nDateTime           = OpcUa_P_DateTime_UtcNow();
    OpcUa_Int64                 nSecondSince1970    = 0
    nSecondSince1970         = (nDateTime / SECS_TO_100NS) - SECS_BETWEEN_EPOCHS;

    a_pTimeVal->uintSeconds      = (OpcUa_UInt32)nSecondSince1970;
    a_pTimeVal->uintMicroSeconds = (OpcUa_UInt32)((nDateTime%SECS_TO_100NS)/10);

#else

    SYSTEMTIME    SystemTime;
    time_t        TimeType;  /* may be int64 */
    struct tm     structTM;

    GetLocalTime(&SystemTime);

    structTM.tm_sec   = SystemTime.wSecond;
    structTM.tm_min   = SystemTime.wMinute;
    structTM.tm_hour  = SystemTime.wHour;
    structTM.tm_mday  = SystemTime.wDay;
    structTM.tm_mon   = SystemTime.wMonth - 1;
    structTM.tm_year  = SystemTime.wYear - 1900;
    structTM.tm_isdst = -1;

#ifdef _WIN32_WCE
    TimeType = OpcUa_P_Mktime(&structTM);
#else
    TimeType = mktime(&structTM);
#endif

    a_pTimeVal->uintSeconds = (OpcUa_UInt32)TimeType;
    a_pTimeVal->uintMicroSeconds = SystemTime.wMilliseconds * 1000;
#endif
}

/*============================================================================
 * The OpcUa_UtcNow function (returns the time in OpcUa_DateTime format)
 *===========================================================================*/
OpcUa_DateTime OPCUA_DLLCALL OpcUa_P_DateTime_UtcNow()
{
	FILETIME ftTime;

#if 0

	OpcUa_Int64 llBuffer = 0;
    OpcUa_Int64 llTime = 0;

    GetSystemTimeAsFileTime(&ftTime);

    llBuffer = (LONGLONG)ftTime.dwHighDateTime;

    if (llBuffer < 0)
    {
        llBuffer += (((LONGLONG)OpcUa_Int32_Max)+1);
    }

    llTime = (llBuffer<<32);

    llBuffer = (LONGLONG)ftTime.dwLowDateTime;

    if (llBuffer < 0)
    {
        llBuffer += (((LONGLONG)OpcUa_Int32_Max)+1);
    }

    llTime += llBuffer;

    return (OpcUa_DateTime)llTime;
#else

    OpcUa_DateTime tmpDateTime;

    GetSystemTimeAsFileTime(&ftTime);

    tmpDateTime.dwHighDateTime = (OpcUa_UInt32)ftTime.dwHighDateTime;
    tmpDateTime.dwLowDateTime  = (OpcUa_UInt32)ftTime.dwLowDateTime;

    return tmpDateTime;


#endif
}

/*============================================================================
 * Convert DateTime into String
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_DateTime_GetStringFromDateTime(  OpcUa_DateTime a_DateTime,
																		OpcUa_StringA  a_pBuffer,
																		OpcUa_UInt32   a_uLength)
{
    OpcUa_Int           apiResult    = 0;
    const char*         formatString = "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ";
    FILETIME            FileTime;
    SYSTEMTIME          SystemTime;

#if 0
    FileTime.dwLowDateTime  = (DWORD)(a_DateTime & 0x00000000FFFFFFFF);
    FileTime.dwHighDateTime = (DWORD)((a_DateTime & 0xFFFFFFFF00000000)>>32);
#else
    FileTime.dwLowDateTime  = (DWORD)(a_DateTime.dwLowDateTime);
    FileTime.dwHighDateTime = (DWORD)(a_DateTime.dwHighDateTime);
#endif

    FileTimeToSystemTime(&FileTime, &SystemTime);

#if OPCUA_USE_SAFE_FUNCTIONS
#ifdef WIN32
    apiResult = OpcUa_SPrintfA(a_pBuffer, a_uLength, formatString, SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds);
#endif
    
#ifdef _GNUC_
    apiResult = OpcUa_SPrintfA(a_pBuffer, formatString, SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds); 
#endif
#else /* OPCUA_USE_SAFE_FUNCTIONS */
    OpcUa_ReferenceParameter(a_uLength);
    if (a_uLength < 25)
    {
        return OpcUa_BadInvalidArgument;
    }
    apiResult = OpcUa_SPrintfA(a_pBuffer, formatString, SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds);
#endif /* OPCUA_USE_SAFE_FUNCTIONS */

    if (apiResult < 20)
    {
        return OpcUa_Bad;
    }

    return OpcUa_Good;
}

/*============================================================================
 * Convert String into DateTime
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_DateTime_GetDateTimeFromString(  OpcUa_StringA   a_pchDateTimeString,
																		OpcUa_DateTime* a_pDateTime)
{
    BOOL        bResult         = FALSE;
    BOOL        milliSet        = FALSE;
    BOOL        leapYear        = FALSE;
    FILETIME    FileTime;
    SYSTEMTIME  SystemTime;
    size_t      stringLength;
    size_t      maxStringLength;
    char        years[5]    = "YYYY";
    char        months[3]   = "MM";
    char        days[3]     = "DD";
    char        hours[3]    = "HH";
    char        minutes[3]  = "MM";
    char        seconds[3]  = "SS";
    char        millis[4]   = "000";
    char        timeZone[4] = "000";
    int         zoneValue   = 0;
    int         signPosition;
    int         tmpVar;
    int         dayChange   = 0;
    int         monthChange = 0;

    /***************************************************************
     *  ToDo:
     *  Timezone can have values from -12 to +14
     *  At the moment only timezones from -12 to +12 are expected
     *  timezones can also have minutes
     *  at the moment minutes are ignored
     ***************************************************************/

    if (    (a_pchDateTimeString  == OpcUa_Null)
         || (a_pDateTime          == OpcUa_Null))
    {
        return OpcUa_BadInvalidArgument;
    }

    /* ToDo: set max stringlength we accept */
    maxStringLength = 50;

    stringLength = strlen(a_pchDateTimeString);

    /*  check length of string -> there can be any number of digits behind ms */
    /*  we'll ignore anything beyond 3 */
    if ( (stringLength < 20 ) || (stringLength > maxStringLength) )
    {
        return OpcUa_BadSyntaxError;
    }

    /* simple syntax check */
    /* ToDo: we might add some syntax checks here */
    if ( (a_pchDateTimeString[4] == '-')
      && (a_pchDateTimeString[7] == '-')
      && ((a_pchDateTimeString[10] == 'T') || (a_pchDateTimeString[10] == 't'))
      && (a_pchDateTimeString[13] == ':')
      && (a_pchDateTimeString[16] == ':') )
    {
        /* copy strings */
#ifdef WIN32
#if OPCUA_USE_SAFE_FUNCTIONS
        strncpy_s(years, 5, a_pchDateTimeString, 4);
        strncpy_s(months, 3, a_pchDateTimeString+5, 2);
        strncpy_s(days, 3, a_pchDateTimeString+8, 2);
        strncpy_s(hours, 3, a_pchDateTimeString+11, 2);
        strncpy_s(minutes, 3, a_pchDateTimeString+14, 2);
        strncpy_s(seconds, 3, a_pchDateTimeString+17, 2);
#else /* OPCUA_USE_SAFE_FUNCTIONS */
        strncpy(years, a_pchDateTimeString, 5);
        strncpy(months, a_pchDateTimeString+5, 2);
        strncpy(days, a_pchDateTimeString+8, 2);
        strncpy(hours, a_pchDateTimeString+11, 2);
        strncpy(minutes, a_pchDateTimeString+14, 2);
        strncpy(seconds, a_pchDateTimeString+17, 2);
#endif /* OPCUA_USE_SAFE_FUNCTIONS */
#endif

#ifdef _GNUC_
        strncpy(years, a_pchDateTimeString, 5);
        strncpy(months, a_pchDateTimeString+5, 2);
        strncpy(days, a_pchDateTimeString+8, 2);
        strncpy(hours, a_pchDateTimeString+11, 2);
        strncpy(minutes, a_pchDateTimeString+14, 2);
        strncpy(seconds, a_pchDateTimeString+17, 2);
#endif
        /* initialize */
        OpcUa_MemSet(&SystemTime, 0, sizeof(SYSTEMTIME));

        /* parse date and time */
        SystemTime.wYear = (WORD)strtol(years, 0, 10);
        if ( (SystemTime.wYear < 1601) || (SystemTime.wYear > 30827) )
        {
            return OpcUa_BadOutOfRange;
        }
        SystemTime.wMonth = (WORD)strtol(months, 0, 10);
        if ( (SystemTime.wMonth == 0) || (SystemTime.wMonth > 12) )
        {
            return OpcUa_BadOutOfRange;
        }
        SystemTime.wDay = (WORD)strtol(days, 0, 10);
        if ( (SystemTime.wDay == 0) || (SystemTime.wDay > 31) )
        {
            return OpcUa_BadOutOfRange;
        }
        SystemTime.wHour = (WORD)strtol(hours, 0, 10);
        if (SystemTime.wHour > 24)
        {
            return OpcUa_BadOutOfRange;
        }
        SystemTime.wMinute = (WORD)strtol(minutes, 0, 10);
        if (SystemTime.wMinute > 60)
        {
            return OpcUa_BadOutOfRange;
        }
        SystemTime.wSecond = (WORD)strtol(seconds, 0, 10);
        if (SystemTime.wSecond > 60)
        {
            return OpcUa_BadOutOfRange;
        }

        signPosition = 19;

        /* check if ms are set */
        if (a_pchDateTimeString[signPosition] == '.')
        {
            milliSet = TRUE;
        }

        /* find sign for timezone or Z (we accept 'z' and 'Z' here) */
        while ( (a_pchDateTimeString[signPosition] != '\0') && (a_pchDateTimeString[signPosition] != '+') && (a_pchDateTimeString[signPosition] != '-') && (a_pchDateTimeString[signPosition] != 'Z') && (a_pchDateTimeString[signPosition] != 'z') )
        {
            ++signPosition;
        }

        if ( (a_pchDateTimeString[signPosition] == 'z') || (a_pchDateTimeString[signPosition] == 'Z') )
        {
            /* utc time */
            if (milliSet)
            {
                /* be careful we can have more or less than 3 digits of milliseconds */
                tmpVar = signPosition - 20;
                if (tmpVar > 3)
                {
                    tmpVar = 3;
                }

#ifdef WIN32
#if OPCUA_USE_SAFE_FUNCTIONS
                strncpy_s(millis, 4, a_pchDateTimeString+20, tmpVar);
#else /* OPCUA_USE_SAFE_FUNCTIONS */
                strncpy(millis, a_pchDateTimeString+20, tmpVar);
#endif /* OPCUA_USE_SAFE_FUNCTIONS */
#endif

#ifdef _GNUC_
                strncpy(millis, a_pchDateTimeString+20, tmpVar);
#endif

                SystemTime.wMilliseconds =  (WORD)strtol(millis, 0, 10);
            }
        }
        else if ( (a_pchDateTimeString[signPosition] == '+') || (a_pchDateTimeString[signPosition] == '-') )
        {
            /* copy timezone */
#ifdef WIN32
#if OPCUA_USE_SAFE_FUNCTIONS
            strncpy_s(timeZone, 4, a_pchDateTimeString+signPosition, 3);
#else /* OPCUA_USE_SAFE_FUNCTIONS */
            strncpy(timeZone, a_pchDateTimeString+signPosition, 3);
#endif /* OPCUA_USE_SAFE_FUNCTIONS */
#endif

#ifdef _GNUC_
            strncpy(timeZone, a_pchDateTimeString+signPosition, 3);
#endif
            /* strtol will take care of the sign */
            zoneValue = strtol(timeZone, 0, 10);

            if (zoneValue < -12 || zoneValue > 12)
            {
                return OpcUa_BadOutOfRange;
            }

            if (milliSet)
            {
                /* be careful we can have more or less than 3 digits of milliseconds */
                tmpVar = signPosition - 20;
                if (tmpVar > 3)
                {
                    tmpVar = 3;
                }

                /* copy timezone */
#ifdef WIN32
#if OPCUA_USE_SAFE_FUNCTIONS
                strncpy_s(millis, 4, a_pchDateTimeString+20, tmpVar);
#else /* OPCUA_USE_SAFE_FUNCTIONS */
                strncpy(millis, a_pchDateTimeString+20, tmpVar);
#endif /* OPCUA_USE_SAFE_FUNCTIONS */
#endif

#ifdef _GNUC_
                strncpy(millis, a_pchDateTimeString+20, tmpVar);
#endif

                SystemTime.wMilliseconds =  (WORD)strtol(millis, 0, 10);
            }
        }
        else
        {
            /* error -> no timezone specified */
            /* a time without timezone is not an absolute time but a time span */
            /* we might handle this as UTC */
            return OpcUa_BadSyntaxError;
        }

        /* correct time to UTC */
        tmpVar = SystemTime.wHour - zoneValue;
        if (tmpVar > 23)
        {
            SystemTime.wHour = (WORD)(tmpVar - 24);
            dayChange = 1;     /* add one day to date */
        }
        else if (tmpVar < 0)
        {
            SystemTime.wHour = (WORD)(tmpVar + 24);
            dayChange = -1;    /* substract one day from date */
        }
        else
        {
            SystemTime.wHour = (WORD)(SystemTime.wHour - zoneValue);
            dayChange = 0;
        }

        /* day will change */
        if (dayChange != 0)
        {
            /* set day */
            tmpVar = SystemTime.wDay + dayChange;

            /* month will decrease */
            if (tmpVar == 0)
            {
                /* check which month */
                switch (SystemTime.wMonth)
                {
                    /* prior month has 31 days */
                case 1:
                case 2:
                case 4:
                case 6:
                case 8:
                case 9:
                case 11:
                    monthChange = -1;
                    SystemTime.wDay = 31;
                    break;

                    /* prior month has 30 days */
                case 5:
                case 7:
                case 10:
                case 12:
                    monthChange = -1;
                    SystemTime.wDay = 30;
                    break;

                    /* prior month has 28/29 days */
                case 3:

                    /* check if it's a leap year */
                    if (SystemTime.wYear % 4 == 0)
                    {
                        if (SystemTime.wYear % 100 == 0)
                        {
                            /* no leapyear */
                            if (SystemTime.wYear % 400 == 0)
                            {
                                leapYear = TRUE;
                            }
                            else
                            {
                                leapYear = FALSE;
                            }
                        }
                        else
                        {
                            leapYear = TRUE;
                        }
                    }
                    else
                    {
                        leapYear = FALSE;
                    }

                    if (leapYear == TRUE)
                    {
                        monthChange = -1;
                        SystemTime.wDay = 29;
                    }
                    else
                    {
                        monthChange = -1;
                        SystemTime.wDay = 28;
                    }

                    break;
                }
            }
            /* month might increase */
            else if (tmpVar > 27)
            {
                switch (SystemTime.wMonth)
                {
                    /* month has 31 days */
                case 1:
                case 3:
                case 5:
                case 7:
                case 8:
                case 10:
                case 12:
                    if (SystemTime.wDay == 31)
                    {
                        /* increase month */
                        monthChange = 1;
                        SystemTime.wDay = 1;
                    }
                    else
                    {
                        SystemTime.wDay++;
                        monthChange = 0;
                    }
                    break;

                    /* month has 30 days */
                case 4:
                case 6:
                case 9:
                case 11:
                    if (SystemTime.wDay == 30)
                    {
                        /* increase month */
                        monthChange = 1;
                        SystemTime.wDay = 1;
                    }
                    else
                    {
                        SystemTime.wDay++;
                        monthChange = 0;
                    }
                    break;

                    /* month has 27/28 days */
                case 2:
                    /* check if it's a leap year */
                    if (SystemTime.wYear % 4 == 0)
                    {
                        if (SystemTime.wYear % 100 == 0)
                        {
                            /* no leapyear */
                            if (SystemTime.wYear % 400 == 0)
                            {
                                leapYear = TRUE;
                            }
                            else
                            {
                                leapYear = FALSE;
                            }
                        }
                        else
                        {
                            leapYear = TRUE;
                        }
                    }
                    else
                    {
                        leapYear = FALSE;
                    }

                    if (leapYear == TRUE)
                    {
                        if (SystemTime.wDay == 29)
                        {
                            /* increase month */
                            monthChange = 1;
                            SystemTime.wDay = 1;
                        }
                        else
                        {
                            SystemTime.wDay++;
                            monthChange = 0;
                        }
                    }
                    else
                    {
                        if (SystemTime.wDay == 28)
                        {
                            /* increase month */
                            monthChange = 1;
                            SystemTime.wDay = 1;
                        }
                        else
                        {
                            SystemTime.wDay++;
                            monthChange = 0;
                        }
                    }
                    break;
                }
            }
            /* month will not change */
            else
            {
                SystemTime.wDay = (WORD)(SystemTime.wDay + dayChange);
                monthChange = 0;
            }

            /* set month */
            if (monthChange != 0)
            {
                tmpVar = SystemTime.wMonth + monthChange;

                /* decrease year */
                if (tmpVar == 0)
                {
                    SystemTime.wMonth = 12;
                    SystemTime.wYear--;
                }
                /* decrease year */
                else if (tmpVar == 13)
                {
                    SystemTime.wMonth = 1;
                    SystemTime.wYear++;
                }
                /* no year change */
                else
                {
                    SystemTime.wMonth = (WORD)(SystemTime.wMonth + monthChange);
                }
            }
        }
    }
    else /* if(strchr(a_pchDateTimeString, ':')) */
    {
        /* other formats are not supported at the moment */
        /* 20060606T06:48:48Z */
        /* 20060606T064848Z */
        return OpcUa_BadSyntaxError;
    }

    bResult = SystemTimeToFileTime(&SystemTime, &FileTime);
    if (bResult == FALSE)
    {
        return OpcUa_Bad;
    }

    a_pDateTime->dwHighDateTime = FileTime.dwHighDateTime;
    a_pDateTime->dwLowDateTime  = FileTime.dwLowDateTime;

    return OpcUa_Good;
}

/* BEGIN Montpellier Workshop */
//************************************************************************************
// Time Functions
#ifdef _GNUC_
DWORD GetTickCount(void)
{

    struct timespec tp;
	struct  timeval  tv;
 

	(void)gettimeofday(&tv, 0);
	tp.tv_sec = tv.tv_sec;
	tp.tv_nsec = tv.tv_usec*1000;


    DWORD dwCount = tp.tv_sec * 1000 + tp.tv_nsec/1000000; // Convert to milli seconds

    return dwCount;
}

/* Convert SystemTime to Filetime using POSIX time structure */
void GetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime)
{
	ULARGE_INTEGER ull;

    //TODO : revoir cette méthode -> pb de date avec le DA-661
    struct tm * tmTime;
    time_t timeTime;


    // Get Current Date and time in time_t type
    time(&timeTime);

    // Get the UTC time in tm struct
    tmTime = gmtime(&timeTime);

    //	Convert to time_t type
    timeTime = mktime(tmTime);

    // Build a LPFILETIME

    //ull.QuadPart = (timeTime + EPOCH_DIFF)* 10000000ULL;*/

    ull.QuadPart = EPOCH_DIFF;
    struct timeval tv;
    gettimeofday(&tv,NULL);

    ull.QuadPart += tv.tv_sec;

    ull.QuadPart *= 10000000ULL;

    ull.QuadPart += tv.tv_usec*10;

    lpSystemTimeAsFileTime->dwLowDateTime = ull.LowPart;
    lpSystemTimeAsFileTime->dwHighDateTime = ull.HighPart;
}

/* Convert Filetime to SystemTime using POSIX time structure */
BOOL FileTimeToSystemTime(const FILETIME *lpFileTime, LPSYSTEMTIME lpSystemTime)
{
    struct tm * tmTime;
    time_t timeTime;

    LONGLONG ll = lpFileTime->dwHighDateTime;
    ll = ll << 32;
    ll += lpFileTime->dwLowDateTime;

    /* Mise à l'échelle temps UNIX */

    ll = ll /10000000LL;
    timeTime = ll - EPOCH_DIFF;
    tmTime = gmtime(&timeTime);

    lpSystemTime->wYear = tmTime->tm_year + 1900;
    lpSystemTime->wMonth = tmTime->tm_mon +1;
    lpSystemTime->wDayOfWeek = tmTime->tm_wday;
    lpSystemTime->wDay = tmTime->tm_mday;
    lpSystemTime->wHour = tmTime->tm_hour; // just delete a +1 ... Need to validate this change
    lpSystemTime->wMinute = tmTime->tm_min;
    lpSystemTime->wSecond = tmTime->tm_sec;
    lpSystemTime->wMilliseconds = 0;

    return TRUE;

}

BOOL SystemTimeToFileTime(const LPSYSTEMTIME lpSystemTime, FILETIME *lpFileTime)
{

    struct tm *tmTime;
    time_t timeTime;

    ULARGE_INTEGER ull;
    time(&timeTime);
    tmTime = gmtime(&timeTime);

    // Convert systemtime to tm
    tmTime->tm_year = lpSystemTime->wYear - 1900;
    tmTime->tm_mon = lpSystemTime->wMonth - 1;
    tmTime->tm_wday = lpSystemTime->wDayOfWeek;
    tmTime->tm_mday = lpSystemTime->wDay;
    tmTime->tm_hour = lpSystemTime->wHour;
    tmTime->tm_min = lpSystemTime->wMinute;
    tmTime->tm_sec = lpSystemTime->wSecond;

    timeTime = mktime(tmTime);

    ull.QuadPart = (timeTime + EPOCH_DIFF) * 10000000ULL;

    lpFileTime->dwLowDateTime = ull.LowPart;
    lpFileTime->dwHighDateTime = ull.HighPart;

    return TRUE;

}

void GetLocalTime(LPSYSTEMTIME lpSystemTime)
{
    struct tm * tmTime;
    time_t timeTime;
    struct timespec tp;
	struct  timeval  tv;

    // Get local time
    time(&timeTime);
    tmTime = localtime(&timeTime);

    // Convert tm to systemtime
    lpSystemTime->wYear = tmTime->tm_year + 1900;
    lpSystemTime->wMonth = tmTime->tm_mon + 1;
    lpSystemTime->wDayOfWeek = tmTime->tm_wday;
    lpSystemTime->wDay = tmTime->tm_mday;
    lpSystemTime->wHour = tmTime->tm_hour;
    lpSystemTime->wMinute = tmTime->tm_min;
    lpSystemTime->wSecond = tmTime->tm_sec;


	// function failed. Let's try an alternative mecanism
	(void)gettimeofday(&tv, 0);
	tp.tv_sec = tv.tv_sec;
	tp.tv_nsec = tv.tv_usec*1000;

    lpSystemTime->wMilliseconds = tp.tv_nsec/1000000;
}
#endif

static const int month_to_day[12] =
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

time_t OPCUA_DLLCALL OpcUa_P_Mktime (struct tm *t)
{
	short month, year;
    time_t result;

    month = (short)t->tm_mon;
    year = ((short)t->tm_year) + month / 12 + 1900;
    month %= 12;
    if (month < 0)
    {
        year -= 1;
        month += 12;
    }
    result = (year - 1970) * 365 + (year - 1969) / 4 + month_to_day[month];
    result = (year - 1970) * 365 + month_to_day[month];
    if (month <= 1)
        year -= 1;
    result += (year - 1968) / 4;
    result -= (year - 1900) / 100;
    result += (year - 1600) / 400;
    result += t->tm_mday;
    result -= 1;
    result *= 24;
    result += t->tm_hour;
    result *= 60;
    result += t->tm_min;
    result *= 60;
    result += t->tm_sec;
    return (result);
}
