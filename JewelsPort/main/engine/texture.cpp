#include "engine/engine.h"
//#include "engine/color.h"

#include <iostream>
#include <unordered_map>

#include <memory/alloc.h>

#include <util/log.h>

extern VGAExtended *vga;

#define PCX_XRES			0x8
#define PCX_BITPLANES		0x41
#define PCX_BYTES_PER_ROW	0x42
#define PCX_IMAGE_DATA		0x80
#define PCX_COLOR_DATA		-0x300

#define PCX_RLE_BITS		(0x80 | 0x40)

class TextureObject
{
public:
	TextureObject() {}
	TextureObject(const char *path)
	{
		uint16_t i, value, color;
		FILE *pcxFile = fopen(path, "rb");
		if (!pcxFile) return;

		fseek(pcxFile, PCX_XRES, SEEK_SET);

		fread(&xres, sizeof(uint16_t), 1, pcxFile);
		fread(&yres, sizeof(uint16_t), 1, pcxFile);
		xres++; yres++;

		fseek(pcxFile, PCX_BITPLANES, SEEK_SET);
		uint8_t bitplanes = fgetc(pcxFile);
		if (bitplanes != 1) goto fail;

		imageData = (unsigned char**)heap_caps_malloc(sizeof(unsigned char*) * yres, MALLOC_CAP_PREFERRED);
		if (!imageData) goto failAlloc;

		for (i = 0; i < yres; i++)
		{
			imageData[i] = (unsigned char*)heap_caps_malloc(xres, MALLOC_CAP_PREFERRED);
		}

		fseek(pcxFile, PCX_IMAGE_DATA, SEEK_SET);

		for(i = 0; i < xres*yres;)
		{
			value = fgetc(pcxFile);
			if (value & PCX_RLE_BITS)
			{
				value &= ~PCX_RLE_BITS;

				color = fgetc(pcxFile);
				for (uint16_t j = i; j < i+value; j++)
				{
					imageData[j/xres][j%xres] = color;
				}
				i += value;
			}
			else
			{
				imageData[i/xres][i%xres] = value;
				i++;
			}
		}

failAlloc:
fail:
		fclose(pcxFile);
	}

	void drawTexture(int x, int y, int w, int h, int sx, int sy, int scaleFactor)
	{
		vga->drawTextureIndexed(imageData, x, y, w, h, sx, sy, scaleFactor);
	}

	~TextureObject()
	{
		for(int i = 0; i < yres; i++) heap_caps_free(imageData[i]);
		heap_caps_free(imageData);
	}

private:
	uint16_t xres, yres;
	unsigned char **imageData = NULL;
};

static std::unordered_map<std::string, TextureObject*> textures;

void createTexture(std::string path, std::string name)
{
	if (textures.find(name) != textures.end()) return;

	textures[name] = new TextureObject(path.c_str());
}

void deleteTexture(std::string name)
{
	if (textures.find(name) == textures.end()) return;

	delete(textures[name]);
	textures.erase(name);
}

void drawTexture(std::string name, int x, int y, int w, int h, int sx, int sy, int scaleFactor)
{
	if (textures.find(name) == textures.end()) return;

	textures[name]->drawTexture(x, y, w, h, sx, sy, scaleFactor);
}