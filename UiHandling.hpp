#pragma once

class UiHandling {
public:
	UiHandling();
	void updateLifeBar(int playerHP, int playerMaxHP);
	void draw();

	void drawAbilityUI(float qCooldownPercent);
private:
	int uiHeight;
	int frameHeight, frameWidth;
	int playerHP;
	int lifeBarWidth;
};