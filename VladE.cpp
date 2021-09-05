#include "VladE.hpp"
#include <chrono>
#include "Renderer.hpp"
#include "AbilityRecources.hpp"
static int i_onCDPhase = 0;
static int i_addToNetworkPhase = 0;
static int i_abilityIndex = 2;

VladE::VladE(int i_myPlayerIndex) : Ability(i_myPlayerIndex, false, i_onCDPhase, i_addToNetworkPhase, i_abilityIndex) {
	endPhaseAfterMS(1500);
}
//constructor through networking
VladE::VladE(int i_myPlayerIndex, int i_currentCol, int i_phase, int i_timeInPhase) : Ability(i_myPlayerIndex, true, i_onCDPhase, i_addToNetworkPhase, i_abilityIndex) {
	endPhaseAfterMS(1500);
}

void VladE::execute0() {
	auto cTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	float diff = cTime - phaseStart [0];
	percentFinishedCharge = diff / phaseDuration [0];
}

void VladE::draw0() {
	int innerCircleRadius = range * abs(1.0f - percentFinishedCharge);
	Player* myPlayer = AbilityRecources::players [myPlayerIndex];

	int innerRow = myPlayer->getRow() + (myPlayer->getHeight() / 2) - innerCircleRadius;
	int innerCol = myPlayer->getCol() + (myPlayer->getWidth() / 2) - innerCircleRadius;
	Renderer::drawCircle(innerRow, innerCol, innerCircleRadius, sf::Color(150, 0, 0, 255), false, 5, false);
	
	int outerRow = myPlayer->getRow() + (myPlayer->getHeight() / 2) - range;
	int outerCol = myPlayer->getCol() + (myPlayer->getWidth() / 2) - range;
	Renderer::drawCircle(outerRow, outerCol, range, sf::Color(150, 0, 0, 255), false, 10, false);
}

void VladE::init1() {
	projectileCount = 16;
	projectiles = new Projectile* [projectileCount];
}

void VladE::execute1() {






	//end ability after all projectiles reached their destination
	bool someoneAlive = false;
	for (int i = 0; i < projectileCount; i++) {
		if (projectiles [i]->getDead() == false) {
			someoneAlive = true;
			break;
		}
	}
	if (someoneAlive == false) {
		finished = true;
	}
}

void VladE::draw1() {

}

void VladE::init2() {
	finished = true;
}

void VladE::execute2() {

}

void VladE::draw2() {

}