#include "stdafx.h"
#include <stdint.h>

#include <ShellApi.h>

#include <string>
#include <functional>


#include <GWCA\Constants\Constants.h>

#include <GWCA\GameContainers\Array.h>
#include <GWCA\GameContainers\GamePos.h>

#include <GWCA\Packets\StoC.h>

#include <GWCA\GameEntities\Map.h>

#include <GWCA\Managers\UIMgr.h>
#include <GWCA\Managers\MapMgr.h>
#include <GWCA\Managers\ChatMgr.h>
#include <GWCA\Managers\AgentMgr.h>
#include <GWCA\Managers\SkillbarMgr.h>
#include <GWCA\Managers\StoCMgr.h>

#include <logger.h>
#include "GuiUtils.h"
#include <Modules\Resources.h>
#include "FactionLeaderboardWindow.h"



void FactionLeaderboardWindow::Initialize() {
	ToolboxWindow::Initialize();
	leaderboard.resize(15);
	Resources::Instance().LoadTextureAsync(&button_texture, Resources::GetPath(L"img/icons", L"list.png"), IDB_Icon_list);
	GW::StoC::AddCallback<GW::Packet::StoC::TownAllianceObject>(
		[this](GW::Packet::StoC::TownAllianceObject *pak) -> bool {
		LeaderboardEntry leaderboardEntry = {
			pak->map_id,
			pak->rank,
			pak->allegiance,
			pak->faction,
			pak->name,
			pak->tag
		};
		if (leaderboard.size() <= leaderboardEntry.rank)
			leaderboard.resize(leaderboardEntry.rank + 1);
        leaderboard.at(leaderboardEntry.rank) = leaderboardEntry;
		return false;
	});
}
void FactionLeaderboardWindow::Draw(IDirect3DDevice9* pDevice) {
	if (!visible)
		return;
	ImGui::SetNextWindowPosCenter(ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 250), ImGuiSetCond_FirstUseEver);
	if (!ImGui::Begin(Name(), GetVisiblePtr(), GetWinFlags()))
		return ImGui::End();
	float offset = 0.0f;
	const float tiny_text_width = 50.0f * ImGui::GetIO().FontGlobalScale;
	const float short_text_width = 80.0f * ImGui::GetIO().FontGlobalScale;
	const float avail_width = ImGui::GetContentRegionAvailWidth();
	const float long_text_width = 200.0f * ImGui::GetIO().FontGlobalScale;
	ImGui::Text("Rank");
	ImGui::SameLine(offset += tiny_text_width);
	ImGui::Text("Allegiance");
	ImGui::SameLine(offset += short_text_width);
	ImGui::Text("Faction");
	ImGui::SameLine(offset += short_text_width);
	ImGui::Text("Outpost");
	ImGui::SameLine(offset += long_text_width);
	ImGui::Text("Guild");
	ImGui::Separator();
	bool has_entries = 0;
	for (size_t i = 0; i < leaderboard.size(); i++) {
		LeaderboardEntry* e = &leaderboard[i];
		if (!e->initialised)
			continue;
		has_entries = 1;
		offset = 0.0f;
		if (e->map_name[0] == 0) {
			// Try to load map name in.
			GW::AreaInfo* info = GW::Map::GetMapInfo((GW::Constants::MapID)e->map_id);
			if (info && GW::UI::UInt32ToEncStr(info->name_id, e->map_name_enc, 256))
				GW::UI::AsyncDecodeStr(e->map_name_enc, e->map_name, 256);
		}
		ImGui::Text("%d",e->rank);
		ImGui::SameLine(offset += tiny_text_width);
		ImGui::Text(e->allegiance == 1 ? "Luxon" : "Kurzick");
		ImGui::SameLine(offset += short_text_width);
		ImGui::Text("%d",e->faction);
		ImGui::SameLine(offset += short_text_width);
		ImGui::Text(e->map_name);
		ImGui::SameLine(offset += long_text_width);
		std::string s = e->guild_str;
		s += " [";
		s += e->tag_str;
		s += "]";
		const char* sc = s.c_str();
		ImGui::Text(sc);
		ImGui::PushID(e->map_id);
		ImGui::SameLine(offset = avail_width - tiny_text_width);
		if (ImGui::Button("Wiki",ImVec2(tiny_text_width,0))) {
			ShellExecuteW(NULL, L"open", GuiUtils::ToWstr(e->guild_wiki_url).c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
		ImGui::PopID();
	}
	if (!has_entries) {
		ImVec2 w = ImGui::CalcTextSize("Enter a Canthan outpost to see data");
		ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2);
		ImGui::SetCursorPosX(avail_width / 2 - (w.x / 2));
		ImGui::Text("Enter a Canthan outpost to see data");
	}
	return ImGui::End();
}
