
#pragma once

namespace LvEdEngine
{
	class Model3dBuilder;

	//--------------------------------------------------
	class CmdlModelFactory : public ResourceFactory
	{
	public:
		CmdlModelFactory(ID3D11Device* device);
		virtual Resource* CreateResource(Resource* def);
		virtual bool LoadResource( Resource* resource, const WCHAR * filename );
	protected:
		ID3D11Device* m_device;
		void ParseError( const char * fmt, ... );
		int m_parseErrors;
	};
};
