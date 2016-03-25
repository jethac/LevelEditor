#ifndef SpawnPointGob_h__
#define SpawnPointGob_h__

#include "./../ConeGob.h"

namespace LvEdEngine
{
	class SpawnPointGob : public GameObject
	{
	public:
		SpawnPointGob() : GameObject() {
			m_meshQuad = ShapeLibGetMesh(RenderShape::Quad);
			m_localBounds = AABB(float3(-1.0f, 0.0f, -1.0f), float3(1.0f, 2.0f, 1.0f));

			mTeam = 0;
			ConvertColor(0xffff0000, &mColor);
		}
		virtual const char* ClassName() const { return StaticClassName(); }
		static const char* StaticClassName(){ return "SpawnPointGob"; }

		void SetTeam( int team );

		virtual void GetRenderables(RenderableNodeCollector* collector, RenderContext* context);

	private:
		int mTeam;

		float4 mColor;

		Mesh* m_meshQuad;
	};
}


#endif // SpawnPointGob_h__
