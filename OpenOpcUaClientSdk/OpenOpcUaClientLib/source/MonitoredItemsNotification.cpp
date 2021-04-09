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
#include "stdafx.h"
#include "MonitoredItemsNotification.h"
using namespace OpenOpcUa;
using namespace UASharedLib;
using namespace UACoreClient;

CMonitoredItemsNotification::CMonitoredItemsNotification()
{
	m_pMonitoredItemNotification = OpcUa_Null;
	m_iNoOfMonitoredItems = 0;
	m_bDone = OpcUa_False;
}

CMonitoredItemsNotification::~CMonitoredItemsNotification()
{
	if (m_pMonitoredItemNotification)
	{
		for (OpcUa_Int32 ii = 0; ii < m_iNoOfMonitoredItems; ii++)
		{
			OpcUa_MonitoredItemNotification_Clear(&m_pMonitoredItemNotification[ii]);
		}
		OpcUa_Free(m_pMonitoredItemNotification);
	}
}
OpcUa_MonitoredItemNotification* CMonitoredItemsNotification::GetMonitoredItemNotification() 
{ 
	return m_pMonitoredItemNotification; 
}
void CMonitoredItemsNotification::SetMonitoredItemNotification(OpcUa_Int32 iNoOfMonitoredItems, OpcUa_MonitoredItemNotification* pMonitoredItemNotification)
{
	if (m_pMonitoredItemNotification)
	{
		for (OpcUa_Int32 i = 0; i < m_iNoOfMonitoredItems; i++)
			OpcUa_MonitoredItemNotification_Clear(&m_pMonitoredItemNotification[i]);
		OpcUa_Free(m_pMonitoredItemNotification);
	}
	m_pMonitoredItemNotification = (OpcUa_MonitoredItemNotification*)OpcUa_Alloc(iNoOfMonitoredItems*sizeof(OpcUa_MonitoredItemNotification));
	for (OpcUa_Int32 i = 0; i < iNoOfMonitoredItems; i++)
	{
		OpcUa_MonitoredItemNotification_Initialize(&(m_pMonitoredItemNotification[i]));
		m_pMonitoredItemNotification[i].ClientHandle = pMonitoredItemNotification[i].ClientHandle;
		OpcUa_DataValue_CopyTo(
			&(pMonitoredItemNotification[i].Value),
			&(m_pMonitoredItemNotification[i].Value));
	}
}

OpcUa_Boolean CMonitoredItemsNotification::IsDone()
{
	return m_bDone;
}
void CMonitoredItemsNotification::Done()
{
	m_bDone = OpcUa_True;
}
OpcUa_Int32 CMonitoredItemsNotification::GetNoOfMonitoredItems()
{
	return m_iNoOfMonitoredItems;
}
void CMonitoredItemsNotification::SetNoOfMonitoredItems(OpcUa_Int32 iVal)
{
	m_iNoOfMonitoredItems = iVal;
}