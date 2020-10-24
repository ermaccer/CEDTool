#pragma once


struct ced_header {
	int fileType;
	int  unk[2];
	int  platform;
	char date[16];
	int  mainType;
	int  sections;
	char pad[24];
};

struct ced_section {
	int  identifier;
	int  size;
};


/*
SECTIONS:
ITAM - Material Info
_TAM - Material Data
IOBJ - Object Info
_OBJ - Object Data
__PB - Binary 3D Data

*/


struct section_info {
	char identifier[8];
	char pad[4];
	int  sectionSize;
};

#pragma pack(push, 1)
struct material_info {
	char _pad[4];
	int  materials;
	int  stringSize;
	char __pad[20];
	//char stringdata[stringSize];
};
#pragma (pop)

struct object_info{
	char mainName[64];
//	char data[unk - 64];
};
#pragma pack(push, 1)
struct binary_data {
	char groupName[208];
	char _pad[12];
	int  indices;
	char __pad[16];
	int  tris;
	char ___pad[92];
};
#pragma (pop)
const char* GetSectionType(int type);
