// PacketStreamParser.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "PacketParser.h"

int main()
{
    std::cout << "Parsing packet streams .....\n";
    PacketParser packetParser;
    packetParser.readInput();
    packetParser.startParser();
    packetParser.printStreams();
    return 0;
}