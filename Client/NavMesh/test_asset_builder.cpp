#include "test_asset_builder.h"












void LoadFontTest()
{
	/*
	unsigned char* fileContent = 0;
	int fileSize = 0;

	FILE *ptr;

	ptr = fopen("c:/Windows/Fonts/arial.ttf", "rb");  // r for read, b for binary
	fseek(ptr, 0L, SEEK_END);
	fileSize = ftell(ptr);
	fseek(ptr, 0L, SEEK_SET);

	fileContent = (unsigned char*)VirtualAlloc(0, (size_t)fileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	fread(fileContent, sizeof(uint8), fileSize, ptr); // read 10 bytes to our buffer

	int width, height, xOffset, yOffset;

	stbtt_fontinfo font;
	stbtt_InitFont(&font, fileContent, stbtt_GetFontOffsetForIndex(fileContent, 0));
	unsigned char* monoBitmap = stbtt_GetCodepointBitmap(&font, 0, stbtt_ScaleForPixelHeight(&font, 20), 'N', &width, &height, &xOffset, &yOffset);

	for (int j = 0; j < height; ++j) {
		for (int i = 0; i < width; ++i)
			putchar(" .:ioVM@"[monoBitmap[j*width + i] >> 5]);
		putchar('\n');
	}

	uint8* src = monoBitmap;

	// 4 bytes per pixel
	LoadedBitmap bitmap;//  = MakeEmtpyBitmap(width, height, false);
	bitmap.width = width;
	bitmap.height = height;
	bitmap.memory = VirtualAlloc(0, (size_t)width * height * 4, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	uint32* dst = (uint32*)bitmap.memory;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			uint8 alpha = *src++;
			*dst++ = (alpha << 24) | (alpha << 16) | (alpha << 8) | (alpha << 0);
		}
	}

	cout << endl;

	unsigned int* memoryPtr = (unsigned int*)bitmap.memory;
	for (int j = 0; j < height; ++j)
	{
		for (int i = 0; i < width; ++i)
		{
			if (memoryPtr[j * width + i] != 0)
			{
				putchar('A');
			}
			else
			{
				putchar(' ');
			}
		}
		putchar('\n');
	}


	stbtt_FreeBitmap(monoBitmap, 0);
	*/
}

