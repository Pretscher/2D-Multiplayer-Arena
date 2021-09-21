#include "NetworkCommunication.hpp"

#include "iostream" 
using namespace std;
#include "PortableClient.hpp"
#include "PortableServer.hpp"

int NetworkCommunication::tokenCount;
string NetworkCommunication::rawData;
vector<int> NetworkCommunication::parseToIntsData;
int NetworkCommunication::tokenIndex;

void NetworkCommunication::addToken(char* token) {
	if (token == nullptr) {
		cout << "networkCommunication received nullptr as outgoing token";
		exit(0);
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
	rawData.append(to_string(token).c_str());
	tokenCount++;
}

void NetworkCommunication::sendTokensToServer(shared_ptr<PortableServer> server) {
	if(server->isConnected() == true) {
		server->sendToClient(rawData.c_str());
		rawData.clear();
	}
}

void NetworkCommunication::sendTokensToClient(shared_ptr<PortableClient> client) {
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

void NetworkCommunication::receiveTonkensFromServer(shared_ptr<const PortableServer> server) {
	if(server->isConnected() == true) {

		bool copyAndParse = false;
		const string* data;
		server->getMutex()->lock();//gets locked before writing message
		data = server->getLastMessage().get();
		if (data != nullptr && data->size() > 0) {
			copyAndParse = true;
		}

		if (copyAndParse == true) {
			data = new string(data->c_str());//copy so network can overwrite
			server->getMutex()->unlock();
			parseToIntsData = extractInts(*data);
		}
		else {
			server->getMutex()->unlock();
		}
		tokenIndex = 0;
	}
}

void NetworkCommunication::receiveTonkensFromClient(shared_ptr<const PortableClient> client) {
	if(client->isConnected() == true) {
		const string* data;
		bool copyAndParse = false;
		client->getMutex()->lock();//gets locked before writing message
		data = client->getLastMessage().get();
		if (data != nullptr && data->size() > 0) {
			copyAndParse = true;
		}

		if (copyAndParse == true) {
			data = new string(data->c_str());//copy so network can overwrite
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