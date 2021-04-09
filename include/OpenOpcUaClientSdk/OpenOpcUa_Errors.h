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

/*============================================================================
* Everything is as expected.
*===========================================================================*/
#define OpcUa_Good 0x00000000

/*============================================================================
* Something unexpected occurred but the results still maybe useable.
*===========================================================================*/
#define OpcUa_Uncertain 0x40000000

/*============================================================================
* An error occurred.
*===========================================================================*/
#define OpcUa_Bad 0x80000000
/*============================================================================
* An unexpected error occurred.
*===========================================================================*/
#define OpcUa_BadUnexpectedError 0x80010000

/*============================================================================
* An internal error occurred as a result of a programming or configuration error.
*===========================================================================*/
#define OpcUa_BadInternalError 0x80020000

/*============================================================================
* Not enough memory to complete the operation.
*===========================================================================*/
#define OpcUa_BadOutOfMemory 0x80030000
/*============================================================================
* The operation timed out.
*===========================================================================*/
#define OpcUa_BadTimeout 0x800A0000

/*============================================================================
* The operation was cancelled because the application is shutting down.
*===========================================================================*/
#define OpcUa_BadShutdown 0x800C0000
/*============================================================================
* One or more arguments are invalid.
*===========================================================================*/
#define OpcUa_BadInvalidArgument 0x80AB0000
/*============================================================================
* The operation cannot be completed because the object is closed, uninitialized or in some other invalid state.
*===========================================================================*/
#define OpcUa_BadInvalidState 0x80AF0000
/*============================================================================
* Invalid file name specified.
*===========================================================================*/
#define OpcUa_BadFileNotFound 0x81090000