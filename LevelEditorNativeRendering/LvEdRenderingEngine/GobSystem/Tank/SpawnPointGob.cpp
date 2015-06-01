#include "SpawnPointGob.h"



void LvEdEngine::SpawnPointGob::SetTeam( int team )
{
	mTeam = team;

	int red = 0xffff0000;
	int blue = 0xff0000ff;
	SetColor( mTeam == 0 ? red : blue );
}
