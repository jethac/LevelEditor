#ifndef NOMINMAX
#define NOMINMAX
#endif

#define ENABLE_PRINT_DEBUG 0

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

void printMBString( const char* s ) {
	WCHAR hogera[MAX_PATH];
	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
						 (char*)s, -1, hogera, MAX_PATH );
	OutputDebugString( hogera );
}

namespace LvEdEngine
{

bool EntityModelFactory::LoadResource( Resource* resource, const WCHAR * filename )
{
	char envUsagiDir[MAX_PATH];
	GetEnvironmentVariableA( "USAGI_DIR", envUsagiDir, (DWORD)strlen( envUsagiDir ) );

	char filename_mb[MAX_PATH];
	WideCharToMultiByte( 0, 0, filename, -1, filename_mb, MAX_PATH, NULL, NULL );

	// Parse a YAML Entity file
	std::string modelName;
	std::string yamlPath( filename_mb );
	do {
		EntityYaml yamlParser;
		yamlParser.parseYaml( yamlPath.c_str() );

		// No interitance anymore
		if( yamlParser.mInherits.empty() ) {
			break;
		}
		else {
			modelName = yamlParser.mModelName;
			yamlPath.assign( envUsagiDir );
			yamlPath.append( "\\Data\\Entities\\" );
			yamlPath.append( yamlParser.mInherits );
			yamlPath.append( ".yml" );
		}
	} while ( modelName.empty() );

	size_t len = modelName.length();
	if( len == 0 ) {
		return false; // No model found
	}

	// Change an extention "vmdc" to "cmdl"
	std::string::size_type dotPos = modelName.find_last_of( "." );
	assert( dotPos != std::string::npos );
	modelName = modelName.substr( 0, dotPos ) + std::string( ".cmdl" );
	replace( modelName, "/", "\\" );

	// Make a path full
	std::string modelPath( envUsagiDir );
	modelPath.append( "\\Data\\Models\\" );
	modelPath.append( modelName );
	WCHAR modelPathW[MAX_PATH];
	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
						 modelPath.c_str(), -1, modelPathW, MAX_PATH );

	Model * model = (Model*)resource;
	return LoadModel( model, modelPathW );
}

bool EntityModelFactory::LoadModel( Model* model, const WCHAR* filepath )
{
	model->SetSourceFileName( filepath );

	Model3dBuilder builder;
	builder.m_model = model;

	pugi::xml_document doc;
	if( !doc.load_file( filepath ) ) {
		return false;
	}

	bool isSucceeded = false;
	m_parseErrors = 0;

	builder.Begin();
	ProcessXml( &builder, doc );
	builder.End();

	if( m_parseErrors > 0 ) {
		Logger::Log( OutputMessageType::Error, L"%d errors occured while parsing, '%s'\n", m_parseErrors, filepath );
	}
	else {
		isSucceeded = true;
		model->Construct( m_device, ResourceManager::Inst() );
	}

	if( !isSucceeded ) {
		model->Destroy();
	}

	return isSucceeded;
}

void EntityModelFactory::EntityYaml::parseYaml( const char* filename )
{
	FILE *fh = fopen( filename, "r" );
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
	printMBString( (char*)event.data.alias.anchor );
	printDebugString( L"\n" );
}

void EntityModelFactory::EntityYaml::yamlScalarEvent( const yaml_event_t& event )
{
	outputSpaces( mDepth );
	//printDebugString( "yamlScalarEvent\n" );
	printDebugString( L"scalar:" );

	const char* const value = (char*)event.data.scalar.value;
	printMBString( value );

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

void EntityModelFactory::EntityYaml::printMBString( const char* s ) {
#if ENABLE_PRINT_DEBUG == 1
	printMBString( s );
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
		mModelName.assign( value );
	}
	else if( mBlockKeyStack.top().compare( "Inherits" ) == 0 ) {
		mInherits.assign( value );
	}
}

}; // namespace LvEdEngine
