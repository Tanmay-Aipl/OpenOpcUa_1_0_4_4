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
using namespace UASharedLib;
namespace OpenOpcUa
{
	namespace UAAddressSpace 
	{
		// cette classe contient la definition des champs utilisés dans le UADataType
		// ces champs sont parsés a partir du fichier XML balise <Field>
		class CDefinition;
		class CField
		{
		public:
			CField(void);
			CField(const char **atts);
			~CField(void);
			OpcUa_StatusCode GetInstanceFieldSize(CUAVariable* pUAVariable);
			OpcUa_Int32 GetFieldSize() 
			{
				if (m_iFieldSize==0)
					UpdateFieldSize();
				return m_iFieldSize;
			}
			CDefinition* GetDefinition() {return	m_pDefinition;}
			void	SetDefinition(CDefinition* pDefinition) {m_pDefinition=pDefinition;}
			// description
			OpcUa_LocalizedText GetDescription()
			{
				return m_Description;
			}
			void SetDescription(OpcUa_LocalizedText aValue)
			{
				OpcUa_LocalizedText_Clear(&m_Description);
				OpcUa_LocalizedText_CopyTo(&aValue, &m_Description);
			}
			// Datatype of the field
			OpcUa_NodeId GetDataType() {return 	m_DataType;}
			void SetDataType(OpcUa_NodeId aNodeId) 
			{

				OpcUa_NodeId_Initialize(&m_DataType);
				OpcUa_NodeId_CopyTo(&aNodeId,&m_DataType);

				UpdateFieldSize();
			}
			// SymbolicName
			OpcUa_String	GetSymbolicName() {return m_SymbolicName;}
			void SetSymbolicName(OpcUa_String* strVal) 
			{
				OpcUa_String_StrnCpy(&m_SymbolicName,strVal,OpcUa_String_StrLen(strVal));
			}
			// Name
			OpcUa_String	GetName() {return m_Name;}
			void SetName(OpcUa_String* strVal) 
			{
				OpcUa_String_StrnCpy(&m_Name,strVal,OpcUa_String_StrLen(strVal));
			}
			// ValueRank
			OpcUa_Int32 GetValueRank()
			{
				return m_ValueRank;
			}
			void SetValueRank(OpcUa_Int32 aValue)
			{
				m_ValueRank=aValue;
			}
			OpcUa_StatusCode UpdateFieldSize(); // calcul de la taille de m_iFieldSize
		private:
			OpcUa_LocalizedText		m_Description; // description du champ.Correspond a la balise <Description>
			CDefinition*			m_pDefinition; // instance de la classe CDefintion encapsuler en cas de type plus complexe
												   // Correspond a la balise <Definition>
			OpcUa_Int32				m_iFieldSize; // permet de connaite la taille du type encapsulé
			OpcUa_String			m_Name;
			OpcUa_String			m_SymbolicName;
			OpcUa_NodeId			m_DataType; // NodeId correspondant au type du champ
			OpcUa_Int32				m_ValueRank;
		};
	}
}