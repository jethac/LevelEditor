
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
		EntityModelFactory(ID3D11Device* device) : CmdlModelFactory(device) {}
		virtual ~EntityModelFactory() {}

		virtual bool LoadResource( Resource* resource, const WCHAR * filename );
		bool LoadModel( Model* model, const WCHAR* filepath );

	private:
		class EntityYaml
		{
		public:
			typedef std::stack<yaml_event_type_t> EventStack;
			typedef std::stack<std::string> KeyStack;

			EntityYaml() {
				mDepth = 0;

				mStreamDepth = 0;
				mDocumentDepth = 0;
				mMappingDepth = 0;
				mSequenceDepth = 0;
			}
			~EntityYaml() {}

			void parseYaml( const char* filename );
			void parseYaml( FILE* fh );

			std::string mModelName;
			std::string mInherits;
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

			void outputSpaces( int num );
			void printMBString( const char* s );
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
