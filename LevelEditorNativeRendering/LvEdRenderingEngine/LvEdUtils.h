//Copyright � 2014 Sony Computer Entertainment America LLC. See License.txt.

#pragma once

#include "Renderer\RenderableNodeSet.h"
#include <unordered_set>
#include "Core/Object.h"

using namespace LvEdEngine;

typedef std::unordered_set<ObjectGUID> Selection;
typedef std::vector<Object*> ObjectList;

class FindGobsByType : public QueryFunctor
{
public:
    FindGobsByType(const char* typeName)
        : m_typeName(typeName)   { }

    virtual bool operator() (const GameObject* gob)
    {
        if(strcmp(m_typeName,gob->ClassName()) == 0)
        {
            Object* obj = (Object*) gob;
            Gobs.push_back(obj);
        }
        return true;
    }
    ObjectList Gobs;
    const char* m_typeName;
};

