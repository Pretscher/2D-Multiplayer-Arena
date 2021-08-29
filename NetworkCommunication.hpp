#pragma once
#include <vector>
#include <string>

class PortableServer;
class PortableClient;
class NetworkCommunication {
public:
	static void init();
	static void initNewCommunication();

	static void addToken(char* token);
	static void addToken(int token);

	static void sendTokensToServer(PortableServer* server);
	static void sendTokensToClient(PortableClient* client);

	static int receiveNextToken();
	static void receiveTonkensFromServer(PortableServer* server);
	static void receiveTonkensFromClient(PortableClient* client);

	static int getTokenCount();

private:
	static int tokenCount;
	static std::string* rawData;
	static std::vector<int>* parseToIntsData;
	static int tokenIndex;

	static std::vector<int>* extractInts(std::string* str);
};