#pragma once
#include <vector>
#include <string>
#include <iostream>
class PortableServer;
class PortableClient;
class NetworkCommunication {
public:
	static void initNewCommunication();

	static void addToken(char* token);
	static void addToken(int token);

	static void sendTokensToServer(std::unique_ptr<PortableServer> server);
	static void sendTokensToClient(std::unique_ptr<PortableClient> client);

	static int receiveNextToken();
	static void receiveTonkensFromServer(std::unique_ptr<PortableServer> server);
	static void receiveTonkensFromClient(std::unique_ptr<PortableClient> client);

	static int getTokenCount();

private:
	static int tokenCount;
	static std::string rawData;
	static std::vector<int> parseToIntsData;
	static int tokenIndex;

	static std::vector<int> extractInts(std::string str);
};