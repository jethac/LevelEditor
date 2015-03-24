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

bool isSingleVertex( const pugi::xml_node& node )
{
	return endsWith( node.name(), "AttributeCtr" );
}

int getShapeRefIndex( const pugi::xpath_node& meshNode )
{
	pugi::xpath_node nodeShapeRef = evaluateXpathQuery( meshNode, "SeparateShapeReference" ).first();
	aya::string shapeRefIndex( nodeShapeRef.node().text().as_string() ); // formatted like "Shapes\[[0-9]+\]"
	return atoi( splitIndexDeclToIndex( shapeRefIndex ).at( 1 ).c_str() );
}

int getBoneRefIndex( const pugi::xpath_node& shapeNode )
{
	pugi::xpath_node_set nodeSet = evaluateXpathQuery( shapeNode, "PrimitiveSets[1]/PrimitiveSetCtr/BoneIndexTable" );
	assert( nodeSet.size() > 0 );
	return atoi( nodeSet.first().node().text().as_string() );
}

namespace LvEdEngine
{
float3 GetVector3( pugi::xml_node node )
{
	return float3( node.attribute( "X" ).as_float(), node.attribute( "Y" ).as_float(), node.attribute( "Z" ).as_float() );
}

void CalcTransform( LvEdEngine::Matrix* pOut, const pugi::xml_node& transformNode )
{
	float3 scale = GetVector3( transformNode.child( "Scale" ) );
	float3 rotate = GetVector3( transformNode.child( "Rotate" ) );
	float3 translate = GetVector3( transformNode.child( "Translate" ) );

	Matrix scaleMatrix = Matrix::CreateScale( scale );
	Matrix rotateXMatrix = Matrix::CreateRotationX( rotate.x );
	Matrix rotateYMatrix = Matrix::CreateRotationX( rotate.y );
	Matrix rotateZMatrix = Matrix::CreateRotationX( rotate.z );
	Matrix translateMatrix = Matrix::CreateTranslation( translate );

	Matrix id;
	*pOut = id * scaleMatrix * rotateXMatrix * rotateYMatrix * rotateZMatrix * translateMatrix;
}

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

void CmdlModelFactory::ParseError( const char * fmt, ... )
{
	va_list args;
	va_start( args, fmt );
	Logger::LogVA( OutputMessageType::Error, fmt, args );
	va_end( args );
	m_parseErrors++;
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
	ProcessHierarchy( pBuilder, pSceneNode, modelNode );
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
	pugi::xpath_node_set texCoordinators = evaluateXpathQuery( materialNode, "TextureCoordinators/TextureCoordinatorCtr" );
	
	int index = 0;
	for( auto textureNode : nodeSet ) {
		pugi::xpath_node coordinator = texCoordinators[index++];

		pugi::xpath_node_set nodeSetTextureRef = evaluateXpathQuery( textureNode, "TextureReference" );
		aya::string textureRefName = split( nodeSetTextureRef.first().node().text().as_string(), "\"" ).at( 1 );
		if( textureRefName.compare( "shadowmap" ) == 0 ) {
			continue;
		}
		textureRefName += ".tga";

		// Textures should locate under 'tga' directory
		textureRefName = "tga/" + textureRefName;
		Model3dBuilder::MaterialData& materialData = pBuilder->m_material;
		const char* pTextureRefName = textureRefName.c_str();
		if( materialData.image2file.find( pTextureRefName ) == materialData.image2file.end() ) {
			materialData.image2file[pTextureRefName] = pTextureRefName;
		}
		
		material->texNames[0] = pTextureRefName;

		// Texture matrix
		float scaleS = coordinator.node().attribute( "ScaleS" ).as_float();
		float scaleT = coordinator.node().attribute( "ScaleT" ).as_float();
		float rotate = coordinator.node().attribute( "Rotate" ).as_float();
		float transS = coordinator.node().attribute( "TranslateS" ).as_float();
		float transT = coordinator.node().attribute( "TranslateT" ).as_float();
		Matrix texMtx;
		material->textureTransform = texMtx * Matrix::CreateScale( scaleS, scaleT, 1.0f ) * Matrix::CreateRotationZ( rotate ) * Matrix::CreateTranslation( transS, transT, 0.0 );;

		// TODO: use only one texture currently. because can't decide witch texture is witch purpose
		break;
	}
}

void CmdlModelFactory::ProcessHierarchy( Model3dBuilder* pBuilder, Node* pRoot, const pugi::xpath_node& modelNode )
{
	// Can find <Skeleton> tag ?
	pugi::xpath_node_set skeletonNodeSet = evaluateXpathQuery( modelNode, "Skeleton" );
	
	if( skeletonNodeSet.size() == 0 ) {
		// No skeleton. Flat hierarchy under pRoot.
		pugi::xpath_node_set nodeSet = evaluateXpathQuery( modelNode, "Meshes/Mesh" );
		std::string todoName( "todo" );
		pugi::xpath_node_set shapesNodeSet = evaluateXpathQuery( modelNode, "Shapes/SeparateDataShapeCtr" );
		for( auto meshNode : nodeSet ) {

			Node* pNode = pBuilder->m_model->CreateNode( todoName );
			todoName += "_todo";
			pNode->parent = pRoot;
			pRoot->children.push_back( pNode );

			ProcessMesh( pBuilder, pNode, meshNode, shapesNodeSet );
		}
	}
	else {
		ProcessSkeleton( pBuilder, pRoot, modelNode, skeletonNodeSet.first() );
	}
}

void CmdlModelFactory::ProcessSkeleton( Model3dBuilder* pBuilder, Node* pRoot, const pugi::xpath_node& modelNode, const pugi::xpath_node& skeletonNode )
{
	// Create root node of hierarchy
	Node* pSkeletonRoot = pBuilder->m_model->CreateNode( "SkeletonRoot" );
	pSkeletonRoot->parent = pRoot;
	pRoot->children.push_back( pSkeletonRoot );

	// Meshes and Shapes
	pugi::xpath_node_set meshes = evaluateXpathQuery( modelNode, "Meshes/Mesh" );
	xpath_node_set shapes = evaluateXpathQuery( modelNode, "Shapes/SeparateDataShapeCtr" );

	size_t boneIndex = 0;
	pugi::xpath_node_set bones = evaluateXpathQuery( skeletonNode, "Bones/Bone" );
	for( auto boneNode : bones ) {
		pugi::xml_node bone = boneNode.node();
		const char* pName = getAttributeString( bone, "Name" );
		const char* pParentName = getAttributeString( bone, "ParentBoneName" );

		Node* pNewNode = pBuilder->m_model->CreateNode( pName );
		Node* pParentNode = NULL;

		// Who is my parent?
		if( pParentName[0] == '\0' ) {
			// This is the ROOT bone
			pParentNode = pSkeletonRoot;
		}
		else {
			// Search my parent
			pParentNode = pBuilder->m_model->GetNode( pParentName );
		}
		assert( pParentNode );
		pNewNode->parent = pParentNode;
		pParentNode->children.push_back( pNewNode );

		// Calculate transform
		CalcTransform( &pNewNode->transform, bone.child( "Transform" ) );

		// Search the target <Mesh> tag
		for( auto meshNode : meshes ) {
			int shapeRefIndex = getShapeRefIndex( meshNode );
			
			auto shapeNode = shapes[shapeRefIndex];
			int boneRefIndex = getBoneRefIndex( shapeNode );

			if( boneRefIndex == boneIndex ) {
				ProcessMesh( pBuilder, pNewNode, meshNode, shapes );
			}
		}

		++boneIndex;
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
	int index = getShapeRefIndex( meshNode );
	
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
		Model3dBuilder::MeshSourceData& meshSource = pBuilder->m_mesh.source;

		// These three types can be supported currently.
		if( compare( pUsage, "Position" ) ) {
			ProcessVertexStream( &meshSource.pos, child );

			pBuilder->m_mesh.poly.hasPos = true;
		}
		else if( compare( pUsage, "Normal" ) ) {
			if( isSingleVertex( child ) ) {
				ProcessSingleVertex( &meshSource.nor, child, (uint32_t)meshSource.pos.size() );
			}
			else {
				ProcessVertexStream( &meshSource.nor, child );
			}

			pBuilder->m_mesh.poly.hasNor = true;
		}
		else if( compare( pUsage, "TextureCoordinate0" ) ) {
			if( isSingleVertex( child ) ) {
				ProcessSingleVertex( &meshSource.tex, child, (uint32_t)meshSource.pos.size() );
			}
			else {
				ProcessVertexStream( &meshSource.tex, child );
			}

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

template <typename T>
void CmdlModelFactory::ProcessSingleVertex( std::vector<T>* out, const pugi::xml_node& node, uint32_t num )
{
	uint32_t columnNum = LoaderCommon::checkVectorColumnNum( node.name() );
	uint32_t verticesNum = num;

	float* p = new float[columnNum];
	LoaderCommon::loadFloatStream( p, node.text().as_string(), columnNum );

	// Copy single vertex to each vertices
	out->resize( verticesNum );
	for( uint32_t row = 0; row < verticesNum; ++row ) {
		float* pHeadOfVector = &out->at( row ).x;
		for( uint32_t col = 0; col < columnNum; ++col ) {
			pHeadOfVector[col] = p[col];
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


