#pragma once
using namespace VpiBuiltinType;
namespace UASubSystem 
{	
	class CSourceObject
	{
	public:
		CSourceObject(void);
		CSourceObject(Vpi_Byte Datatype, Vpi_UInt32 iNbElt);
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
		Vpi_NodeId m_NodeId;
		Vpi_String m_Address;
		CVpiDataValue* m_pValue;
	};
	typedef std::vector<CSourceObject*> CSourceObjectList;
}