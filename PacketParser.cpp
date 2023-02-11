#include "PacketParser.h"
#include <iostream>
#include <algorithm>

using namespace std;

const string INPUT_FILE_NAME = "OUCHLMM2.incoming.packets";

inline void printPacket(vector<char> buf) {
    for (auto it = buf.begin(); it != buf.end(); it++) {
        cout << "it: " << *it << " buf: " << buf[*it] << ", ";
    }
    cout << endl;
}

inline uint32_t swap_endian(vector<char> buffer) {
    return (buffer[0] & 0xff) << 24 | (buffer[1] & 0xff) << 16 |
        (buffer[2] & 0xff) << 8 | (buffer[3] & 0xff);
}

inline uint32_t number_char_vector(const vector<char>& vec) {
    uint32_t result = 0;
    for (int i = 0; i < 4; i++) {
        result |= static_cast<uint32_t>(vec[i]) << (8 * i);
    }
    return result;
}

inline void printStream(const PacketStream& st) {
    cout << "\t Accepted: " << st.accepted << " messages\n";
    cout << "\t Replaced: " << st.replaced << " messages\n";
    cout << "\t System Event: " << st.systemEvent << " messages\n";
    cout << "\t Cancelled: " << st.cancelled << " messages\n";
    cout << "\t Executed: " << st.executed << " messages: " << st.executedShares << " executed shares\n";
}

bool PacketParser::readInput(string filename) {
    if (filename.empty()) {
        filename = INPUT_FILE_NAME;
    }
    inputFile = make_unique<ifstream>(filename, ios::binary);
    if (!inputFile->is_open()) {
        cout << "Failed to open input file\n";
        return false;
    }
    return true;
}

bool PacketParser::getPacketHeader(PacketHeader& header)
{
    vector<char>buffer(PACKET_HEADER_SIZE);

    inputFile->read(&buffer[0], PACKET_HEADER_SIZE);
    if (buffer.empty() || inputFile->fail()) {
        return false;
    }
    header.first = (buffer[0] & 0xff) << 8 | (buffer[1] & 0xff);
    header.second = swap_endian(vector<char>(buffer.begin() + 2, buffer.end()));
    return true;
}

 void PacketParser::parsePacket(const PacketHeader& header)
 {
     if (partialPackets.find(header.first) != partialPackets.end()) {
         //there are partial packets in this stream
         handlePartialPacket(header);
         return;
     }
     if (header.second < DESIRED_MIN_PACKET_SIZE) {
         //packet size is not even enough to know the message type.
         //add it to partial packet for this stream
         partialPackets[header.first] = {header.second,MessageType::Unknown};
         return;
     }

     std::streampos curPos = packetPos;
     //get the message type
     char val;
     curPos += MESSAGE_TYPE_OFFSET;
     inputFile->seekg(curPos);
     inputFile->read(&val, 1);
     MessageType msgType = static_cast<MessageType>(val);
     streams[header.first].updateStream(msgType);
     // if it is executed message, update executed shares too
     if (msgType == MessageType::Executed &&
         (header.second > (EXECUTED_SHARES_OFFSET + EXECUTED_SHARES_LENGTH)))
     {
        curPos += 22;
        getExecutedShares(header.first, header.second, curPos);
     }
     packetPos += header.second;
     inputFile->seekg(packetPos);
     if (isPartialPacket(msgType, header.second)) {
         partialPackets[header.first] = {header.second, msgType};
     }
 }

 void PacketParser::handlePartialPacket(const PacketHeader& header)
 {
     PartialPacket pPacket = partialPackets[header.first];

     if (pPacket.receivedSize >= DESIRED_MIN_PACKET_SIZE) {
         //we would have updated streams with this message already
         //lets check if it is Executed message and packet size is greater than executed shares
         if ((pPacket.msgType == MessageType::Executed) && 
             (pPacket.receivedSize < EXECUTED_SHARES_OFFSET) && 
             (pPacket.receivedSize + header.second > EXECUTED_SHARES_LENGTH + EXECUTED_SHARES_OFFSET)) {
             streampos curPos = packetPos;
             curPos += EXECUTED_SHARES_OFFSET - partialPackets[header.first].receivedSize;
             getExecutedShares(header.first, header.second, curPos);
         }
     }
     else if ((partialPackets[header.first].receivedSize + header.second) > DESIRED_MIN_PACKET_SIZE) {
         //this would be the first time we're knowing the message type of this packet.
         streampos curPos = packetPos;
         curPos += DESIRED_MIN_PACKET_SIZE - partialPackets[header.first].receivedSize - 1;
         char val;
         inputFile->seekg(curPos);
         inputFile->read(&val, sizeof(char));
         MessageType msgType = static_cast<MessageType>(val);
         streams[header.first].updateStream(msgType);
         partialPackets[header.first].msgType = msgType;
         if (msgType == MessageType::Executed &&
             (pPacket.receivedSize + header.second > EXECUTED_SHARES_LENGTH + EXECUTED_SHARES_OFFSET)) {
             curPos += 22;
             getExecutedShares(header.first, header.second, curPos);
         }
     }
     // if we reached complete packet size, erase it.
     if ((pPacket.receivedSize + header.second) ==
         CompletePacketSize.at(pPacket.msgType)) {
         partialPackets.erase(header.first);
     }
     else {
         partialPackets[header.first].receivedSize += header.second;
     }
     packetPos += header.second;
     inputFile->seekg(packetPos);
 }

 void PacketParser::getExecutedShares(unsigned short streamID, uint32_t packetSize, streampos curPos)
 {
    inputFile->seekg(curPos);
    vector<char> buffer(EXECUTED_SHARES_LENGTH);
    inputFile->read(&buffer[0], EXECUTED_SHARES_LENGTH);
    streams[streamID].executedShares += number_char_vector(buffer);
 }

 void PacketParser::startParser() {
    //get start position
    while (!inputFile->eof()) {
        pair<unsigned short, uint32_t> header;
        if(!getPacketHeader(header)) break;
        packetPos = inputFile->tellg();
        parsePacket(header);
    }
}

 void PacketParser::printStreams()
 {
     if (streams.empty()) return;
     PacketStream totalStream;
     for (const auto& st : streams) {
         cout << "Stream " << st.first << "\n";
         totalStream.accepted += st.second.accepted;
         totalStream.replaced += st.second.replaced;
         totalStream.systemEvent += st.second.systemEvent;
         totalStream.cancelled += st.second.cancelled;
         totalStream.executed += st.second.executed;
         totalStream.executedShares += st.second.executedShares;
         printStream(st.second);
         cout << "...\n";
     }
     cout << "Totals:\n";
     printStream(totalStream);
 }








