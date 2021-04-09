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
#pragma once
using namespace std;
namespace OpenOpcUa
{
	namespace UASubSystem
	{
		class CVpiFuncCaller;
		class CVpiWriteObject
		{
		public:
			CVpiWriteObject();
			CVpiWriteObject(OpcUa_NodeId	m_NodeId,CVpiDataValue* m_pVpiDataValue,CVpiFuncCaller* pFuncCaller,CUAVariable* pUAVariable);
			~CVpiWriteObject();
			CVpiFuncCaller* GetVpiFuncCaller() {return m_pFuncCaller;}
			OpcUa_NodeId GetNodeId(){return m_NodeId;}
			CVpiDataValue*	GetVpiDataValue() {return m_pVpiDataValue;}
			CUAVariable* GetUAVariable() {return 	m_pUAVariable;}
			OpcUa_Boolean	WasWrote() { return m_bWrite; }
			void Wrote() { m_bWrite = OpcUa_True; }
		private:
			CVpiDataValue*	m_pVpiDataValue; // Value to write
			OpcUa_NodeId	m_NodeId; // Node Id to write
			CVpiFuncCaller* m_pFuncCaller;
			CUAVariable*	m_pUAVariable;
			OpcUa_Boolean	m_bWrite; // indicate that the write was proceeded
		};
		typedef std::vector<CVpiWriteObject*> CVpiWriteObjectList;
	}
}