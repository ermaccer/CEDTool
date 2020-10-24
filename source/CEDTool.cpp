// CEDTool.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>
#include "eObj.h"
#include "ced.h"
#include <memory>
#include <type_traits>
#include "filef.h"
enum eModes {
	MODE_EXPORT = 1,
	MODE_IMPORT

};

int main(int argc, char* argv[])
{
	if (argc == 1) {
		std::cout << "CEDTool - work with CED model format by ermaccer\n"
			<< "Usage: cedtool <params> <file>\n"
			<< "    -e              Exports .ced to .obj\n"
			<< "    -o              Specifies a folder for extraction\n";
		return 1;
	}

	int mode = 0;
	std::string o_param;
	// params
	for (int i = 1; i < argc - 1; i++)
	{
		if (argv[i][0] != '-' || strlen(argv[i]) != 2) {
			return 1;
		}
		switch (argv[i][1])
		{
		case 'e': mode = MODE_EXPORT;
			break;
		case 'o':
			i++;
			o_param = argv[i];
			break;
		default:
			std::cout << "ERROR: Param does not exist: " << argv[i] << std::endl;
			break;
		}
	}


	if (mode == MODE_EXPORT)
	{
		std::ifstream pFile(argv[argc - 1], std::ifstream::binary);

		if (!pFile)
		{
			std::cout << "ERROR: Could not open: " << argv[argc - 1] << "!" << std::endl;
			return 1;
		}

		ced_header header;

		pFile.read((char*)&header, sizeof(ced_header));

		if (!(header.fileType == 'CED_'))
		{
			std::cout << "ERROR: " << argv[argc - 1] << " is not a valid CED file!" << std::endl;
			return 1;
		}

		std::vector<obj_v> vVerts;
		std::vector<obj_uv> vMaps;
		std::vector<obj_vn> vNorm;
		std::vector<obj_face> vFaces;
		std::vector<std::string> vTextures;
		std::vector<short> vTempFaces;
		int nModel = 0;
		std::string folderName;
		bool skinChecked = false;
		

		if (!o_param.empty())
		{
			if (!std::filesystem::exists(o_param))
				std::filesystem::create_directory(o_param);
			std::filesystem::current_path(o_param);
		}


		std::cout << "Sections:\n";
		for (int i = 0; i < header.sections; i++)
		{
			ced_section section;
			pFile.read((char*)&section, sizeof(ced_section));

			std::cout << "[" << i << "]\t" << GetSectionType(section.identifier) << " Size: " << section.size << std::endl;
		}

		for (int i = 0; i < header.sections; i++)
		{
			//std::cout << "at " << pFile.tellg() << std::endl;
			section_info section;
			pFile.read((char*)&section, sizeof(section_info));


			if (strcmp(section.identifier, "!KLBITAM") == 0)
			{
				std::cout << "Processing Material Info\n";


				material_info mat;

				pFile.read((char*)&mat, sizeof(material_info));

				if (!(mat.stringSize == 0))
				{
					std::unique_ptr<char[]> stringBuff = std::make_unique<char[]>(mat.stringSize);
					pFile.read(stringBuff.get(), mat.stringSize);

					//std::cout << "Textures: " << mat.materials << std::endl;
					vTextures.clear();
					// print textures
					int baseOffset = 0;
					for (int a = 0; a < mat.materials + 1; a++)
					{

						char* textName = (char*)((int)stringBuff.get() + baseOffset);
						//std::cout << textName << std::endl;
						baseOffset += strlen(textName) + 1;
						std::string str(textName - 1, strlen(textName) + 1);

						std::string textureName = splitString(str, true);
						vTextures.push_back(textureName);
					}
				}
				else {
					vTextures.clear();
					for (int a = 0; a < 20; a++)
					vTextures.push_back("default");
				}

			}


			// check body modifier
			if (!skinChecked)
			{
				char data[8];
				int  skipVal = 0;
				pFile.read((char*)&data, sizeof(data));
				if (strcmp(data, "BODY") == 0 && !(data[6] == 'S'))
				{
					skinChecked = true;
					pFile.seekg(0, pFile.cur);
				}
					
				else
					pFile.seekg(-8, pFile.cur);

			}

			if (skinChecked)
			{
				std::cout << "Mesh is a skin" << std::endl;
				std::cout << "Skinned meshes are not supported yet!" << std::endl;
				return 1;
			}


			if (strcmp(section.identifier, "!KLB_TAM") == 0)
			{
				std::cout << "Processing Material Data\n";
			//	std::cout << "at " << pFile.tellg() << std::endl;
				pFile.seekg(section.sectionSize, pFile.cur);
				// don't need this for obj
			}

			if (strcmp(section.identifier, "!KLBIJBO") == 0)
			{
				std::cout << "Processing Object Info\n";
				//std::cout << "at " << pFile.tellg() << std::endl;
				pFile.seekg(section.sectionSize, pFile.cur);
				// don't need this for obj
			}

			if (strcmp(section.identifier, "!KLBINOB") == 0)
			{
				std::cout << "Processing Bone Info\n";
				//std::cout << "at " << pFile.tellg() << std::endl;
				pFile.seekg(section.sectionSize, pFile.cur);
				// don't need this for obj
			}


			if (strcmp(section.identifier, "!KLB_JBO") == 0)
			{
				std::cout << "Processing Object Data\n";
				//std::cout << "at " << pFile.tellg() << std::endl;
				std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(section.sectionSize);
				pFile.read(dataBuff.get(), section.sectionSize);
				std::cout << "Group Name: " << (char*)dataBuff.get() << std::endl;

			}

			if (strcmp(section.identifier, "!KLB__BP") == 0)
			{
				int readBytes = 0;

				std::cout << "Processing 3D Data\n";
				//std::cout << "at " << pFile.tellg() << std::endl;
				binary_data obj;
				pFile.read((char*)&obj, sizeof(binary_data));
				readBytes += pFile.gcount();



				std::cout << "Mesh Name: " << obj.groupName << std::endl;
				//std::cout << "Tris: " << obj.tris << std::endl;
				//std::cout << "Indices: " << obj.indices << std::endl;

				eObj model;
				vVerts.clear();
				vMaps.clear();
				vNorm.clear();
				vFaces.clear();

				// begin reading;

				for (int a = 0; a < obj.tris; a++)
				{
					// x, y, z
					obj_v v;
					pFile.read((char*)&v, sizeof(v));
					readBytes += pFile.gcount();

					if (*(char*)&v.x == 0x00)
						v.x = 0.0f;

					vVerts.push_back(v);
					
					float f_unk[4];
					// unk1, unk2, unk3, unk4
					pFile.read((char*)&f_unk, sizeof(f_unk));
					readBytes += pFile.gcount();

					// u, v

					obj_uv uv;
					pFile.read((char*)&uv, sizeof(obj_uv));
					readBytes += pFile.gcount();


					vMaps.push_back(uv);

				}

				

				for (int a = 0; a < obj.indices; a++)
				{
					short face;
					pFile.read((char*)&face, sizeof(face));
					readBytes += pFile.gcount();
					vTempFaces.push_back(face);

				}

				if (!(readBytes == section.sectionSize))
				{
					do {
						short face;
						pFile.read((char*)&face, sizeof(face));
						readBytes += pFile.gcount();
						vTempFaces.push_back(face);
					} while (!(readBytes == section.sectionSize));
				}


				for (int z = 0; z < vTempFaces.size() - 2; z++)
				{
					short a, b, c;
					if (z & 1)
					{
						a = vTempFaces[z];
						b = vTempFaces[z + 2];
						c = vTempFaces[z + 1];
					}
					else
					{
						a = vTempFaces[z];
						b = vTempFaces[z + 1];
						c = vTempFaces[z + 2];
					}
					if (a == b || b == c || a == c)
						continue;


					obj_face face = { a,b,c };
					vFaces.push_back(face);
				}

				model.SetModelData(vVerts, vMaps, vNorm, vFaces);
				std::string output = obj.groupName;
				output += "obj";
				output.insert(0, std::to_string(nModel));
				char tmp[256] = {};
				sprintf(tmp, "%s", vTextures[nModel].c_str());

				model.SaveFile(output.c_str(), obj.groupName, tmp);
				nModel++;

				//std::cout << "Group Name: " << (char*)dataBuff.get() << std::endl;

			}

		}









	}
	if (mode == MODE_IMPORT)
	{
		
	}

	return 0;
}
