#pragma once
#include "../GameObject.h"
#include "../../Usagi/UsagiUtil.h"
#include "../../Renderer/Resource.h"

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

		void AddResource(ResourceReference * ref, int index);
		void RemoveResource(ResourceReference * ref);
	protected:
		Mesh* m_mesh;
		void BuildRenderables();
	};

}