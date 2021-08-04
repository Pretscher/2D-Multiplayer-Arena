#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "GameClient.hpp"
#include "GameServer.hpp"

class NetworkCommunication {
public:
	static void init() {
		rawData = new std::string();
	}

	static void addToken(char* token) {
		if (token == nullptr) {
			std::cout << "networkCommunication received nullptr as outgoing token";
			std::exit(0);
		}
		if (rawData->size() > 0) {//dont start with a comma
			rawData->push_back(',');
		}
		rawData->append(token);
		tokenCount++;
	}

	static void addToken(int token) {
		if (rawData->size() > 0) {//dont start with a comma
			rawData->push_back(',');
		}
		rawData->append(std::to_string(token).c_str());
		tokenCount++;
	}

	static void sendTokensToServer(GameServer* server) {
		server->sendToClient(rawData->c_str());
		delete rawData;
		rawData = new std::string();
		tokenCount = 0;
	}

	static void sendTokensToClient(GameClient* client) {
		client->sendToServer(rawData->c_str());
		delete rawData;
		rawData = new std::string();
		tokenCount = 0;
	}

	static int receiveNextToken() {
		int out = parseToIntsData->at(tokenIndex);
		tokenIndex++;
		return out;
	}

	static void receiveTonkensFromServer(GameServer* server) {
		std::string* data;
		int otherPlayer;
		bool copyAndParse = false;
		server->getMutex()->lock();//gets locked before writing message
		data = server->getLastMessage();
		if (data != nullptr && data->size() > 0) {
			copyAndParse = true;
		}
		
		if (copyAndParse == true) {
			data = new std::string(data->c_str());//copy so network can overwrite
			server->getMutex()->unlock();
			parseToIntsData = extractInts(data);
		}
		else {
			server->getMutex()->unlock();
		}
		tokenIndex = 0;
	}

	static void receiveTonkensFromClient(GameClient* client) {
		std::string* data;
		int otherPlayer;
		bool copyAndParse = false;
		client->getMutex()->lock();//gets locked before writing message
		data = client->getLastMessage();
		if (data != nullptr && data->size() > 0) {
			copyAndParse = true;
		}

		if (copyAndParse == true) {
			data = new std::string(data->c_str());//copy so network can overwrite
			client->getMutex()->unlock();
			parseToIntsData = extractInts(data);
		}
		else {
			client->getMutex()->unlock();
		}
		tokenIndex = 0;
	}

	static int getTokenCount() {
		return tokenCount;
	}

private:
	static int tokenCount;
	static std::string* rawData;
	static std::vector<int>* parseToIntsData;
	static int tokenIndex;

	static std::vector<int>* extractInts(std::string* str) {
		std::vector<int>* out = new std::vector<int>();
		int lastSplit = 0;
		for (int i = 0; i < str->length(); i++) {
			if (str->at(i) == ',') {
				int temp = std::stoi(str->substr(lastSplit, i));
				out->push_back(temp);
				lastSplit = i + 1;
			}
			if (i == str->length() - 1) {
				int temp = std::stoi(str->substr(lastSplit, i));
				out->push_back(temp);
			}
		}
		return out;
	}
};