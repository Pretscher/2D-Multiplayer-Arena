#include "UiHandling.hpp"

#include "Renderer.hpp"
#include "Terrain.hpp"

sf::Texture* orb;
sf::Texture uiPanel;
UiHandling::UiHandling(int frameRows, int frameCols) {
	uiHeight = 1080;
	this->frameRows = frameRows;
	this->frameCols = frameCols;

	Renderer::limitMouse(uiHeight, frameCols);

	sf::Texture wall;
	sf::Texture ground;

	orb = new sf::Texture [6];

	orb [0] = Renderer::loadTexture("Textures/Stat orb/itsmars_orb_fill.png", false);
	orb [1] = Renderer::loadTexture("Textures/UI/Uipanel.png", false);
	orb [2] = Renderer::loadTexture("Textures/Stat orb/itsmars_orb_back1 _flipped.png", false);

	orb [3] = Renderer::loadTexture("Textures/Stat orb/itsmars_orb_shadow.png", false);
	orb [4] = Renderer::loadTexture("Textures/Stat orb/itsmars_orb_highlight.png", false);
	orb [5] = Renderer::loadTexture("Textures/Stat orb/itsmars_orb_border.png", false);
}

int lifebarSize = 300;
void UiHandling::updateLifeBar(int playerHP, int playerMaxHP) {
	if (playerHP > 0) {
		float widthMult = (float)playerHP / playerMaxHP;
		//cant draw lifebar here because rendering thread will be seperate => save lifebarwidth for drawing
		lifeBarWidth = ((float) lifebarSize) * widthMult;
	}
	else {
		lifeBarWidth = 0;
	}
}



void UiHandling::draw() {

	for (int i = 0; i < 6; i++) {
		if (i == 1) {
			Renderer::drawRect(780 + lifebarSize - lifeBarWidth, 1600, 300, 300, sf::Color(100, 0, 0, 255), true);

			float uiScale = 1.49;
			Renderer::drawRectWithTexture(890, 1132, 530 * uiScale, 129 * uiScale, orb [i], true);
		}
		else if (i == 2) {
			Renderer::drawRectWithTexture(750, 1575, 350, 300, orb [i], true);
		}
		else {
			Renderer::drawRectWithTexture(780, 1600, 300, 300, orb [i], true);
		}
	}
}
