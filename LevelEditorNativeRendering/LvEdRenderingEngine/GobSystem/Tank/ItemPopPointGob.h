#ifndef ItemPopPointGob_h__
#define ItemPopPointGob_h__

#include "./../CubeGob.h"

namespace LvEdEngine
{

class ItemPopPointGob : public CubeGob
{
public:
	ItemPopPointGob() : CubeGob() {}
	virtual ~ItemPopPointGob() {}

	virtual const char* ClassName() const { return StaticClassName(); }
	static const char* StaticClassName(){ return "ItemPopPointGob"; }

	virtual void SetupRenderable( RenderableNode* r, RenderContext* context );
protected:
	
private:
};

}

#endif // ItemPopPointGob_h__
