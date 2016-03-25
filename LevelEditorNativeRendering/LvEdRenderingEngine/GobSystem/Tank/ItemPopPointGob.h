#ifndef ItemPopPointGob_h__
#define ItemPopPointGob_h__

#include "./../CubeGob.h"

namespace LvEdEngine
{

class ItemPopPointGob : public GameObject
{
public:
	ItemPopPointGob() : GameObject() {
		m_meshQuad = ShapeLibGetMesh(RenderShape::Quad);
		m_localBounds = AABB(float3(-1.0f, 0.0f, -1.0f), float3(1.0f, 2.0f, 1.0f));
	}
	virtual ~ItemPopPointGob() {}

	virtual const char* ClassName() const { return StaticClassName(); }
	static const char* StaticClassName(){ return "ItemPopPointGob"; }

	virtual void GetRenderables(RenderableNodeCollector* collector, RenderContext* context);
private:
	Mesh* m_meshQuad;
};

}

#endif // ItemPopPointGob_h__
