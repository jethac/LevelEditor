#include "SpawnPointGob.h"
#include "..\PrimitiveShapeGob.h"

using namespace LvEdEngine;

void SpawnPointGob::SetTeam( int team )
{
	mTeam = team;

	int red = 0xffff0000;
	int blue = 0xff0000ff;
	SetColor( mTeam == 0 ? red : blue );
}

// virtual
void SpawnPointGob::SetupRenderable( RenderableNode* r, RenderContext* context )
{
	PrimitiveShapeGob::SetupRenderable( r, context );
	
	// Arrange cone's direction and scale
	Matrix rotX_p90 = Matrix::CreateRotationX( ToRadian( 90.0f ) );
	Matrix scale_2x = Matrix::CreateScale( float3( 2.0f, 2.0f, 2.0f ) );
	Matrix trans_y = Matrix::CreateTranslation( 0.0f, 0.5f, 0.0f );
	r->WorldXform = rotX_p90 * scale_2x * trans_y * r->WorldXform;
}

