#pragma once
#include "interfaces/ISystem.h"

class LoggerSystem : public ISystem
{
private:
	bool m_Show = true;
public:
	void Process() override;
};
