#pragma once
#include "../../D2Structs.h"
#include "../Module.h"
#include "../../Config.h"
#include "../../Drawing.h"

struct LevelList {
	unsigned int levelId;
	unsigned int x, y, act;
};

struct BaseSkill {
	WORD Skill;
	BYTE Level;
};

class MapNotify : public Module {
private:
	int monsterResistanceThreshold;
	int lkLinesColor;
	int mbMonColor;
	unsigned int maxGhostSelection;
	std::map<string, unsigned int> TextColorMap;

	bool badLevel[255];

public:
	MapNotify();

	void ReadConfig();
	void OnLoad();
	void OnUnload();

	void LoadConfig();

	void OnLoop();
	void OnDraw();
	void OnAutomapDraw();
	void OnGameJoin();
	void OnGamePacketRecv(BYTE* packet, bool* block);

	void OnKey(bool up, BYTE key, LPARAM lParam, bool* block);

	void RevealAct(int act);
	void RevealLevel(Level* level);
	void RevealRoom(Room2* room);

	static Level* GetLevel(Act* pAct, int level);
	static AutomapLayer* InitLayer(int level);
};
