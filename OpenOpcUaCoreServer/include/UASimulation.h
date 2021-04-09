#pragma once

#using <mscorlib.dll>
#using <System.dll>
#using <System.Data.dll>
#using <System.Xml.dll>

using namespace System::Security::Permissions;
[assembly:SecurityPermissionAttribute(SecurityAction::RequestMinimum, SkipVerification=false)];
// 
// Ce code source a été automatiquement généré par xsd, Version=2.0.50727.3038.
// 
namespace Server {
    using namespace System;
    ref class ServerSimulation;
    
    
    /// <summary>
///Represents a strongly typed in-memory cache of data.
///</summary>
    [System::CodeDom::Compiler::GeneratedCodeAttribute(L"System.Data.Design.TypedDataSetGenerator", L"2.0.0.0"), 
    System::Serializable, 
    System::ComponentModel::DesignerCategoryAttribute(L"code"), 
    System::ComponentModel::ToolboxItem(true), 
    System::Xml::Serialization::XmlSchemaProviderAttribute(L"GetTypedDataSetSchema"), 
    System::Xml::Serialization::XmlRootAttribute(L"ServerSimulation"), 
    System::ComponentModel::Design::HelpKeywordAttribute(L"vs.data.DataSet")]
    public ref class ServerSimulation : public ::System::Data::DataSet {
        public : ref class GroupDataTable;
        public : ref class SimulatedDataTable;
        public : ref class GroupRow;
        public : ref class SimulatedRow;
        public : ref class GroupRowChangeEvent;
        public : ref class SimulatedRowChangeEvent;
        
        private: Server::ServerSimulation::GroupDataTable^  tableGroup;
        
        private: Server::ServerSimulation::SimulatedDataTable^  tableSimulated;
        
        private: ::System::Data::DataRelation^  relationGroup_Simulated;
        
        private: ::System::Data::SchemaSerializationMode _schemaSerializationMode;
        
        public : delegate System::Void GroupRowChangeEventHandler(::System::Object^  sender, Server::ServerSimulation::GroupRowChangeEvent^  e);
        
        public : delegate System::Void SimulatedRowChangeEventHandler(::System::Object^  sender, Server::ServerSimulation::SimulatedRowChangeEvent^  e);
        
        public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
        ServerSimulation();
        protected: [System::Diagnostics::DebuggerNonUserCodeAttribute]
        ServerSimulation(::System::Runtime::Serialization::SerializationInfo^  info, ::System::Runtime::Serialization::StreamingContext context);
        public: [System::Diagnostics::DebuggerNonUserCodeAttribute, 
        System::ComponentModel::Browsable(false), 
        System::ComponentModel::DesignerSerializationVisibility(::System::ComponentModel::DesignerSerializationVisibility::Content)]
        property Server::ServerSimulation::GroupDataTable^  Group {
            Server::ServerSimulation::GroupDataTable^  get();
        }
        
        public: [System::Diagnostics::DebuggerNonUserCodeAttribute, 
        System::ComponentModel::Browsable(false), 
        System::ComponentModel::DesignerSerializationVisibility(::System::ComponentModel::DesignerSerializationVisibility::Content)]
        property Server::ServerSimulation::SimulatedDataTable^  Simulated {
            Server::ServerSimulation::SimulatedDataTable^  get();
        }
        
        public: [System::Diagnostics::DebuggerNonUserCodeAttribute, 
        System::ComponentModel::BrowsableAttribute(true), 
        System::ComponentModel::DesignerSerializationVisibilityAttribute(::System::ComponentModel::DesignerSerializationVisibility::Visible)]
        virtual property ::System::Data::SchemaSerializationMode SchemaSerializationMode {
            ::System::Data::SchemaSerializationMode get() override;
            System::Void set(::System::Data::SchemaSerializationMode value) override;
        }
        
        public: [System::Diagnostics::DebuggerNonUserCodeAttribute, 
        System::ComponentModel::DesignerSerializationVisibilityAttribute(::System::ComponentModel::DesignerSerializationVisibility::Hidden)]
        property ::System::Data::DataTableCollection^  Tables {
            ::System::Data::DataTableCollection^  get() new;
        }
        
        public: [System::Diagnostics::DebuggerNonUserCodeAttribute, 
        System::ComponentModel::DesignerSerializationVisibilityAttribute(::System::ComponentModel::DesignerSerializationVisibility::Hidden)]
        property ::System::Data::DataRelationCollection^  Relations {
            ::System::Data::DataRelationCollection^  get() new;
        }
        
        protected: [System::Diagnostics::DebuggerNonUserCodeAttribute]
        virtual ::System::Void InitializeDerivedDataSet() override;
        
        public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
        virtual ::System::Data::DataSet^  Clone() override;
        
        protected: [System::Diagnostics::DebuggerNonUserCodeAttribute]
        virtual ::System::Boolean ShouldSerializeTables() override;
        
        protected: [System::Diagnostics::DebuggerNonUserCodeAttribute]
        virtual ::System::Boolean ShouldSerializeRelations() override;
        
        protected: [System::Diagnostics::DebuggerNonUserCodeAttribute]
        virtual ::System::Void ReadXmlSerializable(::System::Xml::XmlReader^  reader) override;
        
        protected: [System::Diagnostics::DebuggerNonUserCodeAttribute]
        virtual ::System::Xml::Schema::XmlSchema^  GetSchemaSerializable() override;
        
        internal: [System::Diagnostics::DebuggerNonUserCodeAttribute]
        ::System::Void InitVars();
        
        internal: [System::Diagnostics::DebuggerNonUserCodeAttribute]
        ::System::Void InitVars(::System::Boolean initTable);
        
        private: [System::Diagnostics::DebuggerNonUserCodeAttribute]
        ::System::Void InitClass();
        
        private: [System::Diagnostics::DebuggerNonUserCodeAttribute]
        ::System::Boolean ShouldSerializeGroup();
        
        private: [System::Diagnostics::DebuggerNonUserCodeAttribute]
        ::System::Boolean ShouldSerializeSimulated();
        
        private: [System::Diagnostics::DebuggerNonUserCodeAttribute]
        ::System::Void SchemaChanged(::System::Object^  sender, ::System::ComponentModel::CollectionChangeEventArgs^  e);
        
        public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
        static ::System::Xml::Schema::XmlSchemaComplexType^  GetTypedDataSetSchema(::System::Xml::Schema::XmlSchemaSet^  xs);
        
        public : /// <summary>
///Represents the strongly named DataTable class.
///</summary>
        [System::CodeDom::Compiler::GeneratedCodeAttribute(L"System.Data.Design.TypedDataSetGenerator", L"2.0.0.0"), 
        System::Serializable, 
        System::Xml::Serialization::XmlSchemaProviderAttribute(L"GetTypedTableSchema")]
        ref class GroupDataTable : public ::System::Data::DataTable, public ::System::Collections::IEnumerable {
            
            private: ::System::Data::DataColumn^  columnRate;
            
            private: ::System::Data::DataColumn^  columnGroup_Id;
            
            public: event Server::ServerSimulation::GroupRowChangeEventHandler^  GroupRowChanging;
            
            public: event Server::ServerSimulation::GroupRowChangeEventHandler^  GroupRowChanged;
            
            public: event Server::ServerSimulation::GroupRowChangeEventHandler^  GroupRowDeleting;
            
            public: event Server::ServerSimulation::GroupRowChangeEventHandler^  GroupRowDeleted;
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            GroupDataTable();
            internal: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            GroupDataTable(::System::Data::DataTable^  table);
            protected: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            GroupDataTable(::System::Runtime::Serialization::SerializationInfo^  info, ::System::Runtime::Serialization::StreamingContext context);
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            property ::System::Data::DataColumn^  RateColumn {
                ::System::Data::DataColumn^  get();
            }
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            property ::System::Data::DataColumn^  Group_IdColumn {
                ::System::Data::DataColumn^  get();
            }
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute, 
            System::ComponentModel::Browsable(false)]
            property ::System::Int32 Count {
                ::System::Int32 get();
            }
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            property Server::ServerSimulation::GroupRow^  default [::System::Int32 ] {
                Server::ServerSimulation::GroupRow^  get(::System::Int32 index);
            }
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            ::System::Void AddGroupRow(Server::ServerSimulation::GroupRow^  row);
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            Server::ServerSimulation::GroupRow^  AddGroupRow(System::Int64 Rate);
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            virtual ::System::Collections::IEnumerator^  GetEnumerator();
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            virtual ::System::Data::DataTable^  Clone() override;
            
            protected: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            virtual ::System::Data::DataTable^  CreateInstance() override;
            
            internal: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            ::System::Void InitVars();
            
            private: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            ::System::Void InitClass();
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            Server::ServerSimulation::GroupRow^  NewGroupRow();
            
            protected: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            virtual ::System::Data::DataRow^  NewRowFromBuilder(::System::Data::DataRowBuilder^  builder) override;
            
            protected: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            virtual ::System::Type^  GetRowType() override;
            
            protected: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            virtual ::System::Void OnRowChanged(::System::Data::DataRowChangeEventArgs^  e) override;
            
            protected: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            virtual ::System::Void OnRowChanging(::System::Data::DataRowChangeEventArgs^  e) override;
            
            protected: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            virtual ::System::Void OnRowDeleted(::System::Data::DataRowChangeEventArgs^  e) override;
            
            protected: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            virtual ::System::Void OnRowDeleting(::System::Data::DataRowChangeEventArgs^  e) override;
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            ::System::Void RemoveGroupRow(Server::ServerSimulation::GroupRow^  row);
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            static ::System::Xml::Schema::XmlSchemaComplexType^  GetTypedTableSchema(::System::Xml::Schema::XmlSchemaSet^  xs);
        };
        
        public : /// <summary>
///Represents the strongly named DataTable class.
///</summary>
        [System::CodeDom::Compiler::GeneratedCodeAttribute(L"System.Data.Design.TypedDataSetGenerator", L"2.0.0.0"), 
        System::Serializable, 
        System::Xml::Serialization::XmlSchemaProviderAttribute(L"GetTypedTableSchema")]
        ref class SimulatedDataTable : public ::System::Data::DataTable, public ::System::Collections::IEnumerable {
            
            private: ::System::Data::DataColumn^  columnNodeId;
            
            private: ::System::Data::DataColumn^  columnSimulationType;
            
            private: ::System::Data::DataColumn^  columnMin;
            
            private: ::System::Data::DataColumn^  columnMax;
            
            private: ::System::Data::DataColumn^  columnGroup_Id;
            
            public: event Server::ServerSimulation::SimulatedRowChangeEventHandler^  SimulatedRowChanging;
            
            public: event Server::ServerSimulation::SimulatedRowChangeEventHandler^  SimulatedRowChanged;
            
            public: event Server::ServerSimulation::SimulatedRowChangeEventHandler^  SimulatedRowDeleting;
            
            public: event Server::ServerSimulation::SimulatedRowChangeEventHandler^  SimulatedRowDeleted;
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            SimulatedDataTable();
            internal: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            SimulatedDataTable(::System::Data::DataTable^  table);
            protected: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            SimulatedDataTable(::System::Runtime::Serialization::SerializationInfo^  info, ::System::Runtime::Serialization::StreamingContext context);
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            property ::System::Data::DataColumn^  NodeIdColumn {
                ::System::Data::DataColumn^  get();
            }
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            property ::System::Data::DataColumn^  SimulationTypeColumn {
                ::System::Data::DataColumn^  get();
            }
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            property ::System::Data::DataColumn^  MinColumn {
                ::System::Data::DataColumn^  get();
            }
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            property ::System::Data::DataColumn^  MaxColumn {
                ::System::Data::DataColumn^  get();
            }
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            property ::System::Data::DataColumn^  Group_IdColumn {
                ::System::Data::DataColumn^  get();
            }
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute, 
            System::ComponentModel::Browsable(false)]
            property ::System::Int32 Count {
                ::System::Int32 get();
            }
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            property Server::ServerSimulation::SimulatedRow^  default [::System::Int32 ] {
                Server::ServerSimulation::SimulatedRow^  get(::System::Int32 index);
            }
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            ::System::Void AddSimulatedRow(Server::ServerSimulation::SimulatedRow^  row);
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            Server::ServerSimulation::SimulatedRow^  AddSimulatedRow(System::String^  NodeId, System::String^  SimulationType, 
                        System::Double Min, System::Double Max, Server::ServerSimulation::GroupRow^  parentGroupRowByGroup_Simulated);
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            virtual ::System::Collections::IEnumerator^  GetEnumerator();
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            virtual ::System::Data::DataTable^  Clone() override;
            
            protected: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            virtual ::System::Data::DataTable^  CreateInstance() override;
            
            internal: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            ::System::Void InitVars();
            
            private: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            ::System::Void InitClass();
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            Server::ServerSimulation::SimulatedRow^  NewSimulatedRow();
            
            protected: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            virtual ::System::Data::DataRow^  NewRowFromBuilder(::System::Data::DataRowBuilder^  builder) override;
            
            protected: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            virtual ::System::Type^  GetRowType() override;
            
            protected: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            virtual ::System::Void OnRowChanged(::System::Data::DataRowChangeEventArgs^  e) override;
            
            protected: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            virtual ::System::Void OnRowChanging(::System::Data::DataRowChangeEventArgs^  e) override;
            
            protected: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            virtual ::System::Void OnRowDeleted(::System::Data::DataRowChangeEventArgs^  e) override;
            
            protected: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            virtual ::System::Void OnRowDeleting(::System::Data::DataRowChangeEventArgs^  e) override;
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            ::System::Void RemoveSimulatedRow(Server::ServerSimulation::SimulatedRow^  row);
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            static ::System::Xml::Schema::XmlSchemaComplexType^  GetTypedTableSchema(::System::Xml::Schema::XmlSchemaSet^  xs);
        };
        
        public : /// <summary>
///Represents strongly named DataRow class.
///</summary>
        [System::CodeDom::Compiler::GeneratedCodeAttribute(L"System.Data.Design.TypedDataSetGenerator", L"2.0.0.0")]
        ref class GroupRow : public ::System::Data::DataRow {
            
            private: Server::ServerSimulation::GroupDataTable^  tableGroup;
            
            internal: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            GroupRow(::System::Data::DataRowBuilder^  rb);
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            property System::Int64 Rate {
                System::Int64 get();
                System::Void set(System::Int64 value);
            }
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            property System::Int32 Group_Id {
                System::Int32 get();
                System::Void set(System::Int32 value);
            }
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            ::System::Boolean IsRateNull();
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            ::System::Void SetRateNull();
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            cli::array< Server::ServerSimulation::SimulatedRow^  >^  GetSimulatedRows();
        };
        
        public : /// <summary>
///Represents strongly named DataRow class.
///</summary>
        [System::CodeDom::Compiler::GeneratedCodeAttribute(L"System.Data.Design.TypedDataSetGenerator", L"2.0.0.0")]
        ref class SimulatedRow : public ::System::Data::DataRow {
            
            private: Server::ServerSimulation::SimulatedDataTable^  tableSimulated;
            
            internal: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            SimulatedRow(::System::Data::DataRowBuilder^  rb);
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            property System::String^  NodeId {
                System::String^  get();
                System::Void set(System::String^  value);
            }
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            property System::String^  SimulationType {
                System::String^  get();
                System::Void set(System::String^  value);
            }
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            property System::Double Min {
                System::Double get();
                System::Void set(System::Double value);
            }
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            property System::Double Max {
                System::Double get();
                System::Void set(System::Double value);
            }
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            property System::Int32 Group_Id {
                System::Int32 get();
                System::Void set(System::Int32 value);
            }
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            property Server::ServerSimulation::GroupRow^  GroupRow {
                Server::ServerSimulation::GroupRow^  get();
                System::Void set(Server::ServerSimulation::GroupRow^  value);
            }
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            ::System::Boolean IsNodeIdNull();
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            ::System::Void SetNodeIdNull();
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            ::System::Boolean IsSimulationTypeNull();
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            ::System::Void SetSimulationTypeNull();
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            ::System::Boolean IsMinNull();
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            ::System::Void SetMinNull();
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            ::System::Boolean IsMaxNull();
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            ::System::Void SetMaxNull();
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            ::System::Boolean IsGroup_IdNull();
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            ::System::Void SetGroup_IdNull();
        };
        
        public : /// <summary>
///Row event argument class
///</summary>
        [System::CodeDom::Compiler::GeneratedCodeAttribute(L"System.Data.Design.TypedDataSetGenerator", L"2.0.0.0")]
        ref class GroupRowChangeEvent : public ::System::EventArgs {
            
            private: Server::ServerSimulation::GroupRow^  eventRow;
            
            private: ::System::Data::DataRowAction eventAction;
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            GroupRowChangeEvent(Server::ServerSimulation::GroupRow^  row, ::System::Data::DataRowAction action);
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            property Server::ServerSimulation::GroupRow^  Row {
                Server::ServerSimulation::GroupRow^  get();
            }
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            property ::System::Data::DataRowAction Action {
                ::System::Data::DataRowAction get();
            }
        };
        
        public : /// <summary>
///Row event argument class
///</summary>
        [System::CodeDom::Compiler::GeneratedCodeAttribute(L"System.Data.Design.TypedDataSetGenerator", L"2.0.0.0")]
        ref class SimulatedRowChangeEvent : public ::System::EventArgs {
            
            private: Server::ServerSimulation::SimulatedRow^  eventRow;
            
            private: ::System::Data::DataRowAction eventAction;
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            SimulatedRowChangeEvent(Server::ServerSimulation::SimulatedRow^  row, ::System::Data::DataRowAction action);
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            property Server::ServerSimulation::SimulatedRow^  Row {
                Server::ServerSimulation::SimulatedRow^  get();
            }
            
            public: [System::Diagnostics::DebuggerNonUserCodeAttribute]
            property ::System::Data::DataRowAction Action {
                ::System::Data::DataRowAction get();
            }
        };
    };
}
namespace Server {
    
    
    inline ServerSimulation::ServerSimulation() {
        this->BeginInit();
        this->InitClass();
        ::System::ComponentModel::CollectionChangeEventHandler^  schemaChangedHandler = gcnew ::System::ComponentModel::CollectionChangeEventHandler(this, &Server::ServerSimulation::SchemaChanged);
        __super::Tables->CollectionChanged += schemaChangedHandler;
        __super::Relations->CollectionChanged += schemaChangedHandler;
        this->EndInit();
    }
    
    inline ServerSimulation::ServerSimulation(::System::Runtime::Serialization::SerializationInfo^  info, ::System::Runtime::Serialization::StreamingContext context) : 
            ::System::Data::DataSet(info, context, false) {
        if (this->IsBinarySerialized(info, context) == true) {
            this->InitVars(false);
            ::System::ComponentModel::CollectionChangeEventHandler^  schemaChangedHandler1 = gcnew ::System::ComponentModel::CollectionChangeEventHandler(this, &Server::ServerSimulation::SchemaChanged);
            this->Tables->CollectionChanged += schemaChangedHandler1;
            this->Relations->CollectionChanged += schemaChangedHandler1;
            return;
        }
        ::System::String^  strSchema = (cli::safe_cast<::System::String^  >(info->GetValue(L"XmlSchema", ::System::String::typeid)));
        if (this->DetermineSchemaSerializationMode(info, context) == ::System::Data::SchemaSerializationMode::IncludeSchema) {
            ::System::Data::DataSet^  ds = (gcnew ::System::Data::DataSet());
            ds->ReadXmlSchema((gcnew ::System::Xml::XmlTextReader((gcnew ::System::IO::StringReader(strSchema)))));
            if (ds->Tables[L"Group"] != nullptr) {
                __super::Tables->Add((gcnew Server::ServerSimulation::GroupDataTable(ds->Tables[L"Group"])));
            }
            if (ds->Tables[L"Simulated"] != nullptr) {
                __super::Tables->Add((gcnew Server::ServerSimulation::SimulatedDataTable(ds->Tables[L"Simulated"])));
            }
            this->DataSetName = ds->DataSetName;
            this->Prefix = ds->Prefix;
            this->Namespace = ds->Namespace;
            this->Locale = ds->Locale;
            this->CaseSensitive = ds->CaseSensitive;
            this->EnforceConstraints = ds->EnforceConstraints;
            this->Merge(ds, false, ::System::Data::MissingSchemaAction::Add);
            this->InitVars();
        }
        else {
            this->ReadXmlSchema((gcnew ::System::Xml::XmlTextReader((gcnew ::System::IO::StringReader(strSchema)))));
        }
        this->GetSerializationData(info, context);
        ::System::ComponentModel::CollectionChangeEventHandler^  schemaChangedHandler = gcnew ::System::ComponentModel::CollectionChangeEventHandler(this, &Server::ServerSimulation::SchemaChanged);
        __super::Tables->CollectionChanged += schemaChangedHandler;
        this->Relations->CollectionChanged += schemaChangedHandler;
    }
    
    inline Server::ServerSimulation::GroupDataTable^  ServerSimulation::Group::get() {
        return this->tableGroup;
    }
    
    inline Server::ServerSimulation::SimulatedDataTable^  ServerSimulation::Simulated::get() {
        return this->tableSimulated;
    }
    
    inline ::System::Data::SchemaSerializationMode ServerSimulation::SchemaSerializationMode::get() {
        return this->_schemaSerializationMode;
    }
    inline System::Void ServerSimulation::SchemaSerializationMode::set(::System::Data::SchemaSerializationMode value) {
        this->_schemaSerializationMode = __identifier(value);
    }
    
    inline ::System::Data::DataTableCollection^  ServerSimulation::Tables::get() {
        return __super::Tables;
    }
    
    inline ::System::Data::DataRelationCollection^  ServerSimulation::Relations::get() {
        return __super::Relations;
    }
    
    inline ::System::Void ServerSimulation::InitializeDerivedDataSet() {
        this->BeginInit();
        this->InitClass();
        this->EndInit();
    }
    
    inline ::System::Data::DataSet^  ServerSimulation::Clone() {
        Server::ServerSimulation^  cln = (cli::safe_cast<Server::ServerSimulation^  >(__super::Clone()));
        cln->InitVars();
        cln->SchemaSerializationMode = this->SchemaSerializationMode;
        return cln;
    }
    
    inline ::System::Boolean ServerSimulation::ShouldSerializeTables() {
        return false;
    }
    
    inline ::System::Boolean ServerSimulation::ShouldSerializeRelations() {
        return false;
    }
    
    inline ::System::Void ServerSimulation::ReadXmlSerializable(::System::Xml::XmlReader^  reader) {
        if (this->DetermineSchemaSerializationMode(reader) == ::System::Data::SchemaSerializationMode::IncludeSchema) {
            this->Reset();
            ::System::Data::DataSet^  ds = (gcnew ::System::Data::DataSet());
            ds->ReadXml(reader);
            if (ds->Tables[L"Group"] != nullptr) {
                __super::Tables->Add((gcnew Server::ServerSimulation::GroupDataTable(ds->Tables[L"Group"])));
            }
            if (ds->Tables[L"Simulated"] != nullptr) {
                __super::Tables->Add((gcnew Server::ServerSimulation::SimulatedDataTable(ds->Tables[L"Simulated"])));
            }
            this->DataSetName = ds->DataSetName;
            this->Prefix = ds->Prefix;
            this->Namespace = ds->Namespace;
            this->Locale = ds->Locale;
            this->CaseSensitive = ds->CaseSensitive;
            this->EnforceConstraints = ds->EnforceConstraints;
            this->Merge(ds, false, ::System::Data::MissingSchemaAction::Add);
            this->InitVars();
        }
        else {
            this->ReadXml(reader);
            this->InitVars();
        }
    }
    
    inline ::System::Xml::Schema::XmlSchema^  ServerSimulation::GetSchemaSerializable() {
        ::System::IO::MemoryStream^  stream = (gcnew ::System::IO::MemoryStream());
        this->WriteXmlSchema((gcnew ::System::Xml::XmlTextWriter(stream, nullptr)));
        stream->Position = 0;
        return ::System::Xml::Schema::XmlSchema::Read((gcnew ::System::Xml::XmlTextReader(stream)), nullptr);
    }
    
    inline ::System::Void ServerSimulation::InitVars() {
        this->InitVars(true);
    }
    
    inline ::System::Void ServerSimulation::InitVars(::System::Boolean initTable) {
        this->tableGroup = (cli::safe_cast<Server::ServerSimulation::GroupDataTable^  >(__super::Tables[L"Group"]));
        if (initTable == true) {
            if (this->tableGroup != nullptr) {
                this->tableGroup->InitVars();
            }
        }
        this->tableSimulated = (cli::safe_cast<Server::ServerSimulation::SimulatedDataTable^  >(__super::Tables[L"Simulated"]));
        if (initTable == true) {
            if (this->tableSimulated != nullptr) {
                this->tableSimulated->InitVars();
            }
        }
        this->relationGroup_Simulated = this->Relations[L"Group_Simulated"];
    }
    
    inline ::System::Void ServerSimulation::InitClass() {
        this->DataSetName = L"ServerSimulation";
        this->Prefix = L"";
        this->Namespace = L"http://tempuri.org/UASimulation.xsd";
        this->EnforceConstraints = true;
        this->SchemaSerializationMode = ::System::Data::SchemaSerializationMode::IncludeSchema;
        this->tableGroup = (gcnew Server::ServerSimulation::GroupDataTable());
        __super::Tables->Add(this->tableGroup);
        this->tableSimulated = (gcnew Server::ServerSimulation::SimulatedDataTable());
        __super::Tables->Add(this->tableSimulated);
        ::System::Data::ForeignKeyConstraint^  fkc;
        fkc = (gcnew ::System::Data::ForeignKeyConstraint(L"Group_Simulated", gcnew cli::array< ::System::Data::DataColumn^  >(1) {this->tableGroup->Group_IdColumn}, 
            gcnew cli::array< ::System::Data::DataColumn^  >(1) {this->tableSimulated->Group_IdColumn}));
        this->tableSimulated->Constraints->Add(fkc);
        fkc->AcceptRejectRule = ::System::Data::AcceptRejectRule::None;
        fkc->DeleteRule = ::System::Data::Rule::Cascade;
        fkc->UpdateRule = ::System::Data::Rule::Cascade;
        this->relationGroup_Simulated = (gcnew ::System::Data::DataRelation(L"Group_Simulated", gcnew cli::array< ::System::Data::DataColumn^  >(1) {this->tableGroup->Group_IdColumn}, 
            gcnew cli::array< ::System::Data::DataColumn^  >(1) {this->tableSimulated->Group_IdColumn}, false));
        this->relationGroup_Simulated->Nested = true;
        this->Relations->Add(this->relationGroup_Simulated);
    }
    
    inline ::System::Boolean ServerSimulation::ShouldSerializeGroup() {
        return false;
    }
    
    inline ::System::Boolean ServerSimulation::ShouldSerializeSimulated() {
        return false;
    }
    
    inline ::System::Void ServerSimulation::SchemaChanged(::System::Object^  sender, ::System::ComponentModel::CollectionChangeEventArgs^  e) {
        if (e->Action == ::System::ComponentModel::CollectionChangeAction::Remove) {
            this->InitVars();
        }
    }
    
    inline ::System::Xml::Schema::XmlSchemaComplexType^  ServerSimulation::GetTypedDataSetSchema(::System::Xml::Schema::XmlSchemaSet^  xs) {
        Server::ServerSimulation^  ds = (gcnew Server::ServerSimulation());
        ::System::Xml::Schema::XmlSchemaComplexType^  type = (gcnew ::System::Xml::Schema::XmlSchemaComplexType());
        ::System::Xml::Schema::XmlSchemaSequence^  sequence = (gcnew ::System::Xml::Schema::XmlSchemaSequence());
        ::System::Xml::Schema::XmlSchemaAny^  any = (gcnew ::System::Xml::Schema::XmlSchemaAny());
        any->Namespace = ds->Namespace;
        sequence->Items->Add(any);
        type->Particle = sequence;
        ::System::Xml::Schema::XmlSchema^  dsSchema = ds->GetSchemaSerializable();
        if (xs->Contains(dsSchema->TargetNamespace)) {
            ::System::IO::MemoryStream^  s1 = (gcnew ::System::IO::MemoryStream());
            ::System::IO::MemoryStream^  s2 = (gcnew ::System::IO::MemoryStream());
            try {
                ::System::Xml::Schema::XmlSchema^  schema = nullptr;
                dsSchema->Write(s1);
                for (                ::System::Collections::IEnumerator^  schemas = xs->Schemas(dsSchema->TargetNamespace)->GetEnumerator(); schemas->MoveNext();                 ) {
                    schema = (cli::safe_cast<::System::Xml::Schema::XmlSchema^  >(schemas->Current));
                    s2->SetLength(0);
                    schema->Write(s2);
                    if (s1->Length == s2->Length) {
                        s1->Position = 0;
                        s2->Position = 0;
                        for (                        ; ((s1->Position != s1->Length) 
                                    && (s1->ReadByte() == s2->ReadByte()));                         ) {
                            ;
                        }
                        if (s1->Position == s1->Length) {
                            return type;
                        }
                    }
                }
            }
            finally {
                if (s1 != nullptr) {
                    s1->Close();
                }
                if (s2 != nullptr) {
                    s2->Close();
                }
            }
        }
        xs->Add(dsSchema);
        return type;
    }
    
    
    inline ServerSimulation::GroupDataTable::GroupDataTable() {
        this->TableName = L"Group";
        this->BeginInit();
        this->InitClass();
        this->EndInit();
    }
    
    inline ServerSimulation::GroupDataTable::GroupDataTable(::System::Data::DataTable^  table) {
        this->TableName = table->TableName;
        if (table->CaseSensitive != table->DataSet->CaseSensitive) {
            this->CaseSensitive = table->CaseSensitive;
        }
        if (table->Locale->ToString() != table->DataSet->Locale->ToString()) {
            this->Locale = table->Locale;
        }
        if (table->Namespace != table->DataSet->Namespace) {
            this->Namespace = table->Namespace;
        }
        this->Prefix = table->Prefix;
        this->MinimumCapacity = table->MinimumCapacity;
    }
    
    inline ServerSimulation::GroupDataTable::GroupDataTable(::System::Runtime::Serialization::SerializationInfo^  info, ::System::Runtime::Serialization::StreamingContext context) : 
            ::System::Data::DataTable(info, context) {
        this->InitVars();
    }
    
    inline ::System::Data::DataColumn^  ServerSimulation::GroupDataTable::RateColumn::get() {
        return this->columnRate;
    }
    
    inline ::System::Data::DataColumn^  ServerSimulation::GroupDataTable::Group_IdColumn::get() {
        return this->columnGroup_Id;
    }
    
    inline ::System::Int32 ServerSimulation::GroupDataTable::Count::get() {
        return this->Rows->Count;
    }
    
    inline Server::ServerSimulation::GroupRow^  ServerSimulation::GroupDataTable::default::get(::System::Int32 index) {
        return (cli::safe_cast<Server::ServerSimulation::GroupRow^  >(this->Rows[index]));
    }
    
    inline ::System::Void ServerSimulation::GroupDataTable::AddGroupRow(Server::ServerSimulation::GroupRow^  row) {
        this->Rows->Add(row);
    }
    
    inline Server::ServerSimulation::GroupRow^  ServerSimulation::GroupDataTable::AddGroupRow(System::Int64 Rate) {
        Server::ServerSimulation::GroupRow^  rowGroupRow = (cli::safe_cast<Server::ServerSimulation::GroupRow^  >(this->NewRow()));
        cli::array< ::System::Object^  >^  columnValuesArray = gcnew cli::array< ::System::Object^  >(2) {Rate, nullptr};
        rowGroupRow->ItemArray = columnValuesArray;
        this->Rows->Add(rowGroupRow);
        return rowGroupRow;
    }
    
    inline ::System::Collections::IEnumerator^  ServerSimulation::GroupDataTable::GetEnumerator() {
        return this->Rows->GetEnumerator();
    }
    
    inline ::System::Data::DataTable^  ServerSimulation::GroupDataTable::Clone() {
        Server::ServerSimulation::GroupDataTable^  cln = (cli::safe_cast<Server::ServerSimulation::GroupDataTable^  >(__super::Clone()));
        cln->InitVars();
        return cln;
    }
    
    inline ::System::Data::DataTable^  ServerSimulation::GroupDataTable::CreateInstance() {
        return (gcnew Server::ServerSimulation::GroupDataTable());
    }
    
    inline ::System::Void ServerSimulation::GroupDataTable::InitVars() {
        this->columnRate = __super::Columns[L"Rate"];
        this->columnGroup_Id = __super::Columns[L"Group_Id"];
    }
    
    inline ::System::Void ServerSimulation::GroupDataTable::InitClass() {
        this->columnRate = (gcnew ::System::Data::DataColumn(L"Rate", ::System::Int64::typeid, nullptr, ::System::Data::MappingType::Attribute));
        __super::Columns->Add(this->columnRate);
        this->columnGroup_Id = (gcnew ::System::Data::DataColumn(L"Group_Id", ::System::Int32::typeid, nullptr, ::System::Data::MappingType::Hidden));
        __super::Columns->Add(this->columnGroup_Id);
        this->Constraints->Add((gcnew ::System::Data::UniqueConstraint(L"Constraint1", gcnew cli::array< ::System::Data::DataColumn^  >(1) {this->columnGroup_Id}, 
                true)));
        this->columnRate->Namespace = L"";
        this->columnGroup_Id->AutoIncrement = true;
        this->columnGroup_Id->AllowDBNull = false;
        this->columnGroup_Id->Unique = true;
    }
    
    inline Server::ServerSimulation::GroupRow^  ServerSimulation::GroupDataTable::NewGroupRow() {
        return (cli::safe_cast<Server::ServerSimulation::GroupRow^  >(this->NewRow()));
    }
    
    inline ::System::Data::DataRow^  ServerSimulation::GroupDataTable::NewRowFromBuilder(::System::Data::DataRowBuilder^  builder) {
        return (gcnew Server::ServerSimulation::GroupRow(builder));
    }
    
    inline ::System::Type^  ServerSimulation::GroupDataTable::GetRowType() {
        return Server::ServerSimulation::GroupRow::typeid;
    }
    
    inline ::System::Void ServerSimulation::GroupDataTable::OnRowChanged(::System::Data::DataRowChangeEventArgs^  e) {
        __super::OnRowChanged(e);
        {
            this->GroupRowChanged(this, (gcnew Server::ServerSimulation::GroupRowChangeEvent((cli::safe_cast<Server::ServerSimulation::GroupRow^  >(e->Row)), 
                    e->Action)));
        }
    }
    
    inline ::System::Void ServerSimulation::GroupDataTable::OnRowChanging(::System::Data::DataRowChangeEventArgs^  e) {
        __super::OnRowChanging(e);
        {
            this->GroupRowChanging(this, (gcnew Server::ServerSimulation::GroupRowChangeEvent((cli::safe_cast<Server::ServerSimulation::GroupRow^  >(e->Row)), 
                    e->Action)));
        }
    }
    
    inline ::System::Void ServerSimulation::GroupDataTable::OnRowDeleted(::System::Data::DataRowChangeEventArgs^  e) {
        __super::OnRowDeleted(e);
        {
            this->GroupRowDeleted(this, (gcnew Server::ServerSimulation::GroupRowChangeEvent((cli::safe_cast<Server::ServerSimulation::GroupRow^  >(e->Row)), 
                    e->Action)));
        }
    }
    
    inline ::System::Void ServerSimulation::GroupDataTable::OnRowDeleting(::System::Data::DataRowChangeEventArgs^  e) {
        __super::OnRowDeleting(e);
        {
            this->GroupRowDeleting(this, (gcnew Server::ServerSimulation::GroupRowChangeEvent((cli::safe_cast<Server::ServerSimulation::GroupRow^  >(e->Row)), 
                    e->Action)));
        }
    }
    
    inline ::System::Void ServerSimulation::GroupDataTable::RemoveGroupRow(Server::ServerSimulation::GroupRow^  row) {
        this->Rows->Remove(row);
    }
    
    inline ::System::Xml::Schema::XmlSchemaComplexType^  ServerSimulation::GroupDataTable::GetTypedTableSchema(::System::Xml::Schema::XmlSchemaSet^  xs) {
        ::System::Xml::Schema::XmlSchemaComplexType^  type = (gcnew ::System::Xml::Schema::XmlSchemaComplexType());
        ::System::Xml::Schema::XmlSchemaSequence^  sequence = (gcnew ::System::Xml::Schema::XmlSchemaSequence());
        Server::ServerSimulation^  ds = (gcnew Server::ServerSimulation());
        ::System::Xml::Schema::XmlSchemaAny^  any1 = (gcnew ::System::Xml::Schema::XmlSchemaAny());
        any1->Namespace = L"http://www.w3.org/2001/XMLSchema";
        any1->MinOccurs = ::System::Decimal(0);
        any1->MaxOccurs = ::System::Decimal::MaxValue;
        any1->ProcessContents = ::System::Xml::Schema::XmlSchemaContentProcessing::Lax;
        sequence->Items->Add(any1);
        ::System::Xml::Schema::XmlSchemaAny^  any2 = (gcnew ::System::Xml::Schema::XmlSchemaAny());
        any2->Namespace = L"urn:schemas-microsoft-com:xml-diffgram-v1";
        any2->MinOccurs = ::System::Decimal(1);
        any2->ProcessContents = ::System::Xml::Schema::XmlSchemaContentProcessing::Lax;
        sequence->Items->Add(any2);
        ::System::Xml::Schema::XmlSchemaAttribute^  attribute1 = (gcnew ::System::Xml::Schema::XmlSchemaAttribute());
        attribute1->Name = L"namespace";
        attribute1->FixedValue = ds->Namespace;
        type->Attributes->Add(attribute1);
        ::System::Xml::Schema::XmlSchemaAttribute^  attribute2 = (gcnew ::System::Xml::Schema::XmlSchemaAttribute());
        attribute2->Name = L"tableTypeName";
        attribute2->FixedValue = L"GroupDataTable";
        type->Attributes->Add(attribute2);
        type->Particle = sequence;
        ::System::Xml::Schema::XmlSchema^  dsSchema = ds->GetSchemaSerializable();
        if (xs->Contains(dsSchema->TargetNamespace)) {
            ::System::IO::MemoryStream^  s1 = (gcnew ::System::IO::MemoryStream());
            ::System::IO::MemoryStream^  s2 = (gcnew ::System::IO::MemoryStream());
            try {
                ::System::Xml::Schema::XmlSchema^  schema = nullptr;
                dsSchema->Write(s1);
                for (                ::System::Collections::IEnumerator^  schemas = xs->Schemas(dsSchema->TargetNamespace)->GetEnumerator(); schemas->MoveNext();                 ) {
                    schema = (cli::safe_cast<::System::Xml::Schema::XmlSchema^  >(schemas->Current));
                    s2->SetLength(0);
                    schema->Write(s2);
                    if (s1->Length == s2->Length) {
                        s1->Position = 0;
                        s2->Position = 0;
                        for (                        ; ((s1->Position != s1->Length) 
                                    && (s1->ReadByte() == s2->ReadByte()));                         ) {
                            ;
                        }
                        if (s1->Position == s1->Length) {
                            return type;
                        }
                    }
                }
            }
            finally {
                if (s1 != nullptr) {
                    s1->Close();
                }
                if (s2 != nullptr) {
                    s2->Close();
                }
            }
        }
        xs->Add(dsSchema);
        return type;
    }
    
    
    inline ServerSimulation::SimulatedDataTable::SimulatedDataTable() {
        this->TableName = L"Simulated";
        this->BeginInit();
        this->InitClass();
        this->EndInit();
    }
    
    inline ServerSimulation::SimulatedDataTable::SimulatedDataTable(::System::Data::DataTable^  table) {
        this->TableName = table->TableName;
        if (table->CaseSensitive != table->DataSet->CaseSensitive) {
            this->CaseSensitive = table->CaseSensitive;
        }
        if (table->Locale->ToString() != table->DataSet->Locale->ToString()) {
            this->Locale = table->Locale;
        }
        if (table->Namespace != table->DataSet->Namespace) {
            this->Namespace = table->Namespace;
        }
        this->Prefix = table->Prefix;
        this->MinimumCapacity = table->MinimumCapacity;
    }
    
    inline ServerSimulation::SimulatedDataTable::SimulatedDataTable(::System::Runtime::Serialization::SerializationInfo^  info, 
                ::System::Runtime::Serialization::StreamingContext context) : 
            ::System::Data::DataTable(info, context) {
        this->InitVars();
    }
    
    inline ::System::Data::DataColumn^  ServerSimulation::SimulatedDataTable::NodeIdColumn::get() {
        return this->columnNodeId;
    }
    
    inline ::System::Data::DataColumn^  ServerSimulation::SimulatedDataTable::SimulationTypeColumn::get() {
        return this->columnSimulationType;
    }
    
    inline ::System::Data::DataColumn^  ServerSimulation::SimulatedDataTable::MinColumn::get() {
        return this->columnMin;
    }
    
    inline ::System::Data::DataColumn^  ServerSimulation::SimulatedDataTable::MaxColumn::get() {
        return this->columnMax;
    }
    
    inline ::System::Data::DataColumn^  ServerSimulation::SimulatedDataTable::Group_IdColumn::get() {
        return this->columnGroup_Id;
    }
    
    inline ::System::Int32 ServerSimulation::SimulatedDataTable::Count::get() {
        return this->Rows->Count;
    }
    
    inline Server::ServerSimulation::SimulatedRow^  ServerSimulation::SimulatedDataTable::default::get(::System::Int32 index) {
        return (cli::safe_cast<Server::ServerSimulation::SimulatedRow^  >(this->Rows[index]));
    }
    
    inline ::System::Void ServerSimulation::SimulatedDataTable::AddSimulatedRow(Server::ServerSimulation::SimulatedRow^  row) {
        this->Rows->Add(row);
    }
    
    inline Server::ServerSimulation::SimulatedRow^  ServerSimulation::SimulatedDataTable::AddSimulatedRow(System::String^  NodeId, 
                System::String^  SimulationType, System::Double Min, System::Double Max, Server::ServerSimulation::GroupRow^  parentGroupRowByGroup_Simulated) {
        Server::ServerSimulation::SimulatedRow^  rowSimulatedRow = (cli::safe_cast<Server::ServerSimulation::SimulatedRow^  >(this->NewRow()));
        cli::array< ::System::Object^  >^  columnValuesArray = gcnew cli::array< ::System::Object^  >(5) {NodeId, SimulationType, 
            Min, Max, nullptr};
        if (parentGroupRowByGroup_Simulated != nullptr) {
            columnValuesArray[4] = parentGroupRowByGroup_Simulated[1];
        }
        rowSimulatedRow->ItemArray = columnValuesArray;
        this->Rows->Add(rowSimulatedRow);
        return rowSimulatedRow;
    }
    
    inline ::System::Collections::IEnumerator^  ServerSimulation::SimulatedDataTable::GetEnumerator() {
        return this->Rows->GetEnumerator();
    }
    
    inline ::System::Data::DataTable^  ServerSimulation::SimulatedDataTable::Clone() {
        Server::ServerSimulation::SimulatedDataTable^  cln = (cli::safe_cast<Server::ServerSimulation::SimulatedDataTable^  >(__super::Clone()));
        cln->InitVars();
        return cln;
    }
    
    inline ::System::Data::DataTable^  ServerSimulation::SimulatedDataTable::CreateInstance() {
        return (gcnew Server::ServerSimulation::SimulatedDataTable());
    }
    
    inline ::System::Void ServerSimulation::SimulatedDataTable::InitVars() {
        this->columnNodeId = __super::Columns[L"NodeId"];
        this->columnSimulationType = __super::Columns[L"SimulationType"];
        this->columnMin = __super::Columns[L"Min"];
        this->columnMax = __super::Columns[L"Max"];
        this->columnGroup_Id = __super::Columns[L"Group_Id"];
    }
    
    inline ::System::Void ServerSimulation::SimulatedDataTable::InitClass() {
        this->columnNodeId = (gcnew ::System::Data::DataColumn(L"NodeId", ::System::String::typeid, nullptr, ::System::Data::MappingType::Attribute));
        __super::Columns->Add(this->columnNodeId);
        this->columnSimulationType = (gcnew ::System::Data::DataColumn(L"SimulationType", ::System::String::typeid, nullptr, ::System::Data::MappingType::Attribute));
        __super::Columns->Add(this->columnSimulationType);
        this->columnMin = (gcnew ::System::Data::DataColumn(L"Min", ::System::Double::typeid, nullptr, ::System::Data::MappingType::Attribute));
        __super::Columns->Add(this->columnMin);
        this->columnMax = (gcnew ::System::Data::DataColumn(L"Max", ::System::Double::typeid, nullptr, ::System::Data::MappingType::Attribute));
        __super::Columns->Add(this->columnMax);
        this->columnGroup_Id = (gcnew ::System::Data::DataColumn(L"Group_Id", ::System::Int32::typeid, nullptr, ::System::Data::MappingType::Hidden));
        __super::Columns->Add(this->columnGroup_Id);
        this->columnNodeId->Namespace = L"";
        this->columnSimulationType->Namespace = L"";
        this->columnMin->Namespace = L"";
        this->columnMax->Namespace = L"";
    }
    
    inline Server::ServerSimulation::SimulatedRow^  ServerSimulation::SimulatedDataTable::NewSimulatedRow() {
        return (cli::safe_cast<Server::ServerSimulation::SimulatedRow^  >(this->NewRow()));
    }
    
    inline ::System::Data::DataRow^  ServerSimulation::SimulatedDataTable::NewRowFromBuilder(::System::Data::DataRowBuilder^  builder) {
        return (gcnew Server::ServerSimulation::SimulatedRow(builder));
    }
    
    inline ::System::Type^  ServerSimulation::SimulatedDataTable::GetRowType() {
        return Server::ServerSimulation::SimulatedRow::typeid;
    }
    
    inline ::System::Void ServerSimulation::SimulatedDataTable::OnRowChanged(::System::Data::DataRowChangeEventArgs^  e) {
        __super::OnRowChanged(e);
        {
            this->SimulatedRowChanged(this, (gcnew Server::ServerSimulation::SimulatedRowChangeEvent((cli::safe_cast<Server::ServerSimulation::SimulatedRow^  >(e->Row)), 
                    e->Action)));
        }
    }
    
    inline ::System::Void ServerSimulation::SimulatedDataTable::OnRowChanging(::System::Data::DataRowChangeEventArgs^  e) {
        __super::OnRowChanging(e);
        {
            this->SimulatedRowChanging(this, (gcnew Server::ServerSimulation::SimulatedRowChangeEvent((cli::safe_cast<Server::ServerSimulation::SimulatedRow^  >(e->Row)), 
                    e->Action)));
        }
    }
    
    inline ::System::Void ServerSimulation::SimulatedDataTable::OnRowDeleted(::System::Data::DataRowChangeEventArgs^  e) {
        __super::OnRowDeleted(e);
        {
            this->SimulatedRowDeleted(this, (gcnew Server::ServerSimulation::SimulatedRowChangeEvent((cli::safe_cast<Server::ServerSimulation::SimulatedRow^  >(e->Row)), 
                    e->Action)));
        }
    }
    
    inline ::System::Void ServerSimulation::SimulatedDataTable::OnRowDeleting(::System::Data::DataRowChangeEventArgs^  e) {
        __super::OnRowDeleting(e);
        {
            this->SimulatedRowDeleting(this, (gcnew Server::ServerSimulation::SimulatedRowChangeEvent((cli::safe_cast<Server::ServerSimulation::SimulatedRow^  >(e->Row)), 
                    e->Action)));
        }
    }
    
    inline ::System::Void ServerSimulation::SimulatedDataTable::RemoveSimulatedRow(Server::ServerSimulation::SimulatedRow^  row) {
        this->Rows->Remove(row);
    }
    
    inline ::System::Xml::Schema::XmlSchemaComplexType^  ServerSimulation::SimulatedDataTable::GetTypedTableSchema(::System::Xml::Schema::XmlSchemaSet^  xs) {
        ::System::Xml::Schema::XmlSchemaComplexType^  type = (gcnew ::System::Xml::Schema::XmlSchemaComplexType());
        ::System::Xml::Schema::XmlSchemaSequence^  sequence = (gcnew ::System::Xml::Schema::XmlSchemaSequence());
        Server::ServerSimulation^  ds = (gcnew Server::ServerSimulation());
        ::System::Xml::Schema::XmlSchemaAny^  any1 = (gcnew ::System::Xml::Schema::XmlSchemaAny());
        any1->Namespace = L"http://www.w3.org/2001/XMLSchema";
        any1->MinOccurs = ::System::Decimal(0);
        any1->MaxOccurs = ::System::Decimal::MaxValue;
        any1->ProcessContents = ::System::Xml::Schema::XmlSchemaContentProcessing::Lax;
        sequence->Items->Add(any1);
        ::System::Xml::Schema::XmlSchemaAny^  any2 = (gcnew ::System::Xml::Schema::XmlSchemaAny());
        any2->Namespace = L"urn:schemas-microsoft-com:xml-diffgram-v1";
        any2->MinOccurs = ::System::Decimal(1);
        any2->ProcessContents = ::System::Xml::Schema::XmlSchemaContentProcessing::Lax;
        sequence->Items->Add(any2);
        ::System::Xml::Schema::XmlSchemaAttribute^  attribute1 = (gcnew ::System::Xml::Schema::XmlSchemaAttribute());
        attribute1->Name = L"namespace";
        attribute1->FixedValue = ds->Namespace;
        type->Attributes->Add(attribute1);
        ::System::Xml::Schema::XmlSchemaAttribute^  attribute2 = (gcnew ::System::Xml::Schema::XmlSchemaAttribute());
        attribute2->Name = L"tableTypeName";
        attribute2->FixedValue = L"SimulatedDataTable";
        type->Attributes->Add(attribute2);
        type->Particle = sequence;
        ::System::Xml::Schema::XmlSchema^  dsSchema = ds->GetSchemaSerializable();
        if (xs->Contains(dsSchema->TargetNamespace)) {
            ::System::IO::MemoryStream^  s1 = (gcnew ::System::IO::MemoryStream());
            ::System::IO::MemoryStream^  s2 = (gcnew ::System::IO::MemoryStream());
            try {
                ::System::Xml::Schema::XmlSchema^  schema = nullptr;
                dsSchema->Write(s1);
                for (                ::System::Collections::IEnumerator^  schemas = xs->Schemas(dsSchema->TargetNamespace)->GetEnumerator(); schemas->MoveNext();                 ) {
                    schema = (cli::safe_cast<::System::Xml::Schema::XmlSchema^  >(schemas->Current));
                    s2->SetLength(0);
                    schema->Write(s2);
                    if (s1->Length == s2->Length) {
                        s1->Position = 0;
                        s2->Position = 0;
                        for (                        ; ((s1->Position != s1->Length) 
                                    && (s1->ReadByte() == s2->ReadByte()));                         ) {
                            ;
                        }
                        if (s1->Position == s1->Length) {
                            return type;
                        }
                    }
                }
            }
            finally {
                if (s1 != nullptr) {
                    s1->Close();
                }
                if (s2 != nullptr) {
                    s2->Close();
                }
            }
        }
        xs->Add(dsSchema);
        return type;
    }
    
    
    inline ServerSimulation::GroupRow::GroupRow(::System::Data::DataRowBuilder^  rb) : 
            ::System::Data::DataRow(rb) {
        this->tableGroup = (cli::safe_cast<Server::ServerSimulation::GroupDataTable^  >(this->Table));
    }
    
    inline System::Int64 ServerSimulation::GroupRow::Rate::get() {
        try {
            return (cli::safe_cast<::System::Int64 >(this[this->tableGroup->RateColumn]));
        }
        catch (::System::InvalidCastException^ e) {
            throw (gcnew ::System::Data::StrongTypingException(L"La valeur pour la colonne \'Rate\' dans la table \'Group\' est DBNull.", 
                e));
        }
    }
    inline System::Void ServerSimulation::GroupRow::Rate::set(System::Int64 value) {
        this[this->tableGroup->RateColumn] = value;
    }
    
    inline System::Int32 ServerSimulation::GroupRow::Group_Id::get() {
        return (cli::safe_cast<::System::Int32 >(this[this->tableGroup->Group_IdColumn]));
    }
    inline System::Void ServerSimulation::GroupRow::Group_Id::set(System::Int32 value) {
        this[this->tableGroup->Group_IdColumn] = value;
    }
    
    inline ::System::Boolean ServerSimulation::GroupRow::IsRateNull() {
        return this->IsNull(this->tableGroup->RateColumn);
    }
    
    inline ::System::Void ServerSimulation::GroupRow::SetRateNull() {
        this[this->tableGroup->RateColumn] = ::System::Convert::DBNull;
    }
    
    inline cli::array< Server::ServerSimulation::SimulatedRow^  >^  ServerSimulation::GroupRow::GetSimulatedRows() {
        if (this->Table->ChildRelations[L"Group_Simulated"] == nullptr) {
            return gcnew cli::array< Server::ServerSimulation::SimulatedRow^  >(0);
        }
        else {
            return (cli::safe_cast<cli::array< Server::ServerSimulation::SimulatedRow^  >^  >(__super::GetChildRows(this->Table->ChildRelations[L"Group_Simulated"])));
        }
    }
    
    
    inline ServerSimulation::SimulatedRow::SimulatedRow(::System::Data::DataRowBuilder^  rb) : 
            ::System::Data::DataRow(rb) {
        this->tableSimulated = (cli::safe_cast<Server::ServerSimulation::SimulatedDataTable^  >(this->Table));
    }
    
    inline System::String^  ServerSimulation::SimulatedRow::NodeId::get() {
        try {
            return (cli::safe_cast<::System::String^  >(this[this->tableSimulated->NodeIdColumn]));
        }
        catch (::System::InvalidCastException^ e) {
            throw (gcnew ::System::Data::StrongTypingException(L"La valeur pour la colonne \'NodeId\' dans la table \'Simulated\' est DBNull.", 
                e));
        }
    }
    inline System::Void ServerSimulation::SimulatedRow::NodeId::set(System::String^  value) {
        this[this->tableSimulated->NodeIdColumn] = value;
    }
    
    inline System::String^  ServerSimulation::SimulatedRow::SimulationType::get() {
        try {
            return (cli::safe_cast<::System::String^  >(this[this->tableSimulated->SimulationTypeColumn]));
        }
        catch (::System::InvalidCastException^ e) {
            throw (gcnew ::System::Data::StrongTypingException(L"La valeur pour la colonne \'SimulationType\' dans la table \'Simulated\' est DBNull.", 
                e));
        }
    }
    inline System::Void ServerSimulation::SimulatedRow::SimulationType::set(System::String^  value) {
        this[this->tableSimulated->SimulationTypeColumn] = value;
    }
    
    inline System::Double ServerSimulation::SimulatedRow::Min::get() {
        try {
            return (cli::safe_cast<::System::Double >(this[this->tableSimulated->MinColumn]));
        }
        catch (::System::InvalidCastException^ e) {
            throw (gcnew ::System::Data::StrongTypingException(L"La valeur pour la colonne \'Min\' dans la table \'Simulated\' est DBNull.", 
                e));
        }
    }
    inline System::Void ServerSimulation::SimulatedRow::Min::set(System::Double value) {
        this[this->tableSimulated->MinColumn] = value;
    }
    
    inline System::Double ServerSimulation::SimulatedRow::Max::get() {
        try {
            return (cli::safe_cast<::System::Double >(this[this->tableSimulated->MaxColumn]));
        }
        catch (::System::InvalidCastException^ e) {
            throw (gcnew ::System::Data::StrongTypingException(L"La valeur pour la colonne \'Max\' dans la table \'Simulated\' est DBNull.", 
                e));
        }
    }
    inline System::Void ServerSimulation::SimulatedRow::Max::set(System::Double value) {
        this[this->tableSimulated->MaxColumn] = value;
    }
    
    inline System::Int32 ServerSimulation::SimulatedRow::Group_Id::get() {
        try {
            return (cli::safe_cast<::System::Int32 >(this[this->tableSimulated->Group_IdColumn]));
        }
        catch (::System::InvalidCastException^ e) {
            throw (gcnew ::System::Data::StrongTypingException(L"La valeur pour la colonne \'Group_Id\' dans la table \'Simulated\' est DBNull.", 
                e));
        }
    }
    inline System::Void ServerSimulation::SimulatedRow::Group_Id::set(System::Int32 value) {
        this[this->tableSimulated->Group_IdColumn] = value;
    }
    
    inline Server::ServerSimulation::GroupRow^  ServerSimulation::SimulatedRow::GroupRow::get() {
        return (cli::safe_cast<Server::ServerSimulation::GroupRow^  >(this->GetParentRow(this->Table->ParentRelations[L"Group_Simulated"])));
    }
    inline System::Void ServerSimulation::SimulatedRow::GroupRow::set(Server::ServerSimulation::GroupRow^  value) {
        this->SetParentRow(value, this->Table->ParentRelations[L"Group_Simulated"]);
    }
    
    inline ::System::Boolean ServerSimulation::SimulatedRow::IsNodeIdNull() {
        return this->IsNull(this->tableSimulated->NodeIdColumn);
    }
    
    inline ::System::Void ServerSimulation::SimulatedRow::SetNodeIdNull() {
        this[this->tableSimulated->NodeIdColumn] = ::System::Convert::DBNull;
    }
    
    inline ::System::Boolean ServerSimulation::SimulatedRow::IsSimulationTypeNull() {
        return this->IsNull(this->tableSimulated->SimulationTypeColumn);
    }
    
    inline ::System::Void ServerSimulation::SimulatedRow::SetSimulationTypeNull() {
        this[this->tableSimulated->SimulationTypeColumn] = ::System::Convert::DBNull;
    }
    
    inline ::System::Boolean ServerSimulation::SimulatedRow::IsMinNull() {
        return this->IsNull(this->tableSimulated->MinColumn);
    }
    
    inline ::System::Void ServerSimulation::SimulatedRow::SetMinNull() {
        this[this->tableSimulated->MinColumn] = ::System::Convert::DBNull;
    }
    
    inline ::System::Boolean ServerSimulation::SimulatedRow::IsMaxNull() {
        return this->IsNull(this->tableSimulated->MaxColumn);
    }
    
    inline ::System::Void ServerSimulation::SimulatedRow::SetMaxNull() {
        this[this->tableSimulated->MaxColumn] = ::System::Convert::DBNull;
    }
    
    inline ::System::Boolean ServerSimulation::SimulatedRow::IsGroup_IdNull() {
        return this->IsNull(this->tableSimulated->Group_IdColumn);
    }
    
    inline ::System::Void ServerSimulation::SimulatedRow::SetGroup_IdNull() {
        this[this->tableSimulated->Group_IdColumn] = ::System::Convert::DBNull;
    }
    
    
    inline ServerSimulation::GroupRowChangeEvent::GroupRowChangeEvent(Server::ServerSimulation::GroupRow^  row, ::System::Data::DataRowAction action) {
        this->eventRow = row;
        this->eventAction = action;
    }
    
    inline Server::ServerSimulation::GroupRow^  ServerSimulation::GroupRowChangeEvent::Row::get() {
        return this->eventRow;
    }
    
    inline ::System::Data::DataRowAction ServerSimulation::GroupRowChangeEvent::Action::get() {
        return this->eventAction;
    }
    
    
    inline ServerSimulation::SimulatedRowChangeEvent::SimulatedRowChangeEvent(Server::ServerSimulation::SimulatedRow^  row, 
                ::System::Data::DataRowAction action) {
        this->eventRow = row;
        this->eventAction = action;
    }
    
    inline Server::ServerSimulation::SimulatedRow^  ServerSimulation::SimulatedRowChangeEvent::Row::get() {
        return this->eventRow;
    }
    
    inline ::System::Data::DataRowAction ServerSimulation::SimulatedRowChangeEvent::Action::get() {
        return this->eventAction;
    }
}
