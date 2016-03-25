#include "SpawnPointGob.h"
#include "../../Renderer/Model.h"
#include "../../Renderer/ShapeLib.h"
#include "../../Renderer/TextureLib.h"

using namespace LvEdEngine;

void SpawnPointGob::SetTeam( int team )
{
	mTeam = team;

	switch (mTeam)
	{
	case 0:
		ConvertColor(0xFFE80806, &mColor);
		break;
	case 1:
		ConvertColor(0xFF060CE8, &mColor);
		break;
	default:
		ConvertColor(0xffffffff, &mColor);
		break;
	}

}

void SpawnPointGob::GetRenderables(RenderableNodeCollector* collector, RenderContext* context)
{
	if (!IsVisible(context->Cam().GetFrustum()))
		return;

	// Marker node.
	RenderableNode marker_node;
	GameObject::SetupRenderable(&marker_node, context);

	marker_node.mesh = m_meshQuad;
	marker_node.diffuse = mColor;
	LvEdEngine::Texture* pTeamTexture = TextureLib::Inst()->GetByName(L"fa-user.png");
#ifdef _DEBUG
	assert(pTeamTexture != nullptr);
#endif
	marker_node.textures[TextureType::DIFFUSE] = pTeamTexture;

	float3 objectPos = &m_world.M41;
	Camera& cam = context->Cam();
	Matrix billboard = Matrix::CreateBillboard(objectPos, cam.CamPos(), cam.CamUp(), cam.CamLook());
	Matrix scale = Matrix::CreateScale(2.0f);
	Matrix tx = Matrix::CreateTranslation(0, 1.0f, 0);
	marker_node.WorldXform = scale * tx * billboard;

	RenderFlagsEnum flags = RenderFlags::Textured;
	collector->Add(marker_node, flags, Shaders::BillboardShader);
}

