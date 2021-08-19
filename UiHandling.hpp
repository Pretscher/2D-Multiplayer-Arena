#pragma once

class UiHandling {
public:
	UiHandling(int frameRows, int frameCols);
	void updateLifeBar(int playerHP, int playerMaxHP);
	void draw();

	void drawAbilityUI(float qCooldownPercent);
private:
	int uiHeight;
	int frameRows, frameCols;
	int playerHP;
	int lifeBarWidth;
};