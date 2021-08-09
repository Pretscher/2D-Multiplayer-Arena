#pragma once

class UiHandling {
public:
	UiHandling(int frameRows, int frameCols);
	void updateLifeBar(int playerHP, int playerMaxHP);
	void draw();

private:
	int uiHeight;
	int frameRows, frameCols;
	int playerHP;
	int lifeBarWidth;
};