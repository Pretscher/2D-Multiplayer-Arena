#pragma once
#include <memory>
#include <vector>
#include <string>
#include "iostream" 
using namespace std;
class PortableServer;
class PortableClient;
class NetworkCommunication {
public:
	static void initNewCommunication(int index);

	static void addTokenToAll(int token);
	static void addTokenToAll(char* token);

	static void addTokenToAllExceptClient(int token, int clientIndex);
	static void addTokenToAllExceptClient(char* token, int clientIndex);

	static void addTokenToClient(int token, int clientIndex);
	static void addTokenToClient(char* token, int clientIndex);

	static void sendTokensToServer(int index, shared_ptr<PortableServer> server);
	static void sendTokensToClient(shared_ptr<PortableClient> client);

	static int receiveNextToken(int index);
	static void receiveTonkensFromServer(int index, shared_ptr<PortableServer> server);
	static void receiveTonkensFromClient(shared_ptr<PortableClient> client);

	static int getTokenCount(int index);
private:
	static vector<int> tokenCount;
	static vector<string> rawData;
	static vector<vector<int>> parseToIntsData;
	static vector<int> tokenIndex;

	static vector<int> extractInts(string str);
};