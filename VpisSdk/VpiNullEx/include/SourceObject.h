#pragma once
using namespace VpiBuiltinType;
namespace UASubSystem 
{	
	class CSourceObject
	{
	public:
		CSourceObject(void);
		~CSourceObject(void);
		// NodeId
		Vpi_NodeId GetNodeId()
		{
			return m_NodeId;
		}
		void SetNodeId(Vpi_NodeId aNodeId)
		{
			Vpi_NodeId_CopyTo(&aNodeId,&m_NodeId);
		}
		Vpi_DataValue* GetValue() { return m_pValue; }
		void SetValue(Vpi_DataValue* aValue) 
		{ 
			if (!m_pValue)
				m_pValue = (Vpi_DataValue*)malloc(sizeof(Vpi_DataValue));
			else
				Vpi_DataValue_Clear(m_pValue);
			Vpi_DataValue_Initialize(m_pValue);
			Vpi_DataValue_CopyTo(aValue, m_pValue); 
		}
		Vpi_String GetAddress() {return m_Address;}
		void SetAddress(Vpi_String Val) {Vpi_String_CopyTo(&Val,&m_Address);}
	private:
		Vpi_NodeId		m_NodeId;  // NodeId associé a cet element
		Vpi_String		m_Address; // Alias permettant d'identifier l'element dans l'equipement. Chaque adresse est unique
		Vpi_DataValue*	m_pValue;  // Valeur
	};
	typedef std::vector<CSourceObject*> CSourceObjectList;
}
