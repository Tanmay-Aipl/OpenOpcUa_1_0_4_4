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
			m_NodeId=aNodeId;
		}
		CVpiDataValue* GetValue() {return m_pValue;}
		void SetValue(CVpiDataValue* aValue) {m_pValue=aValue;}
		Vpi_String GetAddress() {return m_Address;}
		void SetAddress(Vpi_String Val) {m_Address=Val;}
	private:
		Vpi_NodeId	m_NodeId;  // NodeId associé a cet element
		Vpi_String	m_Address; // Alias permettant d'identifier l'element dans l'equipement. Chaque adresse est unique
		CVpiDataValue*	m_pValue;  // Valeur
	};
	typedef std::vector<CSourceObject*> CSourceObjectList;
}
