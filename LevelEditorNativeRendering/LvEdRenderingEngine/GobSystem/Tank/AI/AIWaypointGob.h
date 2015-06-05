#ifndef AIWaypointGob_h__
#define AIWaypointGob_h__

#include "./../../ConeGob.h"

namespace LvEdEngine
{
	class AIWaypointGob : public ConeGob
	{
	public:
		virtual const char* ClassName() const { return StaticClassName(); }
		static const char* StaticClassName(){ return "AIWaypointGob"; }

		virtual void SetupRenderable( RenderableNode* r, RenderContext* context );
	};
}

#endif // AIWaypointGob_h__
