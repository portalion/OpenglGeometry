#pragma once
#include <string>
#include <queue>

using ID = long long;

class IdManager
{
private:
	IdManager();

	ID m_CurrentFreeId = 0;
	std::queue<ID> m_FreeIds;
public:
	~IdManager();
	IdManager(IdManager& other) = delete;
	void operator=(const IdManager&) = delete;

	static IdManager& GetInstance();

	ID GetNewId();
	void FreeId(ID id);
};

