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
    return m_NextFreeId++;
}

void IdManager::ReserveUpTo(ID id)
{
    if (id >= m_NextFreeId)
    {
        m_NextFreeId = id + 1;
    }
}
