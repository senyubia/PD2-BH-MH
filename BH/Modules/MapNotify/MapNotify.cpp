#include "../../D2Ptrs.h"
#include "../../D2Helpers.h"
#include "../../D2Stubs.h"
#include "../../D2Intercepts.h"
#include "MapNotify.h"
#include "../../BH.h"
#include "../../Drawing.h"
#include "../Item/ItemDisplay.h"
#include "../../AsyncDrawBuffer.h"
#include "../Item/Item.h"
#include "../../Modules/GameSettings/GameSettings.h"

#pragma optimize( "", off)

using namespace Drawing;

DrawDirective automapDraw(true, 5);

MapNotify::MapNotify() : Module("MapNotify") {
	ReadConfig();
}

void MapNotify::LoadConfig() {
	ReadConfig();
}

void MapNotify::ReadConfig() {
	for (int i = 0; i < 255; i++) {
		badLevel[i] = false;
	}

	badLevel[157] = true;
	badLevel[161] = true; // maps 157 and 161 halt the game while revealing for some reason
}

void MapNotify::OnLoad() {
	ReadConfig();
}

void MapNotify::OnKey(bool up, BYTE key, LPARAM lParam, bool* block) {
	if (key == 0x39) { // VK_9
		*block = true;

		if (up) {
			UnitAny* unit = D2CLIENT_GetPlayerUnit();

			int act = unit->pAct->dwAct + 1;

			Level* map = unit->pPath->pRoom1->pRoom2->pLevel;
			int mapId = map->dwLevelNo;

			BYTE difficulty = D2CLIENT_GetDifficulty();

			if (mapId >= 137 && difficulty == 2) {
				RevealAct(6);
			}
			else if (mapId >= 137) {
				RevealLevel(map);
			}
			else {
				RevealAct(act);
			}
		}
	}
}

void MapNotify::OnUnload() {
}

void MapNotify::OnLoop() {
}

Act* lastAct = NULL;

void MapNotify::OnDraw() {
	UnitAny* player = D2CLIENT_GetPlayerUnit();

	if (!player || !player->pAct || player->pPath->pRoom1->pRoom2->pLevel->dwLevelNo == 0)
		return;
	// We're looping over all items and setting 2 flags:
	// UNITFLAG_NO_EXPERIENCE - Whether the item has been checked for a drop notification (to prevent checking it again)
	// UNITFLAG_REVEALED      - Whether the item should be notified and drawn on the automap
	// To my knowledge these flags arent typically used on items. So we can abuse them for our own use.
	for (Room1* room1 = player->pAct->pRoom1; room1; room1 = room1->pRoomNext) {
		for (UnitAny* unit = room1->pUnitFirst; unit; unit = unit->pListNext) {
			if (unit->dwType == UNIT_ITEM && (unit->dwFlags & UNITFLAG_NO_EXPERIENCE) == 0x0) {
				DWORD dwFlags = unit->pItemData->dwFlags;
				char* code = D2COMMON_GetItemText(unit->dwTxtFileNo)->szCode;
				UnitItemInfo uInfo;
				uInfo.item = unit;
				uInfo.itemCode[0] = code[0];
				uInfo.itemCode[1] = code[1] != ' ' ? code[1] : 0;
				uInfo.itemCode[2] = code[2] != ' ' ? code[2] : 0;
				uInfo.itemCode[3] = code[3] != ' ' ? code[3] : 0;
				uInfo.itemCode[4] = 0;
				if (ItemAttributeMap.find(uInfo.itemCode) != ItemAttributeMap.end()) {
					uInfo.attrs = ItemAttributeMap[uInfo.itemCode];
					for (vector<Rule*>::iterator it = MapRuleList.begin(); it != MapRuleList.end(); it++) {
						int filterLevel = Item::GetFilterLevel();
						if (filterLevel != 0 && (*it)->action.pingLevel < filterLevel && (*it)->action.pingLevel != -1) continue;

						if ((*it)->Evaluate(&uInfo))
						{
							if ((unit->dwFlags & UNITFLAG_REVEALED) == 0x0 &&
								App.lootfilter.enableFilter.value && App.lootfilter.detailedNotifications.value != 0 &&
								(App.lootfilter.detailedNotifications.value == 1 || (dwFlags & ITEM_NEW)))
							{
								SoundsTxt* pSoundsTxt = *p_D2CLIENT_SoundsTxt;
								int soundID = (*it)->action.soundID;
								if (soundID > 0 && soundID < *p_D2CLIENT_SoundRecords &&
									(soundID < 4657 || soundID > 4698) &&	// Don't allow music
									App.lootfilter.dropSounds.value && pSoundsTxt)
								{
									pSoundsTxt = pSoundsTxt + soundID;
									if (pSoundsTxt && pSoundsTxt->loop == 0)
									{
										D2CLIENT_PlaySound_STUB(soundID, NULL, 0, 0, 0);
									}
								}

								std::string itemName = GetItemName(unit);
								regex trimName("^\\s*(?:(?:\\s*?)(ÿc[0123456789;:]))*\\s*(.*?\\S)\\s*(?:ÿc[0123456789;:])*\\s*$");	// Matches on leading/trailing spaces (skips most color codes)
								itemName = regex_replace(itemName, trimName, "$1$2");												// Trims the matched spaces from notifications
								size_t start_pos = 0;
								while ((start_pos = itemName.find('\n', start_pos)) != std::string::npos) {
									itemName.replace(start_pos, 1, " - ");
									start_pos += 3;
								}
								PrintText(ItemColorFromQuality(unit->pItemData->dwQuality), "%s", itemName.c_str());
							}
							unit->dwFlags |= UNITFLAG_REVEALED;
							break;
						}
					}
				}
			}
			unit->dwFlags |= UNITFLAG_NO_EXPERIENCE;
		}
	}
}

void MapNotify::OnAutomapDraw() {
	UnitAny* player = D2CLIENT_GetPlayerUnit();

	if (!player || !player->pAct || player->pPath->pRoom1->pRoom2->pLevel->dwLevelNo == 0)
		return;

	if (lastAct != player->pAct) {
		lastAct = player->pAct;
		automapDraw.forceUpdate();
	}

	automapDraw.draw([=](AsyncDrawBuffer& automapBuffer) -> void {
		POINT MyPos;
		Drawing::Hook::ScreenToAutomap(&MyPos,
			D2CLIENT_GetUnitX(D2CLIENT_GetPlayerUnit()),
			D2CLIENT_GetUnitY(D2CLIENT_GetPlayerUnit()));
		for (Room1* room1 = player->pAct->pRoom1; room1; room1 = room1->pRoomNext) {
			for (UnitAny* unit = room1->pUnitFirst; unit; unit = unit->pListNext) {
				DWORD xPos, yPos;
				if (unit->dwType == UNIT_ITEM && (unit->dwFlags & UNITFLAG_REVEALED) == UNITFLAG_REVEALED) {
					char* code = D2COMMON_GetItemText(unit->dwTxtFileNo)->szCode;
					UnitItemInfo uInfo;
					uInfo.item = unit;
					uInfo.itemCode[0] = code[0];
					uInfo.itemCode[1] = code[1] != ' ' ? code[1] : 0;
					uInfo.itemCode[2] = code[2] != ' ' ? code[2] : 0;
					uInfo.itemCode[3] = code[3] != ' ' ? code[3] : 0;
					uInfo.itemCode[4] = 0;
					if (ItemAttributeMap.find(uInfo.itemCode) != ItemAttributeMap.end()) {
						uInfo.attrs = ItemAttributeMap[uInfo.itemCode];
						const vector<Action> actions = map_action_cache.Get(&uInfo);
						for (auto& action : actions) {
							auto color = action.colorOnMap;
							auto borderColor = action.borderColor;
							auto dotColor = action.dotColor;
							auto pxColor = action.pxColor;
							auto lineColor = action.lineColor;
							xPos = unit->pItemPath->dwPosX;
							yPos = unit->pItemPath->dwPosY;
							automapBuffer.push_top_layer(
								[color, unit, xPos, yPos, MyPos, borderColor, dotColor, pxColor, lineColor]()->void {
									POINT automapLoc;
									Drawing::Hook::ScreenToAutomap(&automapLoc, xPos, yPos);
									if (borderColor != UNDEFINED_COLOR)
										Drawing::Boxhook::Draw(automapLoc.x - 4, automapLoc.y - 4, 8, 8, borderColor, Drawing::BTHighlight);
									if (color != UNDEFINED_COLOR)
										Drawing::Boxhook::Draw(automapLoc.x - 3, automapLoc.y - 3, 6, 6, color, Drawing::BTHighlight);
									if (dotColor != UNDEFINED_COLOR)
										Drawing::Boxhook::Draw(automapLoc.x - 2, automapLoc.y - 2, 4, 4, dotColor, Drawing::BTHighlight);
									if (pxColor != UNDEFINED_COLOR)
										Drawing::Boxhook::Draw(automapLoc.x - 1, automapLoc.y - 1, 2, 2, pxColor, Drawing::BTHighlight);
								});
							if (action.stopProcessing) break;
						}
					}
					else {
						HandleUnknownItemCode(uInfo.itemCode, "on map");
					}
				}
			}
		}
		});
}

void MapNotify::OnGameJoin() {
}

void Squelch(DWORD Id, BYTE button) {
	LPBYTE aPacket = new BYTE[7];	//create packet
	*(BYTE*)&aPacket[0] = 0x5d;
	*(BYTE*)&aPacket[1] = button;
	*(BYTE*)&aPacket[2] = 1;
	*(DWORD*)&aPacket[3] = Id;
	D2NET_SendPacket(7, 0, aPacket);

	delete[] aPacket;	//clearing up data

	return;
}

void MapNotify::OnGamePacketRecv(BYTE* packet, bool* block) {
}

void MapNotify::RevealAct(int act) {
	if (act < 1 || act > 6)
		return;

	UnitAny* player = D2CLIENT_GetPlayerUnit();
	if (!player || !player->pAct)
		return;

	int actIds[7] = { 1, 40, 75, 103, 109, 137, 195 };

	Act* pAct = D2COMMON_LoadAct(act - 1, player->pAct->dwMapSeed, *p_D2CLIENT_ExpCharFlag, 0, D2CLIENT_GetDifficulty(), NULL, actIds[act - 1], D2CLIENT_LoadAct_1, D2CLIENT_LoadAct_2);
	if (!pAct || !pAct->pMisc)
		return;

	for (int level = actIds[act - 1]; level < actIds[act]; level++) {
		Level* pLevel = GetLevel(pAct, level);

		if (!pLevel)
			continue;

		if (!pLevel->pRoom2First)
			D2COMMON_InitLevel(pLevel);

		RevealLevel(pLevel);
	}

	InitLayer(player->pPath->pRoom1->pRoom2->pLevel->dwLevelNo);
	D2COMMON_UnloadAct(pAct);
}

void MapNotify::RevealLevel(Level* level) {
	if (!level || level->dwLevelNo < 0 || level->dwLevelNo > 255 || badLevel[level->dwLevelNo])
		return;

	InitLayer(level->dwLevelNo);

	for (Room2* room = level->pRoom2First; room; room = room->pRoom2Next) {
		bool roomData = false;

		if (!room->pRoom1) {
			D2COMMON_AddRoomData(level->pMisc->pAct, level->dwLevelNo, room->dwPosX, room->dwPosY, room->pRoom1);
			roomData = true;
		}

		if (!room->pRoom1)
			continue;

		D2CLIENT_RevealAutomapRoom(room->pRoom1, TRUE, *p_D2CLIENT_AutomapLayer);

		RevealRoom(room);

		if (roomData)
			D2COMMON_RemoveRoomData(level->pMisc->pAct, level->dwLevelNo, room->dwPosX, room->dwPosY, room->pRoom1);
	}
}

void MapNotify::RevealRoom(Room2* room) {
	for (PresetUnit* preset = room->pPreset; preset; preset = preset->pPresetNext) {
		int cellNo = -1;

		if (preset->dwType == UNIT_MONSTER) {
			// Izual Check
			if (preset->dwTxtFileNo == 256)
				cellNo = 300;

			// Hephasto Check
			if (preset->dwTxtFileNo == 745)
				cellNo = 745;
		}
		else if (preset->dwType == UNIT_OBJECT) {
			// Uber Chest in Lower Kurast Check
			if (preset->dwTxtFileNo == 580 && room->pLevel->dwLevelNo == MAP_A3_LOWER_KURAST)
				cellNo = 318;

			// Countess Chest Check
			if (preset->dwTxtFileNo == 371)
				cellNo = 301;

			// Act 2 Orifice Check
			else if (preset->dwTxtFileNo == 152)
				cellNo = 300;

			// Frozen Anya Check
			else if (preset->dwTxtFileNo == 460)
				cellNo = 1468;

			// Canyon / Arcane Waypoint Check
			if ((preset->dwTxtFileNo == 402) && (room->pLevel->dwLevelNo == 46))
				cellNo = 0;

			// Hell Forge Check
			if (preset->dwTxtFileNo == 376)
				cellNo = 376;

			// If it isn't special, check for a preset.
			if (cellNo == -1 && preset->dwTxtFileNo <= 572) {
				ObjectTxt* obj = D2COMMON_GetObjectTxt(preset->dwTxtFileNo);
				if (obj)
					cellNo = obj->nAutoMap; // Set the cell number then.
			}
		}
		else if (preset->dwType == UNIT_TILE) {
			LevelList* level = new LevelList;
			for (RoomTile* tile = room->pRoomTiles; tile; tile = tile->pNext) {
				if (*(tile->nNum) == preset->dwTxtFileNo) {
					level->levelId = tile->pRoom2->pLevel->dwLevelNo;
					break;
				}
			}
			level->x = (preset->dwPosX + (room->dwPosX * 5));
			level->y = (preset->dwPosY + (room->dwPosY * 5));
			level->act = room->pLevel->pMisc->pAct->dwAct;
		}

		if ((cellNo > 0) && (cellNo < 1258)) {
			AutomapCell* cell = D2CLIENT_NewAutomapCell();

			cell->nCellNo = cellNo;
			int x = (preset->dwPosX + (room->dwPosX * 5));
			int y = (preset->dwPosY + (room->dwPosY * 5));
			cell->xPixel = (((x - y) * 16) / 10) + 1;
			cell->yPixel = (((y + x) * 8) / 10) - 3;

			D2CLIENT_AddAutomapCell(cell, &((*p_D2CLIENT_AutomapLayer)->pObjects));
		}
	}
	return;
}

Level* MapNotify::GetLevel(Act* pAct, int level) {
	if (level < 0 || !pAct)
		return NULL;

	for (Level* pLevel = pAct->pMisc->pLevelFirst; pLevel; pLevel = pLevel->pNextLevel) {
		if (!pLevel)
			break;

		if (pLevel->dwLevelNo == level && pLevel->dwPosX > 0)
			return pLevel;
	}

	return D2COMMON_GetLevel(pAct->pMisc, level);
}

AutomapLayer* MapNotify::InitLayer(int level) {
	AutomapLayer2* layer = D2COMMON_GetLayer(level);

	if (!layer)
		return false;

	return (AutomapLayer*)D2CLIENT_InitAutomapLayer(layer->nLayerNo);
}

#pragma optimize( "", on)
