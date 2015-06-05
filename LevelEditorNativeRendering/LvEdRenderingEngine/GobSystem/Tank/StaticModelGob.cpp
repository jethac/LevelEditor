#include "StaticModelGob.h"
#include "../../Renderer/Texture.h"
#include "../../Renderer/TextureLib.h"
#include "../../Renderer/Model.h"

namespace LvEdEngine {

//---------------------------------------------------------------------------
StaticModelGob::StaticModelGob(void)
{
	mPathPrefix[0] = '\0';
	mModelTransforms.clear();
	mRenderables.clear();
	InvalidateBounds();
	InvalidateWorld();
}

//---------------------------------------------------------------------------
StaticModelGob::~StaticModelGob()
{
}

//---------------------------------------------------------------------------
// push Renderable nodes
//virtual 
void StaticModelGob::GetRenderables(RenderableNodeCollector* collector, RenderContext* context)
{
	if( !IsVisible( context->Cam().GetFrustum() ) ) {
		return;
	}

	RenderFlagsEnum flags = (RenderFlagsEnum)( RenderFlags::Textured | RenderFlags::Lit );
	collector->Add( mRenderables.begin(), mRenderables.end(), flags, Shaders::TexturedShader );
}

void StaticModelGob::SetupModel( const wchar_t* filename )
{
	// Load a resource
	assert( mModelResource.GetTarget() == NULL );

	wchar_t fullpath[MAX_PATH];
	swprintf( fullpath, MAX_PATH - 1, L"%s%s", mPathPrefix, filename );
	mModelResource.SetTarget( fullpath );
}

void StaticModelGob::Update( float dt )
{
	bool udpateXforms = m_worldDirty;
	UpdateWorldTransform();
	Model* model = (Model*)mModelResource.GetTarget();
	if( model && model->IsReady() ) {
		if( mModelTransforms.empty() || udpateXforms ) {
			const MatrixList& matrices = model->AbsoluteTransforms();
			mModelTransforms.resize( matrices.size() );
			for( unsigned int i = 0; i < mModelTransforms.size(); ++i ) {
				mModelTransforms[i] = matrices[i] * m_world; // transform matrix array now holds complete world transform.
			}
			BuildRenderables();
			m_boundsDirty = true;
		}
	}

	if( m_boundsDirty ) {
		if( !mModelTransforms.empty() ) {
			// assert(model && model->IsReady());
			m_localBounds = model->GetBounds();
			if( m_parent ) m_parent->InvalidateBounds();
		}
		else {
			m_localBounds = AABB( float3( -0.5f, -0.5f, -0.5f ), float3( 0.5f, 0.5f, 0.5f ) );
		}
		this->UpdateWorldAABB();
	}

	if( RenderContext::Inst()->LightEnvDirty ) {
		// update light env.
		for( auto renderNode = mRenderables.begin(); renderNode != mRenderables.end(); renderNode++ ) {
			LightingState::Inst()->UpdateLightEnvironment( *renderNode );
		}
	}
}

void StaticModelGob::BuildRenderables(void)
{
	mRenderables.clear();
	Model* pModel = (Model*)mModelResource.GetTarget();
	assert( pModel && pModel->IsReady() );

	// Resize & setup vector of transform
	const MatrixList& matrices = pModel->AbsoluteTransforms();
	mModelTransforms.resize( matrices.size() );
	for( unsigned int i = 0; i < mModelTransforms.size(); ++i ) {
		mModelTransforms[i] = matrices[i] * m_world; // transform matrix array now holds complete world transform.
	}

	const NodeDict& nodes = pModel->Nodes();
	for( auto nodeIt = nodes.begin(); nodeIt != nodes.end(); ++nodeIt ) {
		Node* node = nodeIt->second;
		assert( mModelTransforms.size() >= node->index );
		const Matrix& world = mModelTransforms[node->index]; // transform array holds world matricies already, not local
		for( auto geoIt = node->geometries.begin(); geoIt != node->geometries.end(); ++geoIt ) {
			Geometry* geo = ( *geoIt );
			Material* mat = geo->material;
			RenderableNode renderNode;
			renderNode.mesh = geo->mesh;
			renderNode.WorldXform = world;
			renderNode.bounds = geo->mesh->bounds;
			renderNode.bounds.Transform( renderNode.WorldXform );
			renderNode.objectId = GetInstanceId();
			renderNode.diffuse = mat->diffuse;
			renderNode.specular = mat->specular.xyz();
			renderNode.specPower = mat->power;
			renderNode.SetFlag( RenderableNode::kShadowCaster, GetCastsShadows() );
			renderNode.SetFlag( RenderableNode::kShadowReceiver, GetReceivesShadows() );
			renderNode.TextureXForm = mat->textureTransform;

			LightingState::Inst()->UpdateLightEnvironment( renderNode );

			for( unsigned int i = TextureType::MIN; i < TextureType::MAX; ++i ) {
				renderNode.textures[i] = geo->material->textures[i];
			}
			mRenderables.push_back( renderNode );
		}
	}
}

}