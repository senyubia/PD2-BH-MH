#include "Maphack.h"
#include "../../BH.h"

Maphack::Maphack() : Module("Maphack") {
	for (int i = 0; i < 255; i++) {
		badLevel[i] = false;
	}

	badLevel[157] = true;
	badLevel[161] = true; // maps 157 and 161 halt the game while revealing for some reason
}

void Maphack::OnKey(bool up, BYTE key, LPARAM lParam, bool* block) {
	if (key == 0x39) { // VK_9
		*block = true;

		if (up) {
			UnitAny* unit = D2CLIENT_GetPlayerUnit();

			int act = unit->pAct->dwAct + 1;

			Level* map = unit->pPath->pRoom1->pRoom2->pLevel;
			int mapId = map->dwLevelNo;

			if (mapId >= 137) { // PD2 maps
				RevealLevel(map);
			}
			else { // LoD content
				RevealAct(act);
			}
		}
	}
}

void Maphack::RevealAct(int act) {
	if (act < 1 || act > 5)
		return;

	UnitAny* player = D2CLIENT_GetPlayerUnit();
	if (!player || !player->pAct)
		return;

	int actIds[6] = { 1, 40, 75, 103, 109, 137 };

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

void Maphack::RevealLevel(Level* level) {
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

void Maphack::RevealRoom(Room2* room) {
	for (PresetUnit* preset = room->pPreset; preset; preset = preset->pPresetNext) {
		int cellNo = -1;

		if (preset->dwType == UNIT_OBJECT) {
			// If it isn't special, check for a preset.
			if (cellNo == -1 && preset->dwTxtFileNo <= 572) {
				ObjectTxt* obj = D2COMMON_GetObjectTxt(preset->dwTxtFileNo);
				if (obj)
					cellNo = obj->nAutoMap; // Set the cell number then.
			}
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

Level* Maphack::GetLevel(Act* pAct, int level) {
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

AutomapLayer* Maphack::InitLayer(int level) {
	AutomapLayer2* layer = D2COMMON_GetLayer(level);

	if (!layer)
		return false;

	return (AutomapLayer*)D2CLIENT_InitAutomapLayer(layer->nLayerNo);
}
