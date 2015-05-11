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
#include "CmdlModelFactory.h"

#include "StringUtil.h"
#include "LoaderCommon.h"
#include "CmdlModel.h"

namespace LvEdEngine
{
CmdlModelFactory::CmdlModelFactory(ID3D11Device* device) : m_device(device)
{
}

Resource* CmdlModelFactory::CreateResource(Resource* def)
{
    UNREFERENCED_VARIABLE(def);
    return new Model();
}

bool CmdlModelFactory::LoadResource( Resource* resource, const WCHAR * filename )
{
	Model * model = (Model*)resource;
	model->SetSourceFileName( filename );

	// char name for logging 'char*' exceptions
	char charName[MAX_PATH];
	WideCharToMultiByte( 0, 0, filename, -1, charName, MAX_PATH, NULL, NULL );

	Model3dBuilder builder;
	builder.m_model = model;

	pugi::xml_document doc;
	if( !doc.load_file( filename ) ) {
		return false;
	}

	bool succeeded = false;
	m_parseErrors = 0;

	builder.Begin();
	ProcessXml( &builder, doc );
	builder.End();
	
	if( m_parseErrors > 0 ) {
		Logger::Log( OutputMessageType::Error, L"%d errors occured while parsing, '%s'\n", m_parseErrors, filename );
	}
	else {
		succeeded = true;
		model->Construct( m_device, ResourceManager::Inst() );
	}

	if( !succeeded ) {
		model->Destroy();
	}

	return succeeded;
}

void CmdlModelFactory::ParseError( const char * fmt, ... )
{
	va_list args;
	va_start( args, fmt );
	Logger::LogVA( OutputMessageType::Error, fmt, args );
	va_end( args );
	m_parseErrors++;
}

}; // namespace LvEdEngine


