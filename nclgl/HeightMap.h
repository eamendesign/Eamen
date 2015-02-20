#pragma once

#include "../nclgl/Mesh.h"
#include <fstream>

using namespace std;

#define RAW_WIDTH 257
#define RAW_HEIGHT 257

#define HEIGHTMAP_X 16.0f
#define HEIGHTMAP_Z 16.0f
#define HEIGHTMAP_Y 1.25f
#define HEIGHTMAP_TEX_X 1.0f / 16.0f
#define HEIGHTMAP_TEX_Z 1.0f / 16.0f

class HeightMap : public Mesh {
public:
	HeightMap(string name) {
		ifstream file(name.c_str(), ios::binary);
		if (!file) {
			std::cout << "Could not find " << name << std::endl;
			return;
		}

		numVertices = RAW_WIDTH * RAW_HEIGHT;
		numIndices = (RAW_WIDTH - 1) * (RAW_HEIGHT - 1) * 6;
		vertices = new Vector3[numVertices];
		textureCoords = new Vector2[numVertices];
		indices = new GLuint[numIndices];

		unsigned char* data = new unsigned char[numVertices];
		file.read((char*)data, numVertices * sizeof(unsigned char));
		file.close();
		float highval = 0.0f;
		for (int x = 0; x < RAW_WIDTH; ++x) {
			for (int z = 0; z < RAW_HEIGHT; ++z) {
				int offset = (x * RAW_WIDTH) + z;

				vertices[offset] = Vector3(x * HEIGHTMAP_X, data[offset] * HEIGHTMAP_Y, z * HEIGHTMAP_Z);

				textureCoords[offset] = Vector2(x * HEIGHTMAP_TEX_X, z * HEIGHTMAP_TEX_Z);
			}
		}
		delete data;

		numIndices = 0;

		for (int x = 0; x < RAW_WIDTH - 1; ++x) {
			for (int z = 0; z < RAW_HEIGHT - 1; ++z) {
				int a = (x       * (RAW_WIDTH)) + z;
				int b = ((x + 1) * (RAW_WIDTH)) + z;
				int c = ((x + 1) * (RAW_WIDTH)) + (z + 1);
				int d = (x 		 * (RAW_WIDTH)) + (z + 1);

				indices[numIndices++] = c;
				indices[numIndices++] = b;
				indices[numIndices++] = a;

				indices[numIndices++] = a;
				indices[numIndices++] = d;
				indices[numIndices++] = c;
			}
		}
		GenerateNormals();
		GenerateTangents();
		BufferData();
	}

	Vector3* GetVertices() {
		return vertices;
	}
	Vector3* GetNormals() {
		return normals;
	}

	~HeightMap(void){};
};