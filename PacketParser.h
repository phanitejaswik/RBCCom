#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <unordered_map>
#include "OUCHProtocol.h"

typedef std::pair<unsigned short, uint32_t> PacketHeader; //first contains stream ID and second has packet length
class PacketParser
{
private:
	std::unordered_map<int, PacketStream> streams;
	std::unordered_map<int, PartialPacket> partialPackets;
	std::unique_ptr<std::ifstream> inputFile;
	std::streampos packetPos;
	bool getPacketHeader(PacketHeader& header);
	void parsePacket(const PacketHeader& header);
	void handlePartialPacket(const PacketHeader& header);
	void getExecutedShares(unsigned short streamID, uint32_t size, std::streampos curpos);

public:
	PacketParser() {
	};
	~PacketParser() {};
	bool readInput(std::string fileName = "");
	void startParser();
	void printStreams();
};

