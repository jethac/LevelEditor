#pragma once

#include <string>
#include <vector>
#include <map>
#include <D3D11.h>

#include "../../Core/WinHeaders.h"
#include "../../Core/NonCopyable.h"
#include "../../VectorMath/V3dMath.h"
#include "../../VectorMath/CollisionPrimitives.h"
#include "../../Renderer/RenderEnums.h"
#include "../../Renderer/Resource.h"
#include "../../Renderer/Model.h"

namespace LvEdEngine
{

class UsagiModel
{
	// @implementme...
};

class UsagiEntity : public Resource
{
public:
	UsagiEntity();
	virtual ~UsagiEntity();

	// IResource methods.
	virtual ResourceTypeEnum GetType() { return ResourceType::UsagiEntity; }

	void SetSourceFileName(const std::wstring& name);
	const std::wstring & GetSourceFileName() { return m_source; }


protected:
	std::wstring m_source;
	std::wstring m_path;
};

}