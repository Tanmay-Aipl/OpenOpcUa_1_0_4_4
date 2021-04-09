//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define FACILITY_SYSTEM                  0x0
#define FACILITY_STUBS                   0x3
#define FACILITY_RUNTIME                 0x2
#define FACILITY_IO_ERROR_CODE           0x4


//
// Define the severity codes
//
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


//
// MessageId: EVMSG_INSTALLED
//
// MessageText:
//
//  The %1 service was installed.
//
#define EVMSG_INSTALLED                  ((DWORD)0x40000064L)


//
// MessageId: EVMSG_REMOVED
//
// MessageText:
//
//  The %1 service was removed.
//
#define EVMSG_REMOVED                    ((DWORD)0x40000065L)


//
// MessageId: EVMSG_NOTINSTALLED
//
// MessageText:
//
//  The %1 service cannot be installed.
//
#define EVMSG_NOTINSTALLED               ((DWORD)0xC0000066L)


//
// MessageId: EVMSG_NOTREMOVED
//
// MessageText:
//
//  The %1 service cannot be removed.
//
#define EVMSG_NOTREMOVED                 ((DWORD)0xC0000067L)


//
// MessageId: EVMSG_FAILED
//
// MessageText:
//
//  The %1 service failed.
//
#define EVMSG_FAILED                     ((DWORD)0xC0000068L)


//
// MessageId: EVMSG_CTRLHANDLERNOTINSTALLED
//
// MessageText:
//
//  The control handler could not be installed.
//
#define EVMSG_CTRLHANDLERNOTINSTALLED    ((DWORD)0xC0000069L)


//
// MessageId: EVMSG_STOPPED
//
// MessageText:
//
//  The service was stopped.
//
#define EVMSG_STOPPED                    ((DWORD)0x4000006AL)


//
// MessageId: EVMSG_BADREQUEST
//
// MessageText:
//
//  The service received an unsupported request.
//
#define EVMSG_BADREQUEST                 ((DWORD)0xC000006BL)


//
// MessageId: EVMSG_SUCCEED
//
// MessageText:
//
//  The service operation succeed.
//
#define EVMSG_SUCCEED                    ((DWORD)0x4000006CL)


//
// MessageId: EVMSG_SIMULATION_STARTED
//
// MessageText:
//
//  The simulation mecanism was started successfuly.
//
#define EVMSG_SIMULATION_STARTED         ((DWORD)0x4000006DL)


//
// MessageId: EVMSG_OPCCFG_FAILED
//
// MessageText:
//
//  Cannot load configuration file from registry.
//
#define EVMSG_OPCCFG_FAILED              ((DWORD)0xC000006EL)


//
// MessageId: EVMSG_OPCCFG_ERR_LVL01
//
// MessageText:
//
//  Cannot read registry information - Level01 error.
//
#define EVMSG_OPCCFG_ERR_LVL01           ((DWORD)0xC000006FL)


//
// MessageId: EVMSG_OPCCFG_ERR_LVL02
//
// MessageText:
//
//  Cannot read registry information - Level02 error.
//
#define EVMSG_OPCCFG_ERR_LVL02           ((DWORD)0xC0000070L)


//
// MessageId: EVMSG_OPCCFG_ERR_LVL03
//
// MessageText:
//
//  Cannot read registry information - Level03 error.
//
#define EVMSG_OPCCFG_ERR_LVL03           ((DWORD)0xC0000071L)

