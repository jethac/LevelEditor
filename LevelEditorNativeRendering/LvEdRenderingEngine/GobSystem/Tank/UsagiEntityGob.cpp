#include "UsagiEntityGob.h"
#include "../../Renderer/Model.h"
#include "../../Renderer/ShapeLib.h"
#include "../../Renderer/TextureLib.h"

LvEdEngine::UsagiEntityGob::UsagiEntityGob()
{
	SetCastsShadows(false);       // doesn't block the light
	SetReceivesShadows(false);    // no shadow is cast on it
	m_mesh = ShapeLibGetMesh(RenderShape::Quad);
	m_localBounds = AABB(float3(-0.5f, -0.5f, -0.5f), float3(0.5f, 0.5f, 0.5f));
}

LvEdEngine::UsagiEntityGob::~UsagiEntityGob()
{

}

void LvEdEngine::UsagiEntityGob::Update(float dt)
{

}

void LvEdEngine::UsagiEntityGob::GetRenderables(RenderableNodeCollector* collector, RenderContext* context)
{
	if (!IsVisible(context->Cam().GetFrustum()))
		return;

	// Vitei logo.
	RenderableNode renderable;
	GameObject::SetupRenderable(&renderable, context);

	renderable.mesh = m_mesh;
	renderable.textures[TextureType::DIFFUSE] = TextureLib::Inst()->GetByName(L"vitei_256.png");

	float3 objectPos = &m_world.M41;
	Camera& cam = context->Cam();
	Matrix billboard = Matrix::CreateBillboard(objectPos, cam.CamPos(), cam.CamUp(), cam.CamLook());
	Matrix scale = Matrix::CreateScale(0.4f);
	renderable.WorldXform = scale * billboard;
	
	RenderFlagsEnum flags = RenderFlags::Textured;
	collector->Add(renderable, flags, Shaders::BillboardShader);

}

void LvEdEngine::UsagiEntityGob::BuildRenderables()
{

}
