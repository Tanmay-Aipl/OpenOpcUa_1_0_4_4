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
#include "ContinuationPoint.h"

using namespace OpenOpcUa;
using namespace UACoreServer;
CContinuationPoint::CContinuationPoint(void)
{
	//m_ppContinuationPointBrowseResults=(OpcUa_BrowseResult**)OpcUa_Alloc(sizeof(OpcUa_BrowseResult*));
	//*m_ppContinuationPointBrowseResults=OpcUa_Null;
	m_pBrowseDescription=OpcUa_Null;
	//m_pContinuationPointNoOfBrowseResult=0;
	OpcUa_Mutex_Create(&m_hContinuationPointListMutex);
	//m_RefIterator=OpcUa_Null;
	m_CurrentIndex=0;
}

CContinuationPoint::~CContinuationPoint(void)
{
	if (m_pBrowseDescription)
	{
		OpcUa_BrowseDescription_Clear(m_pBrowseDescription);
		OpcUa_Free(m_pBrowseDescription);
	}
	OpcUa_Mutex_Delete(&m_hContinuationPointListMutex);
}
//void CContinuationPoint::AddBrowseResult(OpcUa_BrowseResult* pNewBrowseResult)
//{
//	//OpcUa_Mutex_Lock(m_hContinuationPointListMutex);
//	std::vector<OpcUa_BrowseResult*> aContinuationPointBrowseResultList;
//	if (*m_ppContinuationPointBrowseResults)
//	{
//		// sauvegarde de l'existant
//		for (OpcUa_UInt32 i=0;i<m_pContinuationPointNoOfBrowseResult;i++)
//		{
//			OpcUa_BrowseResult* pBrowseResult=m_ppContinuationPointBrowseResults[i];
//			aContinuationPointBrowseResultList.push_back((OpcUa_BrowseResult*)pBrowseResult);
//		}
//
//		OpcUa_Free(*m_ppContinuationPointBrowseResults);
//		*m_ppContinuationPointBrowseResults=OpcUa_Null;
//	}
//	// reallocation 
//	m_pContinuationPointNoOfBrowseResult++;
//	*m_ppContinuationPointBrowseResults=(OpcUa_BrowseResult*)OpcUa_Alloc(m_pContinuationPointNoOfBrowseResult*sizeof(OpcUa_BrowseResult*));
//	// copie dans la destination m_pContinuationPointBrowseResults
//	for (OpcUa_UInt32 ii=0;ii<aContinuationPointBrowseResultList.size();ii++)
//	{
//		OpcUa_BrowseResult* pBrowseResult=aContinuationPointBrowseResultList.at(ii);
//		m_ppContinuationPointBrowseResults[ii]=pBrowseResult;
//	}
//	// ajout du nouvel element
//	m_ppContinuationPointBrowseResults[m_pContinuationPointNoOfBrowseResult-1]=Utils::Copy(pNewBrowseResult);
//	//OpcUa_Mutex_Unlock(m_hContinuationPointListMutex);
//}
void CContinuationPoint::SetBrowseDescription(OpcUa_BrowseDescription* pBrowseDescription)
{
	if (m_pBrowseDescription)
	{
		OpcUa_BrowseDescription_Clear(m_pBrowseDescription);
		OpcUa_Free(m_pBrowseDescription);
	}
	m_pBrowseDescription=(OpcUa_BrowseDescription*)OpcUa_Alloc(sizeof(OpcUa_BrowseDescription));
	OpcUa_BrowseDescription_Initialize(m_pBrowseDescription);
	m_pBrowseDescription->BrowseDirection=pBrowseDescription->BrowseDirection;
	m_pBrowseDescription->IncludeSubtypes=pBrowseDescription->IncludeSubtypes;
	m_pBrowseDescription->NodeClassMask=pBrowseDescription->NodeClassMask;
	OpcUa_NodeId_CopyTo(&(pBrowseDescription->NodeId),&(m_pBrowseDescription->NodeId));
	
	m_pBrowseDescription->ReferenceTypeId=pBrowseDescription->ReferenceTypeId;
	m_pBrowseDescription->ResultMask=pBrowseDescription->ResultMask;
}