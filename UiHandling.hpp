#pragma once

class UiHandling {
public:
	UiHandling(int frameYs, int frameXs);
	void updateLifeBar(int playerHP, int playerMaxHP);
	void draw();

	void drawAbilityUI(float qCooldownPercent);
private:
	int uiHeight;
	int frameYs, frameXs;
	int playerHP;
	int lifeBarWidth;
};