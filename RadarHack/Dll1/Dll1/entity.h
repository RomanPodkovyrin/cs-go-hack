//#include <Windows.h>
#include <cstdint>
// CS:GO Hack

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
	char pad_0000[96]; //0x0000
	int32_t id; //0x0060
	char pad_0064[48]; //0x0064
	float N0000085A; //0x0094
	float N00000959; //0x0098
	float N0000085B; //0x009C
	float N00000702; //0x00A0
	float N0000085D; //0x00A4
	float N0000095C; //0x00A8
	float N0000085E; //0x00AC
	float N0000052A; //0x00B0
	float N00000860; //0x00B4
	char pad_00B8[60]; //0x00B8
	int32_t team; //0x00F4
	char pad_00F8[8]; //0x00F8
	int32_t health; //0x0100
	char pad_0104[16]; //0x0104
	float xAcceleration; //0x0114
	float yAcceleration; //0x0118
	float zAcceleration; //0x011C
	char pad_0120[12]; //0x0120
	float cameraV; //0x012C
	float cameraH; //0x0130
	char pad_0134[4]; //0x0134
	float xAxis; //0x0138
	float yAxis; //0x013C
	float zAxis; //0x0140
	char pad_0144[616]; //0x0144
	float N0000018E; //0x03AC
	float N0000018F; //0x03B0
	float N00000190; //0x03B4
	char pad_03B8[5924]; //0x03B8
}; //Size: 0x1ADC
