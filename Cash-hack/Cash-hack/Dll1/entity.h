//#include <Windows.h>
#include <cstdint>
// CS:GO Hack

// Created with ReClass.NET 1.2 by KN4CK3R

class ClientInfo
{
public:
	class Entity* EntityPointer; //0x0000
	int32_t isThisID; //0x0004
	class ClientInfo* previousNode; //0x0008
	class ClientInfo* nextNode; //0x000C
}; //Size: 0x0010

class ListOfEntities
{
public:
	char pad_0000[16]; //0x0000
	class ClientInfo listOfEntities[64]; //0x0010
}; //Size: 0x0410

class Entity
{
public:
	char pad_0000[256]; //0x0000
	int32_t Health; //0x0100
	char pad_0104[16]; //0x0104
	float x_acceleration; //0x0114
	float y_acceleration; //0x0118
	float z_acceleration; //0x011C
	char pad_0120[24]; //0x0120
	float x_axis; //0x0138
	float y_axis; //0x013C
	float z_axis; //0x0140
	char pad_0144[12]; //0x0144
}; //Size: 0x0150
