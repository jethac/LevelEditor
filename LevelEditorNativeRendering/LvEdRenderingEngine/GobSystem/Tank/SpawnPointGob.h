#ifndef SpawnPointGob_h__
#define SpawnPointGob_h__

#include "./../ConeGob.h"

namespace LvEdEngine
{
	class SpawnPointGob : public ConeGob
	{
	public:
		SpawnPointGob() : ConeGob() {
			mTeam = 0;
		}
		virtual const char* ClassName() const { return StaticClassName(); }
		static const char* StaticClassName(){ return "SpawnPointGob"; }

		void SetTeam( int team );

	private:
		int mTeam;
	};
}


#endif // SpawnPointGob_h__
