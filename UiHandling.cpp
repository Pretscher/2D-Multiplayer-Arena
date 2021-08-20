#include "UiHandling.hpp"

#include "Renderer.hpp"
#include "Terrain.hpp"

UiHandling::UiHandling(int frameRows, int frameCols) {
	uiHeight = 900;
	this->frameRows = frameRows;
	this->frameCols = frameCols;

	Renderer::limitMouse(uiHeight, frameCols);
}

void UiHandling::updateLifeBar(int playerHP, int playerMaxHP) {
	if (playerHP > 0) {
		float widthMult = (float)playerHP / playerMaxHP;
		//cant draw lifebar here because rendering thread will be seperate => save lifebarwidth for drawing
		lifeBarWidth = ((float)frameCols - 100.0f) * widthMult;
	}
}

void UiHandling::draw() {
	Renderer::drawRect(uiHeight, 0, frameCols, frameRows - uiHeight, sf::Color(50, 50, 50, 255), true);
	Renderer::drawRect(uiHeight + 10, 50, (frameCols - 100), 40, sf::Color(10, 10, 10, 255), true);
	Renderer::drawRect(uiHeight + 10, 50, lifeBarWidth, 40, sf::Color(0, 150, 0, 255), true);
}
