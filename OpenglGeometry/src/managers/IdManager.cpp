#include "IdManager.h"

IdManager::IdManager()
{
    
}

IdManager::~IdManager()
{
}

IdManager& IdManager::GetInstance()
{
    static IdManager instance;

    return instance;
}

ID IdManager::GetNewId()
{
    if (m_FreeIds.empty())
    {
        return m_CurrentFreeId++;
    }

    ID result = m_FreeIds.front();
	m_FreeIds.pop();
	return result;
}

void IdManager::FreeId(ID id)
{
	m_FreeIds.push(id);
}
