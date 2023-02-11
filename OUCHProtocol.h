#pragma once
#include <unordered_map>

enum MessageType {
	SystemEvent = 'S',
	Accepted = 'A',
	Replaced = 'U',
	Executed = 'E',
	Cancelled = 'C',
	Unknown = 'X'
};

// we don't need all the packet parsing for this project
const int EXECUTED_SHARES_OFFSET = 26;
const int EXECUTED_SHARES_LENGTH = 4;
const int PACKET_HEADER_SIZE = 6;
const int DESIRED_MIN_PACKET_SIZE = 4;
const int MESSAGE_TYPE_OFFSET = 3;


struct PacketStream {
	int accepted = 0;
	int systemEvent = 0;
	int replaced = 0;
	int executed = 0;
	int cancelled = 0;
	int executedShares = 0;

	void updateStream(MessageType msg) {
		switch (msg) {
		case Accepted:
			accepted++;
			break;
		case SystemEvent: 
			systemEvent++;
			break;
		case Replaced: 
			replaced++;
			break;
		case Executed: 
			executed++;
			break;
		case Cancelled:
			cancelled++;
			break;
		default:
			break;
		}
	}
};

struct PartialPacket {
	uint32_t receivedSize;
	MessageType msgType;
};

const std::unordered_map<MessageType, int> CompletePacketSize {
	{SystemEvent, 13}, {Accepted, 68}, {Replaced, 82}, {Executed, 43}, {Cancelled, 31}
};

inline bool isPartialPacket(MessageType msgType, int packetSize) {
	return CompletePacketSize.at(msgType) > packetSize;
}
