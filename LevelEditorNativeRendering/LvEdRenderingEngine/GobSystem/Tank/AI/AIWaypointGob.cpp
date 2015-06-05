#include "AIWaypointGob.h"
#include "../../PrimitiveShapeGob.h"

using namespace LvEdEngine;

// virtual
void AIWaypointGob::SetupRenderable( RenderableNode* r, RenderContext* context )
{
	PrimitiveShapeGob::SetupRenderable( r, context );
	
	// Arrange cone's direction and scale
	Matrix rotX_p180 = Matrix::CreateRotationX( ToRadian( 180.0f ) );
	Matrix trans_y = Matrix::CreateTranslation( 0.0f, 0.5f, 0.0f );
	r->WorldXform = rotX_p180 * trans_y * r->WorldXform;
}

