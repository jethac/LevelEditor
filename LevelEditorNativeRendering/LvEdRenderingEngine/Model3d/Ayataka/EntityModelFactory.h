
#pragma once
#include <stack>
#include <string>

#include "CmdlModelFactory.h"
#include "yaml.h"

namespace LvEdEngine
{
	class Model3dBuilder;

	//--------------------------------------------------
	class EntityModelFactory : public CmdlModelFactory
	{
	public:
		typedef std::stack<yaml_event_type_t> EventStack;
		typedef std::stack<std::string> KeyStack;
		EntityModelFactory(ID3D11Device* device) : CmdlModelFactory(device) {}
		virtual ~EntityModelFactory() {}

		virtual bool LoadResource( Resource* resource, const WCHAR * filename );

		class EntityYaml
		{
		public:
			EntityYaml() {
				mDepth = 0;

				mStreamDepth = 0;
				mDocumentDepth = 0;
				mMappingDepth = 0;
				mSequenceDepth = 0;
			}
			~EntityYaml() {}

			void parseYaml( const WCHAR* filename );
			void parseYaml( FILE* fh );


		private:
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

			void outputSpaces( int num );
			void outputMBChar( const char* s );
			void printDebugString( WCHAR* s );

			void catchValue( const char* value );

			int mDepth;
			int mStreamDepth, mDocumentDepth, mMappingDepth, mSequenceDepth;

			std::string mCurrentKey; // Current key string
			EventStack mEventStack_StartEnd; // Stack of Start/End events
			KeyStack mBlockKeyStack; // Stack of Mapping/Sequence block names
		};
	};
};
