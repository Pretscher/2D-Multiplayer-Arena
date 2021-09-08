#include "VladW.hpp"
#include "GlobalRecources.hpp"
#include "Utils.hpp"
static int i_onCDPhase = 0;
static int i_addToNetworkPhase = 0;
static int i_abilityIndex = 1;

VladW::VladW(int i_myPlayerIndex) : Ability(i_myPlayerIndex, false, i_onCDPhase, i_addToNetworkPhase, i_abilityIndex) {
	endPhaseAfterMS(2000);
	GlobalRecources::players[myPlayerIndex]->inVladW = true;
	GlobalRecources::players[myPlayerIndex]->targetAble = false;
}
//constructor through networking
VladW::VladW(int i_myPlayerIndex, int i_timeInPhase) : Ability(i_myPlayerIndex, true, i_onCDPhase, i_addToNetworkPhase, i_abilityIndex) {
	endPhaseAfterMS(2000 - i_timeInPhase);
	GlobalRecources::players[myPlayerIndex]->inVladW = true;
	GlobalRecources::players[myPlayerIndex]->targetAble = false;
}

void VladW::execute0() {
	Player* myPlayer = GlobalRecources::players[myPlayerIndex];
	for (int i = 0; i < GlobalRecources::playerCount; i++) {
		if (i != myPlayerIndex) {
			Player* current = GlobalRecources::players[i];
			if (current->targetAble == true) {
				if (Utils::collisionRectCircle(current->getCol(), current->getRow(), current->getWidth(), current->getHeight(),
					myPlayer->getCol() + (myPlayer->getWidth() / 2) - range, myPlayer->getRow() + (myPlayer->getHeight() / 2) - range, range, 10) == true) {
					current->setHp(current->getHp() - damage);
				}
			}
		}
	}
}

void VladW::draw0() {
	Player* current = GlobalRecources::players[myPlayerIndex];
	Renderer::drawCircle(current->getRow() + (current->getHeight() / 2) - range,
		current->getCol() + (current->getWidth() / 2) - range, range, sf::Color(150, 0, 0, 255), true, 0, false);
}

void VladW::init1() {
	GlobalRecources::players[myPlayerIndex]->inVladW = false;
	GlobalRecources::players[myPlayerIndex]->targetAble = true;

	Player* current = GlobalRecources::players[myPlayerIndex];
	GlobalRecources::pFinding->findPath(current->getCol(), current->getRow(), myPlayerIndex);
}