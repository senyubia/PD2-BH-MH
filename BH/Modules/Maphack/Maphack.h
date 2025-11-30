#pragma once
#include "../../D2Structs.h"
#include "../Module.h"

class Maphack : public Module {
private:
	bool badLevel[255];

public:
	Maphack();

	void OnKey(bool up, BYTE key, LPARAM lParam, bool* block);

	void RevealAct(int act);
	void RevealLevel(Level* level);
	void RevealRoom(Room2* room);

	static Level* GetLevel(Act* pAct, int level);
	static AutomapLayer* InitLayer(int level);
};
