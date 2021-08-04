
#include "Renderer.hpp"
#include "Terrain.hpp"

class UiHandling {
public:
	UiHandling(int frameRows, int frameCols) {
		uiHeight = 900;
		this->frameRows = frameRows;
		this->frameCols = frameCols;

		Renderer::limitMouse(uiHeight, frameCols);
	}

	void updateLifeBar(int playerHP, int playerMaxHP) {
		if (playerHP > 0) {
			float widthMult = (float)playerHP / playerMaxHP;
			//cant draw lifebar here because rendering thread will be seperate => save lifebarwidth for drawing
			lifeBarWidth = ((float)frameCols - 100.0f) * widthMult;
		}
	}

	void draw() {
		Renderer::drawRect(uiHeight, 0, frameCols, frameRows - uiHeight, sf::Color(50, 50, 50, 255), true);
		Renderer::drawRect(uiHeight + 50, 50, (frameCols - 100), 50, sf::Color(10, 10, 10, 255), true);
		Renderer::drawRect(uiHeight + 50, 50, lifeBarWidth, 50, sf::Color(0, 150, 0, 255), true);
	}

	void hardCodeTerrain() {
		
	}

private:
	int uiHeight;
	int frameRows, frameCols;
	int playerHP;
	int lifeBarWidth;
};