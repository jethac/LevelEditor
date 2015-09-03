#include "ItemPopPointGob.h"

using namespace LvEdEngine;

// virtual
void ItemPopPointGob::SetupRenderable( RenderableNode* r, RenderContext* context )
{
	PrimitiveShapeGob::SetupRenderable( r, context );
	SetColor( 0xFFFFFF00 ); // Yellow

	// Arrange cube's position
	Matrix trans_y = Matrix::CreateTranslation( 0.0f, 0.5f, 0.0f );
	r->WorldXform = trans_y * r->WorldXform;
}

