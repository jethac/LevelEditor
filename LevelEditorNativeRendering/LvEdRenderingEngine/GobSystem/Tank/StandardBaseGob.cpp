
#include "StandardBaseGob.h"
#include <algorithm>

#include "../../Renderer/RenderUtil.h"
#include "../../Renderer/ShapeLib.h"
#include "../../Renderer/RenderBuffer.h"
#include "../../Renderer/Model.h"

namespace LvEdEngine
{


// functions
// ----------------------------------------------------------------------------------
void StandardBaseGob::SetLife(int life)
{
    m_life = life;
}

// ----------------------------------------------------------------------------------
void StandardBaseGob::AddResource(ResourceReference* ref, int /*index*/)
{
    m_geometry = ref;
    m_modelTransforms.clear();
    m_renderables.clear();
    InvalidateBounds();
    InvalidateWorld();
}

// ----------------------------------------------------------------------------------
void StandardBaseGob::RemoveResource(ResourceReference* /*ref*/)
{
    AddResource(NULL, -1);
}

// ----------------------------------------------------------------------------------
StandardBaseGob::StandardBaseGob()
{
    m_geometry = NULL;
}

// ----------------------------------------------------------------------------------
StandardBaseGob::~StandardBaseGob()
{
    SAFE_DELETE(m_geometry);
}

// ----------------------------------------------------------------------------------
void StandardBaseGob::GetRenderables(RenderableNodeCollector* collector, RenderContext* /*context*/)
{   
    if(!IsVisible())
    {
        return;
    }

    
    RenderFlagsEnum flags = (RenderFlagsEnum) (RenderFlags::Textured | RenderFlags::Lit);

    if (!m_renderables.empty())
    {
        collector->Add(m_renderables.begin(), m_renderables.end(), flags, Shaders::TexturedShader);
    }
    else
    {
        Mesh* mesh = ShapeLibGetMesh(RenderShape::Cube);
        
        RenderableNode r;
        r.mesh = mesh;
        r.diffuse = float4(0.0f,0.3f,0,1);
        r.objectId = GetInstanceId();
        r.WorldXform = m_world;
        r.bounds = m_bounds;
        LightingState::Inst()->UpdateLightEnvironment(r);
        collector->Add(r, flags, Shaders::TexturedShader);
    }
}

// ----------------------------------------------------------------------------------
void StandardBaseGob::BuildRenderables()
{
    m_renderables.clear();
    Model* model = NULL;
    assert(m_geometry);
    model = (Model*)m_geometry->GetTarget();

    // If !model or !IsReady, then UpdateBegin should have returned false and UpdateEnd|BuildRenderables
    // should not be called.  Assert this is the case.
    assert(model && model->IsReady());

    const NodeDict& nodes = model->Nodes();
    for(auto nodeIt = nodes.begin(); nodeIt != nodes.end(); ++nodeIt)
    {
        Node* node = nodeIt->second;
        assert(m_modelTransforms.size() >= node->index);
        const Matrix& world = m_modelTransforms[node->index]; // transform array holds world matricies already, not local
        for(auto geoIt = node->geometries.begin(); geoIt != node->geometries.end(); ++geoIt)
        {
            Geometry* geo = (*geoIt);
            Material * mat = geo->material;
            RenderableNode renderNode;
            renderNode.mesh = geo->mesh;
            renderNode.WorldXform = world;
            renderNode.bounds = geo->mesh->bounds;
            renderNode.bounds.Transform(renderNode.WorldXform);
            renderNode.objectId = GetInstanceId();
            renderNode.diffuse =  mat->diffuse;
            renderNode.specular = mat->specular.xyz();
            renderNode.specPower = mat->power;
            renderNode.SetFlag( RenderableNode::kShadowCaster, GetCastsShadows() );
            renderNode.SetFlag( RenderableNode::kShadowReceiver, GetReceivesShadows() );

            LightingState::Inst()->UpdateLightEnvironment(renderNode);

            for(unsigned int i = TextureType::MIN; i < TextureType::MAX; ++i)
            {
                renderNode.textures[i] = geo->material->textures[i];
            }
            m_renderables.push_back(renderNode);
        }
    }
}


// ----------------------------------------------------------------------------------
void StandardBaseGob::Update( float dt )
{
     bool udpateXforms = m_worldDirty;
        UpdateWorldTransform();
        Model* model = m_geometry ? (Model*)m_geometry->GetTarget() : NULL;
        if( model && model->IsReady())
        {
            if(m_modelTransforms.empty() || udpateXforms)
            {
                 const MatrixList& matrices = model->AbsoluteTransforms();
                 m_modelTransforms.resize(matrices.size());
                 for( unsigned int i = 0; i < m_modelTransforms.size(); ++i)
                 {
                     m_modelTransforms[i] = matrices[i] * m_world; // transform matrix array now holds complete world transform.
                 }

                 BuildRenderables();
                 m_boundsDirty = true;
            }
        }

        if(m_boundsDirty)
        {
            if(!m_modelTransforms.empty())
            {
               // assert(model && model->IsReady());
                m_localBounds = model->GetBounds();
                if(m_parent) m_parent->InvalidateBounds();
            }
            else
            {
                m_localBounds = AABB(float3(-0.5f,-0.5f,-0.5f), float3(0.5f,0.5f,0.5f));                
            }
            this->UpdateWorldAABB();
        }

        if(RenderContext::Inst()->LightEnvDirty)
        {
            // update light env.
            for(auto  renderNode = m_renderables.begin(); renderNode != m_renderables.end(); renderNode++)
            {
                LightingState::Inst()->UpdateLightEnvironment(*renderNode);
            }
        }

}

}; // namespace
