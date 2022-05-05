#pragma once
#include "Renderer.hpp"
#include "iostream" 
#include <memory>

using namespace std;
class Menu {
public:
	unique_ptr<Button> host;
	unique_ptr<Button> connect;
	shared_ptr<string> selectedHost = nullptr;
	vector<string> avHosts;
	vector<Button> avHostButtons;
	bool selectHost = false;
	Menu() {
		host = unique_ptr<Button>(new Button(850, 200, 300, 150, sf::Color(200, 200, 200, 255), "Host", 80, sf::Color(255, 0, 0, 255)));
		connect = unique_ptr<Button>(new Button(850, 1450, 300, 150, sf::Color(200, 200, 200, 255), "Connect", 80, sf::Color(255, 0, 0, 255)));
		bHostServer = false;
		bConnectAsClient = false;
	}

	void selectHostMenu() {
		selectHost = true;
	}

	shared_ptr<string> getSelectedHost() {
		return selectedHost;
	}

	void giveAvailableHosts(vector<string> i_avHosts) {
		avHosts = i_avHosts;
	}

	void update() {
		if (selectHost == false) {
			if (host->isPressed()) {
				bHostServer = true;
			}
			if (connect->isPressed()) {
				bConnectAsClient = true;
			}
		}
		else {
			avHostButtons.clear();//has to be cleared eitherway if connection has been lost and availablehosts is nullptr again
			for (int i = 0; i < avHosts.size(); i++) {
				avHostButtons.push_back(Button(200 + (i * 150), 310, 1310, 100, sf::Color(50, 50, 50, 100), avHosts.at(i), 60, sf::Color(255, 255, 255, 255)));
				if (avHostButtons[i].isPressed() == true) {
					selectedHost = shared_ptr<string>(new string(avHosts[i]));
				}
			}
		}
	}

	void drawMenu() {
		if (selectHost == false) {
			host->draw();
			connect->draw();
		}
		else {
			for (int i = 0; i < avHostButtons.size(); i++) {
				avHostButtons[i].draw();
			}
		}

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