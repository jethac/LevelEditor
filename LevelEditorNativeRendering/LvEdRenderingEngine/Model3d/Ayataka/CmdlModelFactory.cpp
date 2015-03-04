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

using namespace pugi;

inline aya::StringVector splitIndexDeclToIndex( aya::string target )
{
	CharVector splitters;
	splitters.push_back( '[' );
	splitters.push_back( ']' );

	return split( target, splitters );
}

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
	//UINT dataSize;
	//BYTE* data = FileUtils::LoadFile( filename, &dataSize );
	//if( !data ) {
	//	return false;
	//}

	Model * model = (Model*)resource;
	model->SetSourceFileName( filename );

	// char name for logging 'char*' exceptions
	char charName[MAX_PATH];
	WideCharToMultiByte( 0, 0, filename, -1, charName, MAX_PATH, NULL, NULL );

	bool succeeded = true;

	Model3dBuilder builder;
	builder.m_model = model;

	
	pugi::xml_document doc;
	if( !doc.load_file( filename ) ) {
		return false;
	}

	builder.Begin();

	ProcessXml( &builder, doc );

	builder.End();
	
	model->Construct( m_device, ResourceManager::Inst() );
	
#if 0
	xml_document doc;

	try {
		doc.parse<0>( (char*)data );

		m_parseErrors = 0;

		builder.Begin();

		ProcessXml( doc.first_node(), &builder );

		builder.End();

		if( m_parseErrors > 0 ) {
			Logger::Log( OutputMessageType::Error, L"%d errors occured while parsing, '%s'\n",
						 m_parseErrors, filename );
		}
		else {
			// this will create the D3D vertex/index buffers as well
			// as trigger the loading of the textures.
			model->Construct( m_device, ResourceManager::Inst() );
		}

		succeeded = true;
	}
	catch( rapidxml::parse_error& error ) {
		Logger::Log( OutputMessageType::Error, "Parse exception: '%s' while processing '%s'\n", error.what(), charName );
	}
	catch( std::runtime_error& error ) {
		Logger::Log( OutputMessageType::Error, "Processing exception: '%s' while processing '%s'\n", error.what(), charName );
	}
	catch( ... ) {
		Logger::Log( OutputMessageType::Error, L"Generic exception while processing '%s'\n", filename );
	}

	if( !succeeded ) {
		// clean up model (remove all geometry), but don't free memory because there
		// are other references to this model from 
		model->Destroy();
	}

	SAFE_DELETE_ARRAY( data );
#endif

	return succeeded;
}

void CmdlModelFactory::ProcessXml( Model3dBuilder* builder, const pugi::xpath_node& root )
{
	const char* pRootString = "NintendoWareIntermediateFile/GraphicsContentCtr/Models/%s";
	pugi::xpath_node_set nodeSet = evaluateXpathQuery( root, pRootString, "Model" );
	if( nodeSet.size() == 0 ) {
		nodeSet = evaluateXpathQuery( root, pRootString, "SkeletalModel" );
	}
	assert( nodeSet.size() > 0 );

	ProcessModel( builder, nodeSet.first() );
}

void CmdlModelFactory::ProcessModel( Model3dBuilder* pBuilder, const pugi::xpath_node& modelNode )
{
	pugi::xpath_node_set nodeSet;

	// Materials
	nodeSet = evaluateXpathQuery( modelNode, "Materials/MaterialCtr" );
	for( auto node : nodeSet ) {
		ProcessMaterial( pBuilder, node );
	}

	// Root
	Node* pSceneNode = pBuilder->m_model->CreateNode( "CmdlSceneNode" );
	pBuilder->m_model->SetRoot( pSceneNode );

	// Meshes
	nodeSet = evaluateXpathQuery( modelNode, "Meshes/Mesh" );
	std::string todoName( "todo" );
	xpath_node_set shapesNodeSet = evaluateXpathQuery( modelNode, "Shapes/SeparateDataShapeCtr" );
	for( auto meshNode : nodeSet ) {

		Node* pNode = pBuilder->m_model->CreateNode( todoName );
		todoName += "_todo";
		pNode->parent = pSceneNode;
		pSceneNode->children.push_back( pNode );

		ProcessMesh( pBuilder, pNode, meshNode, shapesNodeSet );
	}
}

void CmdlModelFactory::ProcessMaterial( Model3dBuilder* pBuilder, const pugi::xpath_node& materialNode )
{
	// Material name
	const char* name = getAttributeString( materialNode.node(), "Name" );
	if( name == NULL ) {
		name = "!missing-name!";
	}

	Material* material = pBuilder->m_model->CreateMaterial( name );

	// Textures
	pugi::xpath_node_set nodeSet = evaluateXpathQuery( materialNode, "TextureMappers/PixelBasedTextureMapperCtr" );

	// TODO: use only one texture currently. because can't decide witch texture is witch purpose
	//for( auto textureNode : nodeSet ) 
	{
		auto textureNode = nodeSet.first();

		pugi::xpath_node_set nodeSetTextureRef = evaluateXpathQuery( textureNode, "TextureReference" );
		aya::string textureRefName = split( nodeSetTextureRef.first().node().text().as_string(), "\"" ).at( 1 );
		textureRefName += ".tga";

		// TODO: correct path
		Model3dBuilder::MaterialData& materialData = pBuilder->m_material;
		const char* pTextureRefName = textureRefName.c_str();
		if( materialData.image2file.find( pTextureRefName ) == materialData.image2file.end() ) {
			materialData.image2file[pTextureRefName] = pTextureRefName;
		}
		
		material->texNames[0] = pTextureRefName;
	}
}

void CmdlModelFactory::ProcessMesh( Model3dBuilder* pBuilder, Node* pNode, const pugi::xpath_node& meshNode, const pugi::xpath_node_set& shapesNodeSet )
{
	const char* name = getAttributeString( meshNode.node(), "MeshNodeName" );
	if( name == NULL ) {
		name = "!missing-name!";
	}

	pNode->name = name;

	// TODO: should handle custom data?
	//ProcessCustomDataAttributes()
	
	// Shape reference index
	pugi::xpath_node nodeShapeRef = evaluateXpathQuery( meshNode, "SeparateShapeReference" ).first();
	aya::string shapeRefIndex( nodeShapeRef.node().text().as_string() ); // formatted like "Shapes\[[0-9]+\]"
	int index = atoi( splitIndexDeclToIndex( shapeRefIndex ).at( 1 ).c_str() );
	
	// Material reference name
	pugi::xpath_node nodeMaterialRef = evaluateXpathQuery( meshNode, "MaterialReference" ).first();
	aya::string materialRefName( nodeMaterialRef.node().text().as_string() ); // formatted like "Materials\[\"*****\"\]"
	materialRefName = split( materialRefName, "\"" ).at( 1 );

	// ...and get its material
	Material* pMat = pBuilder->m_model->GetMaterial( materialRefName );

	ProcessShape( pBuilder, pNode, pMat, shapesNodeSet[index] );
}

void CmdlModelFactory::ProcessShape( Model3dBuilder* pBuilder, Node* pNode, Material* pMaterial, const pugi::xpath_node& shapeNode )
{
	// reset before initialize
	pBuilder->Mesh_Reset();

	// Vertices
	pugi::xpath_node verticesNode = evaluateXpathQuery( shapeNode, "VertexAttributes" ).first();
	ProcessVertexAttributes( pBuilder, verticesNode );

	// Primitives (Indices)
	pugi::xpath_node_set primitivesNodeSet = evaluateXpathQuery( shapeNode, "PrimitiveSets/PrimitiveSetCtr" );
	for( auto node : primitivesNodeSet ) {
		ProcessPrimitive( pBuilder, pNode, pMaterial, node );
	}
}

void CmdlModelFactory::ProcessVertexAttributes( Model3dBuilder* pBuilder, const pugi::xpath_node& verticesNode )
{
	// verticesNode points <VertexAttributes> tag.

	pBuilder->Mesh_ResetPolyInfo();

	pugi::xml_node child = verticesNode.node().first_child();
	while( !child.empty() ) {
		const char* pUsage = getAttributeString( child, "Usage" );

		// These three types can be supported currently.
		if( compare( pUsage, "Position" ) ) {
			ProcessVertexStream( &pBuilder->m_mesh.source.pos, child );

			pBuilder->m_mesh.poly.hasPos = true;
		}
		else if( compare( pUsage, "Normal" ) ) {
			ProcessVertexStream( &pBuilder->m_mesh.source.nor, child );

			pBuilder->m_mesh.poly.hasNor = true;
		}
		else if( compare( pUsage, "TextureCoordinate0" ) ) {
			ProcessVertexStream( &pBuilder->m_mesh.source.tex, child );

			pBuilder->m_mesh.poly.hasTex = true;
		}

		child = child.next_sibling();
	}

	pBuilder->m_mesh.poly.stride = 1;
}

template <typename T>
void CmdlModelFactory::ProcessVertexStream( std::vector<T>* out, const pugi::xml_node& node )
{
	uint32_t columnNum = LoaderCommon::checkVectorColumnNum( node.name() );
	uint32_t verticesNum = node.attribute( "VertexSize" ).as_uint();

	assert( compare( node.attribute("QuantizedMode").as_string(), "Float" ) ); // Should be float

	uint32_t arraySize = verticesNum * columnNum;
	float* p = new float[arraySize];
	LoaderCommon::loadFloatStream( p, node.text().as_string(), arraySize );

	out->resize( verticesNum );
	for( uint32_t row = 0; row < verticesNum; ++row ) {
		float* pHeadOfVector = &out->at( row ).x;
		for( uint32_t col = 0; col < columnNum; ++col ) {
			pHeadOfVector[col] = p[( row * columnNum ) + col];
		}
	}

	SAFE_DELETE_ARRAY( p );
}

void CmdlModelFactory::ProcessPrimitive( Model3dBuilder* pBuilder, Node* pNode, Material* pMaterial, const pugi::xpath_node& node )
{
	// Create geometry
	aya::string geoName = pNode->name + pMaterial->name;

	// avoid non-unique name
	while( pBuilder->m_model->GetGeometry( geoName ) != NULL ) {
		geoName += "_dup";
	}
	Geometry* pGeometry = pBuilder->m_model->CreateGeometry( geoName );
	
	pGeometry->material = pMaterial;
	pNode->geometries.push_back( pGeometry );

	pBuilder->Mesh_Begin( geoName.c_str() );
	pBuilder->Mesh_SetPrimType( "TRIANGLES" );

	// Index stream
	const char* queryString = "Primitives/PrimitiveCtr/IndexStreams/%s";
	pugi::xpath_node_set nodeSetIndexStream = evaluateXpathQuery( node, queryString, "UbyteIndexStreamCtr" );
	if( nodeSetIndexStream.size() == 0 ) {
		nodeSetIndexStream = evaluateXpathQuery( node, queryString, "UshortIndexStreamCtr" );
		assert( nodeSetIndexStream.size() > 0 );
	}
	
	ProcessIndexStream( &pBuilder->m_mesh.poly.indices, nodeSetIndexStream.first().node() );

	pBuilder->Mesh_AddTriangles();
	pGeometry->mesh = pBuilder->m_mesh.mesh;
	pBuilder->Mesh_End();
}

template <typename T>
void CmdlModelFactory::ProcessIndexStream( std::vector<T>* out, const pugi::xml_node& node )
{
	assert( compare( node.attribute( "PrimitiveMode" ).as_string(), "Triangles" ) );
	uint32_t indicesNum = node.attribute( "Size" ).as_uint();
	uint32_t* p = new uint32_t[indicesNum];
	LoaderCommon::loadUIntStream( p, node.text().as_string(), indicesNum );

	out->resize( indicesNum );
	for( uint32_t i = 0; i < indicesNum; ++i ) {
		out->at( i ) = p[i];
	}

	SAFE_DELETE_ARRAY( p );
}

}; // namespace LvEdEngine


