#pragma once

#include "../GUI.hpp"

namespace Components
{
	class AssetList : public Component
	{
	public:
		AssetList();

		static void AddAssetViewCallback(Game::XAssetType, const std::function<void(const std::string&)>& callback);
		static void AddViewButton(int id, Game::XAssetType type, const char* name);

		template <typename T>
		void add_asset_view(Game::XAssetType type, const std::function<bool(T*)>& draw_callback, ImVec2 min_size = ImVec2(0, 0))
		{
			static std::unordered_set<std::string> opened_assets;
			AddAssetViewCallback(type, [](const std::string& name)
			{
				opened_assets.insert(name);
			});

			GUI::RegisterCallback([=]()
			{
				for (auto i = opened_assets.begin(); i != opened_assets.end(); )
				{
					const auto& name = *i;
					const auto header = reinterpret_cast<T*>(Game::DB_FindXAssetHeader(type, name.data()).data);
					if (header == nullptr)
					{
						i = opened_assets.erase(i);
						continue;
					}

					if (min_size.x != 0 && min_size.y != 0)
					{
						auto& io = ImGui::GetIO();
						ImGui::SetNextWindowSizeConstraints(min_size, ImVec2(io.DisplaySize.x, io.DisplaySize.y));
					}

					auto is_open = true;
					if (ImGui::Begin(name.data(), &is_open))
					{
						if (!draw_callback(header))
						{
							is_open = false;
						}
					}
					ImGui::End();

					if (is_open)
					{
						++i;
					}
					else
					{
						i = opened_assets.erase(i);
					}
				}
			}, false);
		}

	private:
		static void draw_table_row(Game::XAssetType type, Game::XAssetEntry* entry, bool should_add_view_btn);
		static void draw_asset_type_list();
		static void draw_asset_list_filter(Game::XAssetType type);
		static constexpr auto get_table_flags();
		static void draw_asset_list_entries(Game::XAssetType type);
		static void draw_asset_list(Game::XAssetType type);
		static void render_window();
	};
}
