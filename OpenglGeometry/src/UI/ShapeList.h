#pragma once
#include <array>
#include <string>
#include <vector>
#include "core/Base.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "scene/ObjectType.h"

namespace GUI
{
	class ShapeList
	{
	private:
		Ref<Scene> m_Scene;

		std::string m_Filter;
		std::array<bool, ObjectTypeCount> m_VisibleTypes;

		Entity m_RenameTarget;
		std::string m_RenameBuffer;
		bool m_RenameRequested = false;
		bool m_RenameNeedsFocus = false;

		void DrawActionRow();
		void DrawFilterRow();
		void DrawGroup(ObjectType type, const std::vector<Entity>& entities);
		void DrawRow(Entity entity);
		void DrawRenameField(Entity entity);
		void DrawRowContextMenu(Entity entity);

		bool PassesFilter(Entity entity) const;
		bool IsVisible(ObjectType type) const;

		void BeginRename(Entity entity);
		void CommitRename();
		void CancelRename();
	public:
		ShapeList(Ref<Scene> scene);
		void RequestRename();

		void Display();
	};
}
