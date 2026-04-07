#pragma once
#include "core/Base.h"
#include "interfaces/ISystem.h"
#include <vector>
#include <string>
#include <interfaces/IPopup.h>
#include <scene/Scene.h>

class PopupSystem : public ISystem
{
private:
	Ref<IPopup> m_OpenedPopup;
	std::vector<Ref<IPopup>> m_RegisteredPopups;
public:
	PopupSystem(Ref<Scene> scene);

	void Process() override;
};