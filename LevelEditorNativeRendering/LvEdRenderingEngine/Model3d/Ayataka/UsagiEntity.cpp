#include "UsagiEntity.h"

LvEdEngine::UsagiEntity::UsagiEntity()
{
	this->m_ready = false;
}

LvEdEngine::UsagiEntity::~UsagiEntity()
{

}

void LvEdEngine::UsagiEntity::SetSourceFileName(const std::wstring& name)
{
	m_source = name;
	m_path = name;
	// trim the filename from the path
	size_t lastSlash = m_path.find_last_of('\\');
	if (lastSlash != std::wstring::npos)
	{
		m_path = m_path.substr(0, lastSlash + 1);
	}
}
