#pragma once
#include <unordered_map>
#include <string>
#include <set>
class IdManager
{
private:
	IdManager();

	std::unordered_map<std::string, unsigned int> ids;
public:
	~IdManager();
	IdManager(IdManager& other) = delete;
	void operator=(const IdManager&) = delete;

	static IdManager& GetInstance();

	unsigned int GetNewId(std::string name);
	void SaveId(std::string name, unsigned int id);
	void FreeId(std::string name, unsigned int id);
	void ClearIds();
};

