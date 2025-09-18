#ifdef _DEBUG

#include "AssetList.hpp"

namespace Components
{
	namespace
	{
		bool strstr_lower(const char* a, const char* b)
		{
			const char* a_ = a;
			const char* b_ = b;

			while (*a_ != '\0' && *b_ != '\0')
			{
				if (*b_ == '*' || std::tolower(*a_) == std::tolower(*b_))
				{
					b_++;
				}
				else
				{
					b_ = b;
				}

				a_++;
			}

			return *b_ == '\0';
		}
	}

	bool shown_assets[Game::XAssetType::ASSET_TYPE_COUNT]{};
	std::string asset_type_filter;
	std::string assets_name_filter[Game::XAssetType::ASSET_TYPE_COUNT];
	std::string assets_value_filter[Game::XAssetType::ASSET_TYPE_COUNT];
	std::string zone_name_filter[Game::XAssetType::ASSET_TYPE_COUNT];

	std::unordered_map<Game::XAssetType, std::function<void(const std::string&)>> asset_view_callbacks;

	bool default_only[Game::ASSET_TYPE_COUNT]{};
	int asset_count[Game::ASSET_TYPE_COUNT]{};
	bool disabled_zones[Game::ASSET_TYPE_COUNT][0x100]{};
	bool show_asset_zone = true;

	AssetList::AssetList()
	{
		if (Dedicated::IsEnabled())
		{
			return;
		}

		GUI::RegisterMenu("asset_list", "Asset List", render_window);
	}

	void AssetList::AddViewButton(int id, Game::XAssetType type, const char* name)
	{
		if (asset_view_callbacks.contains(type))
		{
			ImGui::SameLine();
			ImGui::PushID(id);
			if (ImGui::Button("view"))
			{
				asset_view_callbacks.at(type)(name);
			}
			ImGui::PopID();
		}
	}

	void AssetList::AddAssetViewCallback(Game::XAssetType type, const std::function<void(const std::string&)>& callback)
	{
		asset_view_callbacks.insert(std::make_pair(type, callback));
	}

	void AssetList::draw_table_row(Game::XAssetType type, Game::XAssetEntry* entry, bool should_add_view_btn)
	{
		auto* asset = &entry->asset;
		auto asset_name = Game::DB_GetXAssetName(asset);
		const auto is_default = entry->zoneIndex == 0;

		if (asset_name[0] == '\0' || disabled_zones[type][entry->zoneIndex] || default_only[type] && !is_default)
		{
			return;
		}

		if (is_default)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.43f, 0.15f, 0.15f, 1.f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.98f, 0.26f, 0.26f, 1.f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.98f, 0.06f, 0.06f, 1.f));
		}

		const auto _0 = gsl::finally([&]
		{
			if (is_default)
			{
				ImGui::PopStyleColor(3);
			}
		});

		auto col_index = 0;
		if (!strstr_lower(asset_name, assets_name_filter[type].data()))
		{
			return;
		}

		if (type == Game::ASSET_TYPE_LOCALIZE_ENTRY)
		{
			if (!strstr_lower(entry->asset.header.localize->value, assets_value_filter[type].data()))
			{
				return;
			}
		}

		ImGui::TableNextRow();

		if (should_add_view_btn)
		{
			ImGui::TableSetColumnIndex(col_index++);
			ImGui::PushID(asset_count[type]);
			if (ImGui::Button("view"))
			{
				asset_view_callbacks.at(type)(asset_name);
			}
			ImGui::PopID();
		}

		if (show_asset_zone)
		{
			ImGui::TableSetColumnIndex(col_index++);
			if (entry->zoneIndex > 0)
			{
				// fastfiles::get_zone_name((int)entry->zoneIndex)
				ImGui::Text("TODO: zone name");
			}
			else
			{
				ImGui::Text("default");
			}
		}

		ImGui::TableSetColumnIndex(col_index++);

		if (ImGui::Button(asset_name))
		{
			// TODO: clipboard
			//gui::copy_to_clipboard(asset_name);
		}

		if (type == Game::ASSET_TYPE_LOCALIZE_ENTRY)
		{
			ImGui::TableSetColumnIndex(col_index++);

			if (ImGui::Button(entry->asset.header.localize->value))
			{
				// TODO: clipboard
				//gui::copy_to_clipboard(entry->asset.header.localize->value);
			}
		}
	}

	void AssetList::draw_asset_type_list()
	{
		if (ImGui::TreeNode("loaded zones"))
		{
			for (int i = 0; i < 32; ++i)
			{
				const auto name = Game::g_zones[i].name;
				if (ImGui::Button(name))
				{
					// TODO: clipboard
					//gui::copy_to_clipboard(name);
				}
			}

			ImGui::TreePop();
		}

		ImGui::Checkbox("show asset zone", &show_asset_zone);
		ImGui::InputText("asset type", &asset_type_filter);
		ImGui::BeginChild("asset type list");

		for (auto i = 0; i < Game::XAssetType::ASSET_TYPE_COUNT; i++)
		{
			const auto name = Game::g_assetNames[i];
			const auto type = static_cast<Game::XAssetType>(i);

			if (asset_type_filter.size() == 0 || strstr_lower(name, asset_type_filter.data()))
			{
				ImGui::Checkbox(name, &shown_assets[type]);
			}
		}

		ImGui::EndChild();
	}

	void AssetList::draw_asset_list_filter(const Game::XAssetType type)
	{
		ImGui::Text("count: %i / %i", asset_count[type], Game::g_poolSize[type]);
		ImGui::InputText("name", &assets_name_filter[type]);

		if (type == Game::ASSET_TYPE_LOCALIZE_ENTRY)
		{
			ImGui::InputText("value", &assets_value_filter[type]);
		}

		/*
		if (ImGui::InputText("zone name", &zone_name_filter[type]))
		{
			for (auto zone = 0u; zone <= *Game::g_zoneCount; zone++)
			{
				const auto zone_name = fastfiles::get_zone_name(zone);
				disabled_zones[type][zone] = !strstr_lower(zone_name, zone_name_filter[type].data());
			}
		}
		*/

		ImGui::Checkbox("default assets only", &default_only[type]);
	}

	constexpr auto AssetList::get_table_flags()
	{
		constexpr auto flags =
			ImGuiTableFlags_BordersInnerH |
			ImGuiTableFlags_BordersOuterH |
			ImGuiTableFlags_BordersInnerV |
			ImGuiTableFlags_BordersOuterV |
			ImGuiTableFlags_RowBg |
			ImGuiTableFlags_ScrollX |
			ImGuiTableFlags_ScrollY;

		return flags;
	}

	void AssetList::draw_asset_list_entries(const Game::XAssetType type)
	{
		const auto should_add_view_btn = asset_view_callbacks.contains(type);
		constexpr auto flags = get_table_flags();

		asset_count[type] = 0;

		ImGui::BeginChild("assets list");

		auto column_count = 1;
		column_count += should_add_view_btn;
		column_count += show_asset_zone;
		column_count += type == Game::ASSET_TYPE_LOCALIZE_ENTRY;

		if (ImGui::BeginTable("assets", column_count, flags))
		{
			DB_EnumXAssetEntries(type, [&](Game::XAssetEntry* entry)
			{
				asset_count[type]++;
				draw_table_row(type, entry, should_add_view_btn);
			}, true);

			ImGui::EndTable();
		}

		ImGui::EndChild();
	}

	void AssetList::draw_asset_list(const Game::XAssetType type)
	{
		if (!shown_assets[type])
		{
			return;
		}

		const auto name = Game::g_assetNames[type];

		auto& io = ImGui::GetIO();
		ImGui::SetNextWindowSizeConstraints(ImVec2(500, 500), io.DisplaySize);
		ImGui::Begin(name, &shown_assets[type]);

		draw_asset_list_filter(type);
		draw_asset_list_entries(type);

		ImGui::End();
	}

	void AssetList::render_window()
	{
		static auto* enabled = GUI::GetEnabledMenu("asset_list");
		ImGui::Begin("Asset list", enabled);

		draw_asset_type_list();

		for (auto i = 0; i < Game::XAssetType::ASSET_TYPE_COUNT; i++)
		{
			draw_asset_list(static_cast<Game::XAssetType>(i));
		}

		ImGui::End();
	}
}

#endif
