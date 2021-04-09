/*****************************************************************************
	  Author
		�. Michel Condemine, 4CE Industry (2010-2012)
	  
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
#include <typeinfo>
#include <map>
#include <vector>
#include <string>
using namespace std;
using namespace OpenOpcUa;
using namespace UASharedLib;
namespace OpenOpcUa
{
	namespace UAAddressSpace 
	{
		typedef std::vector<OpcUa_ReferenceNode*> OpcUa_ReferenceNodeList;
		class CUABase
		{
		public:
			CUABase(void);
			CUABase(OpcUa_NodeClass aNodeClass, const char **atts);

			~CUABase(void);
			void Init(const char** atts);
			// NodeId
			OpcUa_NodeId* GetNodeId();
			void SetNodeId(OpcUa_NodeId aNodeId);
			// NodeClass
			OpcUa_NodeClass GetNodeClass() const;
			void SetNodeClass(OpcUa_NodeClass aNodeClass);
			// BrowseName
			OpcUa_QualifiedName* GetBrowseName() const;
			void SetBrowseName(OpcUa_QualifiedName* aName);
			OpcUa_LocalizedText GetDisplayName() const;
			void SetDisplayName(OpcUa_LocalizedText* pValue);
			OpcUa_LocalizedText GetDescription();
			void SetDescription(OpcUa_LocalizedText aValue);
			OpcUa_UInt32 GetWriteMask();
			void SetWriteMask(OpcUa_UInt32 aValue);
			OpcUa_UInt32 GetUserWriteMask();
			void SetUserWriteMask(OpcUa_UInt32 aValue);
			OpcUa_Int32 GetNoOfReferences();
			CUAReference* operator[](int index);
			CUABase operator=(CUABase* pUABase);
			CUAReferenceList* GetReferenceNodeList();
			OpcUa_StatusCode Write(OpcUa_UInt32 AttributeId, OpcUa_String szIndexRange, OpcUa_DataValue Value);
			OpcUa_NodeId GetTypeDefinition() const;
			void SetTypeDefinition(OpcUa_NodeId* pNodeId);
			OpcUa_StatusCode IsReferenceExist(CUAReference* pRefNode); // d�termine si la reference pRefNode existe d�ja sur cette node
		protected:
			OpcUa_NodeId*				m_pNodeId;
			OpcUa_NodeClass				m_NodeClass;
			OpcUa_QualifiedName*		m_pBrowseName;
			OpcUa_LocalizedText			m_DisplayName;
			OpcUa_LocalizedText			m_Description;
			OpcUa_UInt32				m_WriteMask;
			OpcUa_UInt32				m_UserWriteMask;			
			CUAReferenceList*			m_pReferences;
			OpcUa_NodeId				m_TypeDefinition; // Il s'agit d'un NodeId contenant la definition du type du noeud. Cette definition correspond � la r�f�rence HasTypeDefinition associ� a ce noeud.
		};
	}
}