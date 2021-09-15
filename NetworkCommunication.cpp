#include "NetworkCommunication.hpp"

#include <iostream>
#include "PortableClient.hpp"
#include "PortableServer.hpp"

int NetworkCommunication::tokenCount;
std::string NetworkCommunication::rawData;
std::vector<int> NetworkCommunication::parseToIntsData;
int NetworkCommunication::tokenIndex;

void NetworkCommunication::addToken(char* token) {
	if (token == nullptr) {
		std::cout << "networkCommunication received nullptr as outgoing token";
		std::exit(0);
	}
	if (rawData.size() > 0) {//dont start with a comma
		rawData.push_back(',');
	}
	rawData.append(token);
	tokenCount++;
}

void NetworkCommunication::addToken(int token) {
	if (rawData.size() > 0) {//dont start  or end with a comma
		rawData.push_back(',');
	}
	rawData.append(std::to_string(token).c_str());
	tokenCount++;
}

void NetworkCommunication::sendTokensToServer(std::unique_ptr<PortableServer> server) {
	if(server->isConnected() == true) {
		server->sendToClient(rawData.c_str());
		rawData.clear();
	}
}

void NetworkCommunication::sendTokensToClient(std::unique_ptr<PortableClient> client) {
	if(client->isConnected() == true) {
		client->sendToServer(rawData.c_str());
		rawData.clear();
	}
}

int NetworkCommunication::receiveNextToken() {
	int out = parseToIntsData.at(tokenIndex);
	tokenIndex++;
	return out;
}

void NetworkCommunication::receiveTonkensFromServer(std::unique_ptr<PortableServer> server) {
	if(server->isConnected() == true) {
		std::string* data;
		bool copyAndParse = false;
		server->getMutex()->lock();//gets locked before writing message
		data = server->getLastMessage();
		if (data != nullptr && data->size() > 0) {
			copyAndParse = true;
		}

		if (copyAndParse == true) {
			data = new std::string(data->c_str());//copy so network can overwrite
			server->getMutex()->unlock();
			parseToIntsData = extractInts(*data);
		}
		else {
			server->getMutex()->unlock();
		}
		tokenIndex = 0;
	}
}

void NetworkCommunication::receiveTonkensFromClient(std::unique_ptr<PortableClient> client) {
	if(client->isConnected() == true) {
		std::string* data;
		bool copyAndParse = false;
		client->getMutex()->lock();//gets locked before writing message
		data = client->getLastMessage();
		if (data != nullptr && data->size() > 0) {
			copyAndParse = true;
		}

		if (copyAndParse == true) {
			data = new std::string(data->c_str());//copy so network can overwrite
			client->getMutex()->unlock();
			parseToIntsData = extractInts(*data);
		}
		else {
			client->getMutex()->unlock();
		}
		tokenIndex = 0;
	}
}

void NetworkCommunication::initNewCommunication() {
	tokenCount = 0;
}

int NetworkCommunication::getTokenCount() {
	return tokenCount;
}

std::vector<int> NetworkCommunication::extractInts(std::string str) {
	std::vector<int> out = std::vector<int>();
	int lastSplit = 0;
	for (int i = 0; i < str.length(); i++) {
		if (str.at(i) == ',') {
			int temp = std::stoi(str.substr(lastSplit, i));
			out.push_back(temp);
			lastSplit = i + 1;
		}
		if (i == str.length() - 1) {
			int temp = std::stoi(str.substr(lastSplit, i));
			out.push_back(temp);
		}
	}
	return out;
}