#pragma once
#include <cstdint>

using ID = std::uint32_t;
class IdManager
{
private:
	IdManager();

	ID m_NextFreeId = 1;
public:
	~IdManager();
	IdManager(IdManager& other) = delete;
	void operator=(const IdManager&) = delete;

	static IdManager& GetInstance();

	ID GetNewId();
	void ReserveUpTo(ID id);
};
