#pragma once

#include "../../Renderer/RenderUtil.h"
#include "../../Renderer/ShapeLib.h"
#include "../../Renderer/Resource.h"
#include "../GameObject.h"

#include "../../Usagi/UsagiUtil.h"

namespace LvEdEngine
{

	class StaticModelGob : public GameObject
	{
	public:
		virtual const char* ClassName() const {return StaticClassName();}
		static const char* StaticClassName(){return "StaticModelGob";}
		StaticModelGob( void );
		virtual ~StaticModelGob();

		virtual void Update(float dt);
	
		// push Renderable nodes
		virtual void GetRenderables(RenderableNodeCollector* collector, RenderContext* context);

		void SetupModel( const wchar_t* filename );
	protected:
		void BuildRenderables( void );

		ResourceReference mModelResource;
		std::vector<Matrix> mModelTransforms;
		RenderNodeList mRenderables;

		wchar_t mPathPrefix[MAX_PATH/2];
	};

	class ItemPopPointGob : public StaticModelGob
	{
	public:
		virtual const char* ClassName() const { return StaticClassName(); }
		static const char* StaticClassName(){ return "ItemPopPointGob"; }

		ItemPopPointGob() : StaticModelGob() {
			GetUsagiDir( mPathPrefix, ARRAY_SIZE( mPathPrefix ) );
			wcscat( mPathPrefix, L"\\Data\\" );
			SetupModel( ModelPath() );
		}
		virtual ~ItemPopPointGob() {}
		static const wchar_t* ModelPath() {
			return L"Models\\t1obj_item_secretbox\\t1obj_item_secretbox.cmdl";
		}

	};
}
