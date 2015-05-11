
#pragma once
#include "CmdlModelFactory.h"
#include "yaml.h"

namespace LvEdEngine
{
	class Model3dBuilder;

	//--------------------------------------------------
	class EntityModelFactory : public CmdlModelFactory
	{
	public:
		EntityModelFactory(ID3D11Device* device) : CmdlModelFactory(device) {}
		virtual ~EntityModelFactory() {}

		virtual bool LoadResource( Resource* resource, const WCHAR * filename );

		void parseYaml( FILE* fh );

		void yamlStreamStartToken( const yaml_token_t& token );
		void yamlStreamEndToken( const yaml_token_t& token );
		void yamlKeyToken( const yaml_token_t& token );
		void yamlValueToken( const yaml_token_t& token );
		void yamlBlockSequenceStartToken( const yaml_token_t& token );
		void yamlBlockEntryToken( const yaml_token_t& token );
		void yamlBlockEndToken( const yaml_token_t& token );
		void yamlFlowSequenceStartToken( const yaml_token_t& token );
		void yamlFlowSequenceEndToken( const yaml_token_t& token );
		void yamlScalarToken( const yaml_token_t& token );
		void yamlBlockMappingStartToken( const yaml_token_t& token );

		void yamlStreamStartEvent( const yaml_event_t& event );
		void yamlStreamEndEvent( const yaml_event_t& event );
		void yamlDocumentStartEvent( const yaml_event_t& event );
		void yamlDocumentEndEvent( const yaml_event_t& event );
		void yamlSequenceStartEvent( const yaml_event_t& event );
		void yamlSequenceEndEvent( const yaml_event_t& event );
		void yamlMappingStartEvent( const yaml_event_t& event );
		void yamlMappingEndEvent( const yaml_event_t& event );
		void yamlAliasEvent( const yaml_event_t& event );
		void yamlScalarEvent( const yaml_event_t& event );
	private:
		int m_Depth;
	};
};
