
#pragma once
#include "../GameObject.h"
#include "../../Renderer/Resource.h"

namespace LvEdEngine
{
	class StandardBaseGob : public GameObject
	{
	public:
		StandardBaseGob();
		virtual ~StandardBaseGob();

		virtual const char* ClassName() const { return StaticClassName(); }
		static const char* StaticClassName(){ return "StandardBaseGob"; }

		// push Renderable nodes
		virtual bool GetRenderables( RenderableNodeCollector* collector, RenderContext* context );
		virtual void Update( float dt );

		// functions
		void SetLife( int life );

		void AddResource( ResourceReference * ref, int index );
		void RemoveResource( ResourceReference * ref );

	protected:
		void BuildRenderables();

		ResourceReference* m_geometry;
		RenderNodeList m_renderables;

		//std::vector<GameObjectReference*> m_friends;
		//std::vector<StandardBaseGob*> m_children;
		
		int m_life;
		std::vector<Matrix> m_modelTransforms;

	};
}
