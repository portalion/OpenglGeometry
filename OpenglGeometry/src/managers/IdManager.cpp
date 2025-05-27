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

unsigned int IdManager::GetNewId(std::string name)
{
    ids[name] = ids[name] + 1;

    return ids[name];
}

void IdManager::SaveId(std::string name, unsigned int id)
{
    ids[name] = std::max(id, ids[name]);
}

void IdManager::FreeId(std::string name, unsigned int id)
{
}

void IdManager::ClearIds()
{
    ids.clear();
}
