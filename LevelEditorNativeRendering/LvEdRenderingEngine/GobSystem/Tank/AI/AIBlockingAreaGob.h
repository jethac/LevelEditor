#ifndef AIBlockingAreaGob_h__
#define AIBlockingAreaGob_h__

#include "./../../CubeGob.h"

namespace LvEdEngine
{
	class AIBlockingAreaGob : public CubeGob
	{
	public:
		AIBlockingAreaGob() : CubeGob() {}
		virtual const char* ClassName() const { return StaticClassName(); }
		static const char* StaticClassName(){ return "AIBlockGob"; }
	};
}

#endif // AIBlockingAreaGob_h__
