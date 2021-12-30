#include "NetworkCommunication.hpp"
#include <string>
#include "iostream" 
using namespace std;
#include "PortableClient.hpp"
#include "PortableServer.hpp"

vector<int> NetworkCommunication::tokenCount;
vector<string> NetworkCommunication::rawData;
vector<vector<int>> NetworkCommunication::parseToIntsData;
vector<int> NetworkCommunication::tokenIndex;

void NetworkCommunication::addTokenToAll(char* token) {
	for (int i = 0; i < rawData.size(); i++) {
		if (token == nullptr) {
			cout << "networkCommunication received nullptr as outgoing token";
			exit(0);
		}
		if (rawData[i].size() > 0) {//dont start with a comma
			rawData[i].push_back(',');
		}
		rawData[i].append(token);
		tokenCount[i]++;
	}
}

void NetworkCommunication::addTokenToAll(int token) {
	for (int i = 0; i < rawData.size(); i++) {
		if (rawData[i].size() > 0) {//dont start  or end with a comma
			rawData[i].push_back(',');
		}
		rawData[i].append(to_string(token).c_str());
		tokenCount[i]++;
	}
}

void NetworkCommunication::addTokenToAllExceptClient(char* token, int playerIndex) {
	if (playerIndex == 0) {//host can be ignored
		addTokenToAll(token);
	}
	for (int i = 0; i < rawData.size(); i++) {
		//playerIndex is always clientIndex + 1 because host is also a player, dont think too much about how i could fix this and accept the -1.
		if (i != playerIndex - 1) {
			if (token == nullptr) {
				cout << "networkCommunication received nullptr as outgoing token";
				exit(0);
			}
			if (rawData[i].size() > 0) {//dont start with a comma
				rawData[i].push_back(',');
			}
			rawData[i].append(token);
			tokenCount[i]++;
		}
	}
}

void NetworkCommunication::addTokenToAllExceptClient(int token, int playerIndex) {
	if (playerIndex == 0) {//host can be ignored
		addTokenToAll(token);
	}
	for (int i = 0; i < rawData.size(); i++) {
		//playerIndex is always clientIndex + 1 because host is also a player, dont think too much about how i could fix this and accept the -1.
		if (i != playerIndex - 1) {
			if (rawData[i].size() > 0) {//dont start  or end with a comma
				rawData[i].push_back(',');
			}
			rawData[i].append(to_string(token).c_str());
			tokenCount[i]++;
		}
	}
}

void NetworkCommunication::addTokenToClient(char* token, int playerIndex) {
	if (token == nullptr) {
		cout << "networkCommunication received nullptr as outgoing token";
		exit(0);
	}
	//playerIndex is always clientIndex + 1 because host is also a player, dont think too much about how i could fix this and accept the -1.
	if (rawData[playerIndex - 1].size() > 0) {//dont start with a comma
		rawData[playerIndex - 1].push_back(',');
	}
	rawData[playerIndex - 1].append(token);
	tokenCount[playerIndex - 1]++;
}

void NetworkCommunication::addTokenToClient(int token, int playerIndex) {
	//playerIndex is always clientIndex + 1 because host is also a player, dont think too much about how i could fix this and accept the -1.
	if (rawData[playerIndex - 1].size() > 0) {//dont start  or end with a comma
		rawData[playerIndex - 1].push_back(',');
	}
	rawData[playerIndex - 1].append(to_string(token).c_str());
	tokenCount[playerIndex - 1]++;
	
}


void NetworkCommunication::sendTokensToServer(int index, shared_ptr<PortableServer> server) {
	for (int i = 0; i < server->getClientCount(); i++) {
		server->sendToClient(i, string(rawData[i]));
		rawData[i].clear();
	}
}

void NetworkCommunication::sendTokensToClient(shared_ptr<PortableClient> client) {
	if(client->isConnected() == true) {
		client->sendToServer(string(rawData[0]));
		rawData[0].clear();
	}
}

int NetworkCommunication::receiveNextToken(int index) {
	int out = parseToIntsData[index].at(tokenIndex[index]);
	tokenIndex[index]++;
	return out;
}

void NetworkCommunication::receiveTonkensFromServer(int index, shared_ptr<PortableServer> server) {
	for (int i = 0; i < server->getClientCount(); i++) {
	
		bool copyAndParse = false;
		string data;
		server->getMutex()->lock();//gets locked before writing message
		data = server->getLastMessages()[i];
		server->getMutex()->unlock();
		if (data.size() > 0) {
			copyAndParse = true;
		}

		if (parseToIntsData.size() == 0) {
			parseToIntsData.push_back(vector<int>());
		}

		if (copyAndParse == true) {
			parseToIntsData[index] = extractInts(data);
		}
		tokenIndex[index] = 0;
	}
}

void NetworkCommunication::receiveTonkensFromClient(shared_ptr<PortableClient> client) {
	if(client->isConnected() == true) {
		string data;
		bool copyAndParse = false;
		client->getMutex()->lock();//gets locked before writing message
		auto msg = client->getLastMessage().get();
		if (msg != nullptr && msg->size() > 0) {
			copyAndParse = true;
		}

		if (copyAndParse == true) {
			string data = *msg;//copy so network can overwrite
			client->getMutex()->unlock();
			parseToIntsData[0] = extractInts(std::move(data));
		}
		else {
			client->getMutex()->unlock();
		}
		tokenIndex[0] = 0;
	}
}

void NetworkCommunication::initNewCommunication(int index) {
	if(index == rawData.size()) {
		rawData.push_back(string());
		tokenIndex.push_back(0);
		tokenCount.push_back(0);
		parseToIntsData.push_back(vector<int>());
		return;
	}
	else {
		tokenIndex[index] = 0;
		tokenCount[index] = 0;
		parseToIntsData[index].clear();
		return;
	}
	std::cout << "Bad index in initNewCommunication (NetworkCommunication): " << index;
	std::exit(0);
}

int NetworkCommunication::getTokenCount(int index) {
	return tokenCount[index];
}

vector<int> NetworkCommunication::extractInts(string str) {
	vector<int> out = vector<int>();
	int lastSplit = 0;
	for (int i = 0; i < str.length(); i++) {
		if (str.at(i) == ',') {
			int temp = stoi(str.substr(lastSplit, i));
			out.push_back(temp);
			lastSplit = i + 1;
		}
		if (i == str.length() - 1) {
			int temp = stoi(str.substr(lastSplit, i));
			out.push_back(temp);
		}
	}
	return out;
}