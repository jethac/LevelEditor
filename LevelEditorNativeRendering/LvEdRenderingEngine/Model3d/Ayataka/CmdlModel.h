
#pragma once
#include "pugixmlUtil.h"

namespace LvEdEngine
{
	class Model3dBuilder;

    void ProcessXml( Model3dBuilder* builder, const pugi::xpath_node& root );
	void ProcessModel( Model3dBuilder* pBuilder, const pugi::xpath_node& modelNode );
	void ProcessMaterial( Model3dBuilder* pBuilder, const pugi::xpath_node& materialNode );
	void ProcessHierarchy( Model3dBuilder* pBuilder, Node* pRoot, const pugi::xpath_node& modelNode );
	void ProcessSkeleton( Model3dBuilder* pBuilder, Node* pRoot, const pugi::xpath_node& modelNode, const pugi::xpath_node& skeletonNode );
	void ProcessMesh( Model3dBuilder* pBuilder, Node* pNode, const pugi::xpath_node& meshNode, const pugi::xpath_node_set& shapesNodeSet );
	void ProcessShape( Model3dBuilder* pBuilder, Node* pNode, Material* pMaterial, const pugi::xpath_node& shapeNode );

	void ProcessVertexAttributes( Model3dBuilder* pBuilder, const pugi::xpath_node& verticesNode );
	template <typename T> void ProcessVertexStream( std::vector<T>* out, const pugi::xml_node& node );
	template <typename T> void ProcessSingleVertex( std::vector<T>* out, const pugi::xml_node& node, uint32_t num );

	void ProcessPrimitive( Model3dBuilder* pBuilder, Node* pNode, Material* pMaterial, const pugi::xpath_node& node );
	template <typename T> void ProcessIndexStream( std::vector<T>* out, const pugi::xml_node& node );
};