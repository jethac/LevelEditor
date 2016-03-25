#ifndef AIWaypointGob_h__
#define AIWaypointGob_h__

#include "./../../ConeGob.h"

namespace LvEdEngine
{
	class AIWaypointGob : public GameObject
	{
	public:
		AIWaypointGob() : GameObject() {
			m_meshQuad = ShapeLibGetMesh(RenderShape::Quad);
			m_localBounds = AABB(float3(-1.0f, 0.0f, -1.0f), float3(1.0f, 2.0f, 1.0f));
		}
		virtual const char* ClassName() const { return StaticClassName(); }
		static const char* StaticClassName(){ return "AIWaypointGob"; }

		virtual void GetRenderables(RenderableNodeCollector* collector, RenderContext* context);
	private:
		Mesh* m_meshQuad;
	};
}

#endif // AIWaypointGob_h__
