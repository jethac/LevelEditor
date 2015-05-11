#ifndef NOMINMAX
#define NOMINMAX
#endif

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
	FILE *fh = _wfopen( filename, L"r" );
	assert( fh != NULL );

	m_Depth = 0;
	parseYaml( fh );

	return false;
}

void EntityModelFactory::parseYaml( FILE* fh )
{
	yaml_parser_t parser;

	// Initialize parser
	int result = yaml_parser_initialize( &parser );
	assert( result );

	// Set input file
	yaml_parser_set_input_file( &parser, fh );

#if 1
	yaml_event_t event;
	do {
		result = yaml_parser_parse( &parser, &event );
		assert( result );

		switch( event.type ) {
			case YAML_NO_EVENT: break;
				/* Stream start/end */
			case YAML_STREAM_START_EVENT: yamlStreamStartEvent( event ); break;
			case YAML_STREAM_END_EVENT: yamlStreamEndEvent( event );     break;
				/* Block delimeters */
			case YAML_DOCUMENT_START_EVENT: yamlDocumentStartEvent( event ); break;
			case YAML_DOCUMENT_END_EVENT: yamlDocumentEndEvent( event );     break;
			case YAML_SEQUENCE_START_EVENT: yamlSequenceStartEvent( event ); break;
			case YAML_SEQUENCE_END_EVENT: yamlSequenceEndEvent( event );     break;
			case YAML_MAPPING_START_EVENT: yamlMappingStartEvent( event );   break;
			case YAML_MAPPING_END_EVENT: yamlMappingEndEvent( event );       break;
				/* Data */
			case YAML_ALIAS_EVENT: yamlAliasEvent( event );  /*wprintf( L"Got alias (anchor %s)\n", event.data.alias.anchor );*/ break;
			case YAML_SCALAR_EVENT: yamlScalarEvent( event ); /*wprintf( L"Got scalar (value %s)\n", event.data.scalar.value );*/ break;
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

void outputSpaces( int num ) {
	for( int i = 0; i < num; i++ ) {
		OutputDebugString( L"  " );
	}
}

void EntityModelFactory::yamlStreamStartToken( const yaml_token_t& token )
{
	outputSpaces( m_Depth );
	OutputDebugString( L"yamlStreamStartToken\n" );
	++m_Depth;
}

void EntityModelFactory::yamlStreamEndToken( const yaml_token_t& token )
{
	--m_Depth;
	outputSpaces( m_Depth );
	OutputDebugString( L"yamlStreamEndToken\n" );
}

void EntityModelFactory::yamlKeyToken( const yaml_token_t& token )
{
	outputSpaces( m_Depth );
	OutputDebugString( L"yamlKeyToken - " );
}

void EntityModelFactory::yamlValueToken( const yaml_token_t& token )
{
	outputSpaces( m_Depth );
	OutputDebugString( L"yamlValueToken - " );
}

void EntityModelFactory::yamlBlockSequenceStartToken( const yaml_token_t& token )
{
	outputSpaces( m_Depth );
	OutputDebugString( L"yamlBlockSequenceStartToken\n" );
	++m_Depth;
}

void EntityModelFactory::yamlBlockEntryToken( const yaml_token_t& token )
{
	outputSpaces( m_Depth );
	OutputDebugString( L"yamlBlockEntryToken\n" );
}

void EntityModelFactory::yamlBlockEndToken( const yaml_token_t& token )
{
	--m_Depth;
	outputSpaces( m_Depth );
	OutputDebugString( L"yamlBlockEndToken\n" );
}

void EntityModelFactory::yamlFlowSequenceStartToken( const yaml_token_t& token )
{
	outputSpaces( m_Depth );
	OutputDebugString( L"yamlFlowSequenceStartToken\n" );
	++m_Depth;

}

void EntityModelFactory::yamlFlowSequenceEndToken( const yaml_token_t& token )
{
	--m_Depth;
	outputSpaces( m_Depth );
	OutputDebugString( L"yamlFlowSequenceEndToken\n" );
}

void EntityModelFactory::yamlBlockMappingStartToken( const yaml_token_t& token )
{
	outputSpaces( m_Depth );
	OutputDebugString( L"[Block mapping]\n" );
	++m_Depth;
}

void EntityModelFactory::yamlScalarToken( const yaml_token_t& token )
{
	OutputDebugString( L"scalar:" );
	WCHAR hogera[MAX_PATH];
	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
						 (char*)token.data.scalar.value, -1, hogera, wcslen( hogera ) );
	OutputDebugString( hogera );
	OutputDebugString( L"\n" );
}

void EntityModelFactory::yamlStreamStartEvent( const yaml_event_t& event )
{
	throw std::logic_error( "The method or operation is not implemented." );
}

void EntityModelFactory::yamlStreamEndEvent( const yaml_event_t& event )
{
	throw std::logic_error( "The method or operation is not implemented." );
}

void EntityModelFactory::yamlDocumentStartEvent( const yaml_event_t& event )
{
	throw std::logic_error( "The method or operation is not implemented." );
}

void EntityModelFactory::yamlDocumentEndEvent( const yaml_event_t& event )
{
	throw std::logic_error( "The method or operation is not implemented." );
}

void EntityModelFactory::yamlSequenceStartEvent( const yaml_event_t& event )
{
	throw std::logic_error( "The method or operation is not implemented." );
}

void EntityModelFactory::yamlSequenceEndEvent( const yaml_event_t& event )
{
	throw std::logic_error( "The method or operation is not implemented." );
}

void EntityModelFactory::yamlMappingStartEvent( const yaml_event_t& event )
{
	throw std::logic_error( "The method or operation is not implemented." );
}

void EntityModelFactory::yamlMappingEndEvent( const yaml_event_t& event )
{
	throw std::logic_error( "The method or operation is not implemented." );
}

void EntityModelFactory::yamlAliasEvent( const yaml_event_t& event )
{
	throw std::logic_error( "The method or operation is not implemented." );
}

void EntityModelFactory::yamlScalarEvent( const yaml_event_t& event )
{
	throw std::logic_error( "The method or operation is not implemented." );
}

}; // namespace LvEdEngine


