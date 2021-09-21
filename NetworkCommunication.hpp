#pragma once
#include <vector>
#include <string>
#include "iostream" 
using namespace std;
class PortableServer;
class PortableClient;
class NetworkCommunication {
public:
	static void initNewCommunication();

	static void addToken(char* token);
	static void addToken(int token);

	static void sendTokensToServer(shared_ptr<PortableServer> server);
	static void sendTokensToClient(shared_ptr<PortableClient> client);

	static int receiveNextToken();
	static void receiveTonkensFromServer(shared_ptr<const PortableServer> server);
	static void receiveTonkensFromClient(shared_ptr<const PortableClient> client);

	static int getTokenCount();

private:
	static int tokenCount;
	static string rawData;
	static vector<int> parseToIntsData;
	static int tokenIndex;

	static vector<int> extractInts(string str);
};