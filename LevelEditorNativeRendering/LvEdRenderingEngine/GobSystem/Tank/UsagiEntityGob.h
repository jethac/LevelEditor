#pragma once
#include "../GameObject.h"
#include "../../Usagi/UsagiUtil.h"

namespace LvEdEngine
{

	class UsagiEntityGob : public GameObject
	{
	public:
		virtual const char* ClassName() const { return StaticClassName(); }
		static const char* StaticClassName() { return "UsagiEntityGob"; }

		UsagiEntityGob();
		virtual ~UsagiEntityGob();

		virtual void Update(float dt);

		virtual void GetRenderables(RenderableNodeCollector* collector, RenderContext* context);
	protected:
		Mesh* m_mesh;
		void BuildRenderables();
	};

}