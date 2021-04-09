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
	namespace UAAddressSpace 
	{
		// cette classe contient la definition des champs utilisés dans le UADataType
		// ces champs sont parsés a partir du fichier XML balise <Definition>
		class CDefinition
		{
		public:
			CDefinition(void);
			CDefinition(const char **atts);
			~CDefinition(void);
			// FieldList manipulation
			void AddField(CField* pField)
			{
				m_FieldList.push_back(pField);
			}
			std::vector<CField*> GetFieldList() {return m_FieldList;}
			// Name
			OpcUa_QualifiedName* GetName()
			{
				return &m_Name;
			}
			void SetName(OpcUa_QualifiedName* aName)
			{
				OpcUa_String_AttachCopy(&(m_Name.Name),OpcUa_String_GetRawString(&(aName->Name)));
				m_Name.NamespaceIndex=0;
			}
			// BaseType
			OpcUa_QualifiedName* GetBaseType()
			{
				return &m_Name;
			}
			void SetBaseType(OpcUa_QualifiedName* aBaseType)
			{
				OpcUa_String_AttachCopy(&(m_BaseType.Name),OpcUa_String_GetRawString(&(aBaseType->Name)));
				m_BaseType.NamespaceIndex=0;
			}
			// SymbolicName
			OpcUa_String	GetSymbolicName() {return m_SymbolicName;}
			void SetSymbolicName(OpcUa_String* strVal) 
			{
				OpcUa_String_CopyTo(strVal,&m_SymbolicName);
			}
			OpcUa_Int32 GetSize();
			// compute the sise of the instance 
			OpcUa_StatusCode GetInstanceSize(OpcUa_Int32 *pSize);
			// Duplicate any extension object from an existing one based on its definition
			OpcUa_StatusCode DuplicateExtensionObject(OpcUa_ExtensionObject* pExtensionObject, OpcUa_ExtensionObject** ppNewExtensionObject);		
		private:
				OpcUa_StatusCode DuplicateString(void** pVoidBuf, void** pVoidResult);
				OpcUa_StatusCode DuplicateNodeId(void** pVoidBuf, void** pVoidResult);
		private:
			OpcUa_QualifiedName		m_Name;
			OpcUa_QualifiedName		m_BaseType;
			OpcUa_String			m_SymbolicName;
			std::vector<CField*>	m_FieldList;
		};
	}
}