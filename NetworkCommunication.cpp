#include "NetworkCommunication.hpp"
#include <string>
#include "iostream" 
using namespace std;
#include "PortableClient.hpp"
#include "PortableServer.hpp"

vector<int> NetworkCommunication::tokenCount;
vector<string> NetworkCommunication::rawSendData;
vector<vector<int>> NetworkCommunication::parseToIntsData;
vector<int> NetworkCommunication::tokenIndex;
vector<shared_ptr<string>> NetworkCommunication::rawReceivedData;

void NetworkCommunication::addTokenToAll(char* token) {
	for (int i = 0; i < rawSendData.size(); i++) {
		if (token == nullptr) {
			cout << "networkCommunication received nullptr as outgoing token";
			exit(0);
		}
		if (rawSendData[i].size() > 0) {//dont start with a comma
			rawSendData[i].push_back(',');
		}
		rawSendData[i].append(token);
		tokenCount[i]++;
	}
}

void NetworkCommunication::addTokenToAll(int token) {
	for (int i = 0; i < rawSendData.size(); i++) {
		if (rawSendData[i].size() > 0) {//dont start  or end with a comma
			rawSendData[i].push_back(',');
		}
		rawSendData[i].append(to_string(token).c_str());
		tokenCount[i]++;
	}
}

void NetworkCommunication::addTokenToAllExceptClient(char* token, int playerIndex) {
	if (playerIndex == 0) {//host can be ignored
		addTokenToAll(token);
		return;
	}
	for (int i = 0; i < rawSendData.size(); i++) {
		//playerIndex is always clientIndex + 1 because host is also a player, dont think too much about how i could fix this and accept the -1.
		if (i != playerIndex - 1) {
			if (token == nullptr) {
				cout << "networkCommunication received nullptr as outgoing token";
				exit(0);
			}
			if (rawSendData[i].size() > 0) {//dont start with a comma
				rawSendData[i].push_back(',');
			}
			rawSendData[i].append(token);
			tokenCount[i]++;
		}
	}
}

void NetworkCommunication::addTokenToAllExceptClient(int token, int playerIndex) {
	if (playerIndex == 0) {//host can be ignored
		addTokenToAll(token);
		return;
	}
	for (int i = 0; i < rawSendData.size(); i++) {
		//playerIndex is always clientIndex + 1 because host is also a player, dont think too much about how i could fix this and accept the -1.
		if (i != playerIndex - 1) {
			if (rawSendData[i].size() > 0) {//dont start  or end with a comma
				rawSendData[i].push_back(',');
			}
			rawSendData[i].append(to_string(token).c_str());
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
	if (rawSendData[playerIndex - 1].size() > 0) {//dont start with a comma
		rawSendData[playerIndex - 1].push_back(',');
	}
	rawSendData[playerIndex - 1].append(token);
	tokenCount[playerIndex - 1]++;
}

void NetworkCommunication::addTokenToClient(int token, int playerIndex) {
	//playerIndex is always clientIndex + 1 because host is also a player, dont think too much about how i could fix this and accept the -1.
	if (rawSendData[playerIndex - 1].size() > 0) {//dont start  or end with a comma
		rawSendData[playerIndex - 1].push_back(',');
	}
	rawSendData[playerIndex - 1].append(to_string(token).c_str());
	tokenCount[playerIndex - 1]++;
	
}


void NetworkCommunication::sendTokensToServer(int index, shared_ptr<PortableServer> server) {
	for (int i = 0; i < server->getClientCount(); i++) {
		server->sendToClient(i, string(rawSendData[i]));
		rawSendData[i].clear();
	}
}

void NetworkCommunication::sendTokensToClient(shared_ptr<PortableClient> client) {
	if(client->isConnected() == true) {
		client->sendToServer(string(rawSendData[0]));
		rawSendData[0].clear();
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
		bool copyAndParse = false;
		client->getMutex()->lock();//gets locked before writing message
		rawReceivedData[0] = client->getLastMessage();
		if (rawReceivedData[0] != nullptr && rawReceivedData[0]->size() > 0) {
			copyAndParse = true;
		}
		if (copyAndParse == true) {
			rawReceivedData[0] = shared_ptr<string>(new string(*rawReceivedData[0]));//copy so received datais saved and network can overwrite string
			string data = *rawReceivedData[0];//copy again to move and manipulate this string while keeping the data
			client->getMutex()->unlock();
			parseToIntsData[0] = extractInts(std::move(data));
		}
		else {
			client->getMutex()->unlock();
		}
		tokenIndex[0] = 0;
	}
}

void NetworkCommunication::initNewCommunication(int index, shared_ptr<vector<bool>> io_clientInits) {
	if(index == rawSendData.size()) {
		rawSendData.push_back(string());
		rawReceivedData.push_back(shared_ptr<string>(new string()));
		tokenIndex.push_back(0);
		tokenCount.push_back(0);
		parseToIntsData.push_back(vector<int>());
		io_clientInits->push_back(false);
		return;
	}
	else {
		tokenIndex[index] = 0;
		tokenCount[index] = 0;
		parseToIntsData[index].clear();
		io_clientInits->at(index) = false;
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