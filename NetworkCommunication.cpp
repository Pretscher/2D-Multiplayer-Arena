#include "NetworkCommunication.hpp"

#include "iostream" 
using namespace std;
#include "PortableClient.hpp"
#include "PortableServer.hpp"

vector<int> NetworkCommunication::tokenCount;
vector<string> NetworkCommunication::rawData;
vector<int> NetworkCommunication::parseToIntsData;
vector<int> NetworkCommunication::tokenIndex;

void NetworkCommunication::addToken(char* token) {
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

void NetworkCommunication::addToken(int token) {
	for (int i = 0; i < rawData.size(); i++) {
		if (rawData[i].size() > 0) {//dont start  or end with a comma
			rawData[i].push_back(',');
		}
		rawData[i].append(to_string(token).c_str());
		tokenCount[i]++;
	}
}

void NetworkCommunication::sendTokensToServer(shared_ptr<PortableServer> server) {
	for (int i = 0; i < server->getClientCount(); i++) {
		server->sendToClient(i, rawData[0].c_str());
		rawData[0].clear();
	}
}

void NetworkCommunication::sendTokensToClient(int index, shared_ptr<PortableClient> client) {
	if(client->isConnected() == true) {
		client->sendToServer(rawData[index].c_str());
		rawData[index].clear();
	}
}

int NetworkCommunication::receiveNextToken(int index) {
	int out = parseToIntsData.at(tokenIndex[index]);
	tokenIndex[index]++;
	return out;
}

void NetworkCommunication::receiveTonkensFromServer(int index, shared_ptr<PortableServer> server) {
	for (int i = 0; i < server->getClientCount(); i++) {
		server->sendToClient(i, rawData[index].c_str());
		rawData[index].clear();
	
		bool copyAndParse = false;
		string data;
		server->getMutex()->lock();//gets locked before writing message
		data = server->getLastMessages()[i];
		server->getMutex()->unlock();
		if (data.size() > 0) {
			copyAndParse = true;
		}

		if (copyAndParse == true) {
			parseToIntsData = extractInts(data);
		}
		else {
			server->getMutex()->unlock();
		}
		tokenIndex[0] = 0;
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
			parseToIntsData = extractInts(std::move(data));
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
		return;
	}
	else {
		rawData[index].clear();
		tokenIndex[index] = 0;
		tokenCount[index] = 0;
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