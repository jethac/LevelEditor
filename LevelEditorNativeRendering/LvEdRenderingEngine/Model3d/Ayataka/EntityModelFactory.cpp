#ifndef NOMINMAX
#define NOMINMAX
#endif

#define ENABLE_PRINT_DEBUG 1

#include "../../VectorMath/V3dMath.h"
#include "../../VectorMath/CollisionPrimitives.h"
#include "../../Renderer/Model.h"
#include "../../Renderer/CustomDataAttribute.h"
#include "../../Core/Utils.h"
#include "../../Core/Logger.h"
#include "../../Core/FileUtils.h"
#include "../../ResourceManager/ResourceManager.h"
#include "../Model3dBuilder.h"
#include "../rapidxmlhelpers.h"
#include "EntityModelFactory.h"

#include "StringUtil.h"
#include "LoaderCommon.h"
#include "CmdlModel.h"

namespace LvEdEngine
{

bool EntityModelFactory::LoadResource( Resource* resource, const WCHAR * filename )
{
	WCHAR envUsagiDir[MAX_PATH];
	GetEnvironmentVariable( L"USAGI_DIR", envUsagiDir, (DWORD)wcslen( envUsagiDir ) );

	// Parse a YAML Entity file
	EntityYaml yamlParser;
	yamlParser.parseYaml( filename );

	return false;
}

void EntityModelFactory::EntityYaml::parseYaml( const WCHAR* filename )
{
	FILE *fh = _wfopen( filename, L"r" );
	assert( fh != NULL );
	parseYaml( fh );
}

void EntityModelFactory::EntityYaml::parseYaml( FILE* fh )
{
	yaml_parser_t parser;

	// Initialize parser
	int result = yaml_parser_initialize( &parser );
	assert( result );

	// Set input file
	yaml_parser_set_input_file( &parser, fh );

	mDepth = 0;
	mStreamDepth = -1;
	mDocumentDepth = -1;
	mMappingDepth = -1;
	mSequenceDepth = -1;

	mCurrentKey.clear();
	mCurrentKey.reserve( MAX_PATH );
	assert( mEventStack_StartEnd.empty() );
#if 1
	yaml_event_t event;
	do {
		result = yaml_parser_parse( &parser, &event );
		assert( result );

		switch( event.type ) {
			case YAML_NO_EVENT: break;
				/* Stream start/end */
			case YAML_STREAM_START_EVENT:
				++mStreamDepth;
				yamlStreamStartEvent( event );

				mEventStack_StartEnd.push( event.type );
				break;
			case YAML_STREAM_END_EVENT:
				yamlStreamEndEvent( event );
				--mStreamDepth;

				mEventStack_StartEnd.pop();
				break;
				/* Block delimeters */
			case YAML_DOCUMENT_START_EVENT:
				++mDocumentDepth;
				yamlDocumentStartEvent( event );

				mEventStack_StartEnd.push( event.type );
				break;
			case YAML_DOCUMENT_END_EVENT:
				yamlDocumentEndEvent( event );
				--mStreamDepth;

				mEventStack_StartEnd.pop();
				break;
			case YAML_SEQUENCE_START_EVENT:
				++mStreamDepth;
				yamlSequenceStartEvent( event );

				mEventStack_StartEnd.push( event.type );
				break;
			case YAML_SEQUENCE_END_EVENT:
				yamlSequenceEndEvent( event );
				--mSequenceDepth;
				
				mEventStack_StartEnd.pop();
				break;
			case YAML_MAPPING_START_EVENT:
				++mMappingDepth;
				yamlMappingStartEvent( event );

				mEventStack_StartEnd.push( event.type );
				break;
			case YAML_MAPPING_END_EVENT:
				yamlMappingEndEvent( event );
				--mMappingDepth;
				
				mEventStack_StartEnd.pop();
				break;
				/* Data */
			case YAML_ALIAS_EVENT: yamlAliasEvent( event );   break;
			case YAML_SCALAR_EVENT: yamlScalarEvent( event ); break;
		}
		if( event.type != YAML_STREAM_END_EVENT )
			yaml_event_delete( &event );
	} while( event.type != YAML_STREAM_END_EVENT );
	yaml_event_delete( &event );
#else
	yaml_token_t token;
	do {
		yaml_parser_scan( &parser, &token );
		switch( token.type ) {
			/* Stream start/end */
			case YAML_STREAM_START_TOKEN: yamlStreamStartToken( token ); break;
			case YAML_STREAM_END_TOKEN:   yamlStreamEndToken( token );   break;
				/* Token types (read before actual token) */
			case YAML_KEY_TOKEN:   yamlKeyToken( token );   break;
			case YAML_VALUE_TOKEN: yamlValueToken( token ); break;
				/* Block delimeters */
			case YAML_BLOCK_SEQUENCE_START_TOKEN: yamlBlockSequenceStartToken( token ); break;
			case YAML_BLOCK_ENTRY_TOKEN:          yamlBlockEntryToken( token );         break;
			case YAML_BLOCK_END_TOKEN:            yamlBlockEndToken( token );           break;
			case YAML_FLOW_SEQUENCE_START_TOKEN:  yamlFlowSequenceStartToken( token );  break;
			case YAML_FLOW_SEQUENCE_END_TOKEN:    yamlFlowSequenceEndToken( token );    break;
				/* Data */
			case YAML_BLOCK_MAPPING_START_TOKEN:  yamlBlockMappingStartToken( token ); break;
			case YAML_SCALAR_TOKEN:  yamlScalarToken( token ); break;
				/* Others */
			default:
				printf( "Got token of type %d\n", token.type );
		}
		if( token.type != YAML_STREAM_END_TOKEN )
			yaml_token_delete( &token );
	} while( token.type != YAML_STREAM_END_TOKEN );
	yaml_token_delete( &token );
#endif
	// Cleanup
	yaml_parser_delete( &parser );
	fclose( fh );
}

void EntityModelFactory::EntityYaml::yamlStreamStartEvent( const yaml_event_t& event )
{
	outputSpaces( mDepth );
	printDebugString( L"yamlStreamStartEvent\n" );
	++mDepth;
}

void EntityModelFactory::EntityYaml::yamlStreamEndEvent( const yaml_event_t& event )
{
	--mDepth;
	outputSpaces( mDepth );
	printDebugString( L"yamlStreamEndEvent\n" );
}

void EntityModelFactory::EntityYaml::yamlDocumentStartEvent( const yaml_event_t& event )
{
	outputSpaces( mDepth );
	printDebugString( L"yamlDocumentStartEvent\n" );
	++mDepth;
}

void EntityModelFactory::EntityYaml::yamlDocumentEndEvent( const yaml_event_t& event )
{
	--mDepth;
	outputSpaces( mDepth );
	printDebugString( L"yamlDocumentEndEvent\n" );
}

void EntityModelFactory::EntityYaml::yamlSequenceStartEvent( const yaml_event_t& event )
{
	// Push the block key as a sequence starts. (sometimes blank)
	mBlockKeyStack.push( mCurrentKey );
	mCurrentKey.clear();

	outputSpaces( mDepth );
	printDebugString( L"yamlSequenceStartEvent\n" );
	++mDepth;
}

void EntityModelFactory::EntityYaml::yamlSequenceEndEvent( const yaml_event_t& event )
{
	// Pop block key
	mBlockKeyStack.pop();

	--mDepth;
	outputSpaces( mDepth );
	printDebugString( L"yamlSequenceEndEvent\n" );
}

void EntityModelFactory::EntityYaml::yamlMappingStartEvent( const yaml_event_t& event )
{
	// Push the block key as a mapping starts. (sometimes blank)
	mBlockKeyStack.push( mCurrentKey );
	mCurrentKey.clear();

	outputSpaces( mDepth );
	printDebugString( L"yamlMappingStartEvent\n" );
	++mDepth;
}

void EntityModelFactory::EntityYaml::yamlMappingEndEvent( const yaml_event_t& event )
{
	// Pop block key
	mBlockKeyStack.pop();

	--mDepth;
	outputSpaces( mDepth );
	printDebugString( L"yamlMappingEndEvent\n" );
}

void EntityModelFactory::EntityYaml::yamlAliasEvent( const yaml_event_t& event )
{
	outputSpaces( mDepth );
	printDebugString( L"event:" );
	outputMBChar( (char*)event.data.alias.anchor );
	printDebugString( L"\n" );
}

void EntityModelFactory::EntityYaml::yamlScalarEvent( const yaml_event_t& event )
{
	outputSpaces( mDepth );
	//printDebugString( "yamlScalarEvent\n" );
	printDebugString( L"scalar:" );

	const char* const value = (char*)event.data.scalar.value;
	outputMBChar( value );

	if( mEventStack_StartEnd.top() == YAML_SEQUENCE_START_EVENT ) {
		catchValue( value );
		printDebugString( L" !This is VALUE!" );
		mCurrentKey.clear();
	}
	else if( mEventStack_StartEnd.top() == YAML_MAPPING_START_EVENT ) {
		if( mCurrentKey.empty() ) {
			mCurrentKey.assign( value );
			printDebugString( L" !This is KEY!" );
		}
		else {
			catchValue( value );
			printDebugString( L" !This is VALUE!" );
			mCurrentKey.clear();
		}
	}

	printDebugString( L"\n" );
}

void EntityModelFactory::EntityYaml::outputSpaces( int num ) {
#if ENABLE_PRINT_DEBUG == 1
	for( int i = 0; i < num; i++ ) {
		printDebugString( L"  " );
	}
#endif
}

void EntityModelFactory::EntityYaml::outputMBChar( const char* s ) {
#if ENABLE_PRINT_DEBUG == 1
	WCHAR hogera[MAX_PATH];
	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
						 (char*)s, -1, hogera, (int)wcslen( hogera ) );
	printDebugString( hogera );
#endif
}

void EntityModelFactory::EntityYaml::printDebugString( WCHAR* s )
{
#if ENABLE_PRINT_DEBUG == 1
	OutputDebugString( s );
#endif
}

void EntityModelFactory::EntityYaml::catchValue( const char* value )
{
	// Process Key and Value data
	if( mBlockKeyStack.top().compare( "ModelComponent" ) == 0 && mCurrentKey.compare( "m_name" ) == 0 ) {
		printDebugString( L" !Model name!" );
	}
}

#if 0
void EntityModelFactory::EntityYaml::yamlStreamStartToken( const yaml_token_t& token )
{
	outputSpaces( m_Depth );
	printDebugString( L"yamlStreamStartToken\n" );
	++m_Depth;
}

void EntityModelFactory::EntityYaml::yamlStreamEndToken( const yaml_token_t& token )
{
	--m_Depth;
	outputSpaces( m_Depth );
	printDebugString( L"yamlStreamEndToken\n" );
}

void EntityModelFactory::EntityYaml::yamlKeyToken( const yaml_token_t& token )
{
	outputSpaces( m_Depth );
	printDebugString( L"yamlKeyToken - " );
}

void EntityModelFactory::EntityYaml::yamlValueToken( const yaml_token_t& token )
{
	outputSpaces( m_Depth );
	printDebugString( L"yamlValueToken - " );
}

void EntityModelFactory::EntityYaml::yamlBlockSequenceStartToken( const yaml_token_t& token )
{
	outputSpaces( m_Depth );
	printDebugString( L"yamlBlockSequenceStartToken\n" );
	++m_Depth;
}

void EntityModelFactory::EntityYaml::yamlBlockEntryToken( const yaml_token_t& token )
{
	outputSpaces( m_Depth );
	printDebugString( L"yamlBlockEntryToken\n" );
}

void EntityModelFactory::EntityYaml::yamlBlockEndToken( const yaml_token_t& token )
{
	--m_Depth;
	outputSpaces( m_Depth );
	printDebugString( L"yamlBlockEndToken\n" );
}

void EntityModelFactory::EntityYaml::yamlFlowSequenceStartToken( const yaml_token_t& token )
{
	outputSpaces( m_Depth );
	printDebugString( L"yamlFlowSequenceStartToken\n" );
	++m_Depth;

}

void EntityModelFactory::EntityYaml::yamlFlowSequenceEndToken( const yaml_token_t& token )
{
	--m_Depth;
	outputSpaces( m_Depth );
	printDebugString( L"yamlFlowSequenceEndToken\n" );
}

void EntityModelFactory::EntityYaml::yamlBlockMappingStartToken( const yaml_token_t& token )
{
	outputSpaces( m_Depth );
	printDebugString( L"[Block mapping]\n" );
	++m_Depth;
}

void EntityModelFactory::EntityYaml::yamlScalarToken( const yaml_token_t& token )
{
	printDebugString( L"scalar:" );
	outputMBChar( (char*)token.data.scalar.value );
	printDebugString( L"\n" );
}
#endif
}; // namespace LvEdEngine
