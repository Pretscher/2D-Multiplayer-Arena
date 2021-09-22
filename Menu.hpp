#pragma once
#include "Renderer.hpp"
#include "iostream" 
#include <memory>

using namespace std;
class Menu {
public:
	unique_ptr<Button> host;
	unique_ptr<Button> connect;
	Menu() {
		host = unique_ptr<Button>(new Button(850, 200, 300, 150, sf::Color(200, 200, 200, 255), "Host", sf::Color(255, 0, 0, 255)));
		connect = unique_ptr<Button>(new Button(850, 1450, 300, 150, sf::Color(200, 200, 200, 255), "Connect", sf::Color(255, 0, 0, 255)));
		bHostServer = false;
		bConnectAsClient = false;
	}

	void update() {
		if (host->isPressed()) {
			bHostServer = true;
		}
		if (connect->isPressed()) {
			bConnectAsClient = true;
		}
	}

	void drawMenu() {
		host->draw();
		connect->draw();

	}

	bool hostServer() {
		return bHostServer;
	}

	bool connectAsClient() {
		return bConnectAsClient;
	}
private:
	bool bHostServer;
	bool bConnectAsClient;
};