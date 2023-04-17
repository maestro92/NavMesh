#pragma once
#include "../PlatformShared/platform_shared.h"
#include "memory.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

/*
handmade hero style asset system

we have 4 master arrays
these master arrays contains all the data.



	AssetFamily assetFamilies[NUM_ASSET_FAMILY];

											asset *Assets;                          asset_slot *Bitmaps;
				 ___________
				| Asset_    |               backdrop0                               bitmap0
				|  Backdrop |               backdrop1                               bitmap1
				|___________|               backdrop2                               bitmap2
				| Asset_    |               backdrop3                               bitmap3
				|  Shadow   |               backdrop4                               bitmap4
				|___________|               shadow0         Bitmaps[slotID]         bitmap5
				| Asset_    |  --------->   shadow1         --------->              bitmap6
				|  Tree     |               shadow2                                 ...
				|___________|               shadow3                                 ...
				|           |               tree0
				|  ...      |               tree1
				|___________|               tree2
				|           |               tree3
				|  ...      |               tree4
				|___________|               tree5
				|           |               ...
				|           |               ...
				|___________|


*/

namespace AssetDataFormatType
{
	enum Enum
	{
		Bitmap,
		Sound,
		Font,
		FontGlyph
	};
}

namespace AssetFamilyType
{
	enum Enum
	{
		None,
		Default,
		Font,
		FontGlyph,
		Ground,
		Chessboard,
		Wall,
		NUM_ASSET_FAMILY,
	};
}
extern PlatformAPI platformAPI;

struct BitmapId
{
	uint32 value;
};

struct SoundId
{
	uint32 value;
};

struct FontId
{
	uint32 value;
};

struct GlyphId
{
	uint32 value;
};


enum AssetState
{
	Unloaded,
	Queued,
	Loaded,
	Locked
};

struct AssetBitmapInfo
{
	char* filename;
};

struct AssetFontInfo
{
	char* filename;
};

struct AssetFamily
{
	uint32 startAssetIndex;
	uint32 onePastLastAssetIndex;
};

struct AssetTag
{
	uint32 tagProperty;
	float value;
};


struct LoadedFont
{
	char* filename;
	int maxGlyphs;
	int numGlyphs;
	GlyphId* glyphs;

	stbtt_fontinfo fontInfo;

	// this is not really needed since we are just gonna do 2^16, but for consistency, we do those. 
	int unicodeMapSize;
	// unicode to glyphIndex
	uint16* unicodeMap;

	// maybe have to put bitmapIds here, if we want to compute position of text without having to load Each glyphs
	// currently it is in LoadedGlyph
};

struct LoadedGlyph
{
	char c;
	glm::ivec2 bitmapXYOffsets;		// will I have situations where I calculate the offset without wanting to load the data?
									// if so, then i'll put it in LoadedFont.
	LoadedBitmap bitmap;
};

// contains meta data about the asset 
struct AssetHandle
{
	// this looks into our master tag Table
	uint32 startTagIndex;
	uint32 onePastLastTagIndex;


	union 
	{
		AssetBitmapInfo bitmapInfo;
		AssetFontInfo fontInfo;
	};

	// or int AssetIndex?
};



// This is the handmade hero assetAlot class
struct Asset
{
	AssetDataFormatType::Enum type;
	AssetState state;

	union
	{
		LoadedBitmap loadedBitmap;
		LoadedFont loadedFont;
		LoadedGlyph loadedGlyph;
	};
};


struct GameAssets
{
	AssetFamily masterAssetFamilyTable[AssetFamilyType::NUM_ASSET_FAMILY];

	uint32 maxAssetHandles;
	uint32 numAssetHandles;
	AssetHandle* masterAssetHandleTable;

	// Asset handle doesnt have a 
	uint32 maxAssets;
	uint32 numAssets;
	Asset* assets;

	uint32 numMasterTags;
	AssetTag* masterTagTable;

	// These are mostly used when writing in assets
	AssetFamily* currentEditedAssetFamily = nullptr;
//	uint32 currentNumAssetHandle;		do you need this guy

	// used for all filenames and stuff
	char** stringHandles;
	int numStrings;
	int maxStrings;
};


void BeginAssetFamily(GameAssets* ga, AssetFamilyType::Enum type)
{
	assert(!ga->currentEditedAssetFamily);
	ga->currentEditedAssetFamily = &ga->masterAssetFamilyTable[(int)type];

	AssetFamily* family = ga->currentEditedAssetFamily;
	family->startAssetIndex = ga->numAssetHandles;   // does this work?
	family->onePastLastAssetIndex = family->startAssetIndex;
}

void EndAssetFamily(GameAssets* ga)
{
//	std::cout << "onePastLastAssetIndex " << ga->currentAssetFamilyType->onePastLastAssetIndex << std::endl;

	assert(ga->currentEditedAssetFamily);
	ga->currentEditedAssetFamily = nullptr;

	
}

uint32 GetFirstAssetIdFrom(GameAssets* Assets, AssetFamilyType::Enum familyType)
{
	uint32 Result = 0;

	AssetFamily* family = &Assets->masterAssetFamilyTable[familyType];
	if (family->startAssetIndex != family->onePastLastAssetIndex)
	{
		Result = family->startAssetIndex;
	}

	return(Result);
}

BitmapId GetFirstBitmapIdFrom(GameAssets* ga, AssetFamilyType::Enum familyType)
{
	BitmapId result = { GetFirstAssetIdFrom(ga, familyType) };
	return(result);
}

// code point means ascci code number
GlyphId GetGlyph(GameAssets* ga, LoadedFont *font, uint32 desiredCodePoint)
{
	int glyphIndex = font->unicodeMap[desiredCodePoint];

	GlyphId result = font->glyphs[glyphIndex];
	return result;
}


LoadedBitmap* GetBitmap(GameAssets* ga, BitmapId id)
{
	LoadedBitmap* result = &ga->assets[id.value].loadedBitmap;
	return result;
}

LoadedFont* GetFont(GameAssets* ga, FontId id)
{
	LoadedFont* result = &ga->assets[id.value].loadedFont;
	return result;
}

LoadedGlyph* GetGlyph(GameAssets* ga, GlyphId id)
{
	LoadedGlyph* result = &ga->assets[id.value].loadedGlyph;
	return result;
}

LoadedBitmap CreateEmptyBitmap(MemoryArena* memoryArena, uint32 width, uint32 height, bool clearToZero)
{
	LoadedBitmap result = {};

	//	Result.AlignPercentage = V2(0.5f, 0.5f);
	//	Result.WidthOverHeight = SafeRatio1((r32)Width, (r32)Height);

	result.width = width;
	result.height = height;
	result.pitch = result.width * 4;
	int32 totalBitmapSize = result.pitch * height;
	result.memory = PushSize(memoryArena, totalBitmapSize);

	return result;
}

// 
LoadedFont CreateEmptyLoadedFont(MemoryArena* memoryArena, char* filename)
{
	LoadedFont loadedFont = {};
	loadedFont.maxGlyphs = 256;
	loadedFont.numGlyphs = 0;
	loadedFont.glyphs = PushArray(memoryArena, loadedFont.maxGlyphs, GlyphId);
	loadedFont.filename = filename;

	// assuming we are supporting 65536
	loadedFont.unicodeMapSize = 65536;
	loadedFont.unicodeMap = PushArray(memoryArena, loadedFont.unicodeMapSize, uint16);
	// loadedFont.horizontalAdvance = 10;


	unsigned char* fileContent = 0;
	int fileSize = 0;

	FILE *ptr;

	//	ptr = fopen("c:/Windows/Fonts/arial.ttf", "rb");  // r for read, b for binary
	ptr = fopen(filename, "rb");
	fseek(ptr, 0L, SEEK_END);
	fileSize = ftell(ptr);
	fseek(ptr, 0L, SEEK_SET);

	fileContent = (unsigned char*)VirtualAlloc(0, (size_t)fileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	fread(fileContent, sizeof(uint8), fileSize, ptr); // read 10 bytes to our buffer

	stbtt_InitFont(&loadedFont.fontInfo, fileContent, stbtt_GetFontOffsetForIndex(fileContent, 0));

	return loadedFont;
}

void LoadBitmapToMemory(GameAssets* ga, BitmapId id)
{
	AssetHandle* handle = &ga->masterAssetHandleTable[id.value];

	AssetBitmapInfo* info = &handle->bitmapInfo;
	BitmapInfo bitmap = platformAPI.readImageFile(info->filename);

	Asset* asset = &ga->assets[id.value];
	asset->state = AssetState::Unloaded;

	LoadedBitmap result = {};
	result.width = bitmap.width;
	result.height = bitmap.height;
	result.memory = bitmap.memory;

	void* textureHandle = platformAPI.allocateTexture(result.width, result.height, result.memory);
	result.textureHandle = POINTER_TO_UINT32(textureHandle);
	std::cout << "texture handle " << result.textureHandle << std::endl;
	asset->loadedBitmap = result;
}

int FONT_SCALE = 20;

void LoadGlyphBitmapToMemory(MemoryArena* memoryArena, GameAssets* ga, LoadedFont* loadedFont, BitmapId id)
{
	AssetHandle* handle = &ga->masterAssetHandleTable[id.value];

	Asset* asset = &ga->assets[id.value];
	asset->state = AssetState::Unloaded;

	char c = asset->loadedGlyph.c;

	float scale = stbtt_ScaleForPixelHeight(&loadedFont->fontInfo, FONT_SCALE);

	int width, height, xOffset, yOffset;
	unsigned char*  monoBitmap = stbtt_GetCodepointBitmap(&loadedFont->fontInfo, scale,
															scale, c,
															&width, &height, &xOffset, &yOffset);

	int c_x1, c_y1, c_x2, c_y2;
	stbtt_GetCodepointBitmapBox(&loadedFont->fontInfo, c, scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);

	
	uint8* src = monoBitmap;

	if (c == 'l' || c == 'm' || c == 'n')
	{
		std::cout << "char is " << c << ", " << "c_1 (" << c_x1 << ", " << c_y1 << ")" << "		(" << c_x2 << ", " << c_y2 << ")" << std::endl;
	}
	// 4 bytes per pixel
	LoadedBitmap result = CreateEmptyBitmap(memoryArena, width, height, false);
	result.width = width;
	result.height = height;
	result.memory = VirtualAlloc(0, (size_t)width * height * 4, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);


	// STB bitmaps are upside down
	uint32* dst = (uint32*)result.memory;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			uint8 alpha = *src++;

			int row = height - y - 1;
			dst[row * width + x] = (alpha << 24) | (alpha << 16) | (alpha << 8) | (alpha << 0);
		}
	}

	/*
	for (int j = 0; j < height; ++j) 
	{
		for (int i = 0; i < width; ++i)
		{
			putchar(" .:ioVM@"[monoBitmap[j*width + i] >> 5]);
		}
		putchar('\n');
	}

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			int row = height - y - 1;
			if (dst[row * width + x] )
			{
				std::cout << '@';
			}
			else
			{
				std::cout << ' ';
			}
		}
		std::cout << '\n';
	}
	*/

	stbtt_FreeBitmap(monoBitmap, 0);


	void* textureHandle = platformAPI.allocateTexture(result.width, result.height, result.memory);
	result.textureHandle = POINTER_TO_UINT32(textureHandle);

//	result.textureHandle = platformAPI.allocateTexture2(result.width, result.height, result.memory);

	asset->loadedGlyph.bitmap = result;
	asset->loadedGlyph.bitmapXYOffsets = glm::ivec2(c_x1, c_y1);
	
	//	std::cout << "id " << id.value << ", texture handle " << result.textureHandle << ", sset->loadedGlyph.bitmap  " << asset->loadedGlyph.bitmap.textureHandle << std::endl;

}





// This is just used so that I can return two things
struct AddedAsset
{
	uint32 id;  // index of the handle in the master handle array 
				// also the index of data inside the master data array
	AssetHandle* handle;
	Asset* data;
};

AddedAsset AddBaseAsset(GameAssets* ga)
{
	assert(ga->currentEditedAssetFamily);
	int index = ga->currentEditedAssetFamily->onePastLastAssetIndex++;

	// add a new handle and a source
	AssetHandle* handle = &ga->masterAssetHandleTable[index];
	Asset* data = &ga->assets[index];

	ga->numAssetHandles++;
	ga->numAssets++;

	handle->startTagIndex = 0;
	handle->onePastLastTagIndex = 0;
	
	AddedAsset addedAsset = {};
	addedAsset.id = index;
	addedAsset.handle = handle;
	addedAsset.data = data;
	return addedAsset;
}

void AddBitmapAsset(GameAssets* ga, char* filename)
{
	assert(ga->currentEditedAssetFamily);
	AddedAsset newAssetInfo = AddBaseAsset(ga);

	newAssetInfo.handle->bitmapInfo.filename = filename;
	newAssetInfo.data->type = AssetDataFormatType::Bitmap;

}

void AddCharacterAsset(GameAssets* ga, LoadedFont* fontAssetInfo, char c)
{
	assert(ga->currentEditedAssetFamily);
	AddedAsset newAssetInfo = AddBaseAsset(ga);

	newAssetInfo.data->type = AssetDataFormatType::FontGlyph;
	int glyphIndex = fontAssetInfo->numGlyphs;
	fontAssetInfo->glyphs[glyphIndex] = { newAssetInfo.id };
	fontAssetInfo->numGlyphs++;

	fontAssetInfo->unicodeMap[c] = glyphIndex;
	newAssetInfo.data->loadedGlyph.c = c;
	
}

void AddFontAsset(GameAssets* ga, LoadedFont* fontAssetInfo)
{
	assert(ga->currentEditedAssetFamily);
	AddedAsset newAssetInfo = AddBaseAsset(ga);

	newAssetInfo.data->type = AssetDataFormatType::Font;


	/*
	LoadedFont font = {};
	font.numGlyphs = fontAssetInfo->numGlyphs;
	font.maxGlyphs = fontAssetInfo
	font.horizontalAdvance = fontAssetInfo->horizontalAdvance;

	font.unicodeMapSize = fontAssetInfo->unicodeMapSize;


//	int size = font.numGlyphs * sizeof(BitmapId);
	//	font.glyphs = (BitmapId*)malloc(size);
//	memcpy((void*)font.glyphs, (void*)fontAssetInfo->glyphs, size);


//	font.unicodeMap = (uint16*)malloc(65536);
//	memcpy((void*)font.glyphs, (void*)fontAssetInfo->glyphs, size);
*/

	newAssetInfo.data->loadedFont = *fontAssetInfo;
}


// defined in the gamecode


int STBtest()
{
	
//	char buffer[10000];
	unsigned char screen[20][79];


	stbtt_fontinfo font;
	int i, j, ascent, baseline, ch = 0;
	float scale, xpos = 0; // leave a little padding in case the character extends left
//	char *text = "Heljo World!"; // intentionally misspelled to show 'lj' brokenness
	char text[4] = "lmn"; // intentionally misspelled to show 'lj' brokenness


	int fileSize = 0;
	FILE *ptr;
	ptr = fopen("c:/Windows/Fonts/arial.ttf", "rb");  // r for read, b for binary
	fseek(ptr, 0L, SEEK_END);
	fileSize = ftell(ptr);
	fseek(ptr, 0L, SEEK_SET);
	unsigned char* buffer = new unsigned char[fileSize];
	fread(buffer, sizeof(uint8), fileSize, ptr); // read 10 bytes to our buffer


//	fread(buffer, 1, 10000, fopen("c:/Windows/Fonts/arial.ttf", "rb"));
	stbtt_InitFont(&font, (const unsigned char*)buffer, 0);
	
	scale = stbtt_ScaleForPixelHeight(&font, 15);
	stbtt_GetFontVMetrics(&font, &ascent, 0, 0);
	baseline = (int)(ascent*scale);
	
	std::cout << "ascent " << ascent << std::endl;
	std::cout << "baseline " << baseline << std::endl;

	while (text[ch]) {
		unsigned char screen2[20][79];

		int advance, lsb, x0, y0, x1, y1;
		float x_shift = 0;// xpos - (float)floor(xpos);
		stbtt_GetCodepointHMetrics(&font, text[ch], &advance, &lsb);
//		stbtt_GetCodepointBitmapBoxSubpixel(&font, text[ch], scale, scale, x_shift, 0, &x0, &y0, &x1, &y1);
	
		stbtt_GetCodepointBitmapBox(&font, text[ch], scale, scale, &x0, &y0, &x1, &y1);

		std::cout << "text[ch] " << text[ch] << ", x0 y0 (" << x0 << ", " << y0 << ")   " << "x1 y1 (" << x1 << ", " << y1 << ")" << std::endl;

		std::cout << "			pos " << ((int)xpos + x0) << "," << (baseline + y0) << std::endl;
		
		stbtt_MakeCodepointBitmapSubpixel(&font, &screen[baseline + y0][(int)xpos + x0], x1 - x0, y1 - y0, 79, scale, scale, x_shift, 0, text[ch]);
		// note that this stomps the old data, so where character boxes overlap (e.g. 'lj') it's wrong
		// because this API is really for baking character bitmaps into textures. if you want to render
		// a sequence of characters, you really need to render each bitmap to a temp buffer, then
		// "alpha blend" that into the working buffer
	
	//	stbtt_MakeCodepointBitmapSubpixel(&font, &screen2[baseline + y0][x0], x1 - x0, y1 - y0, 79, scale, scale, x_shift, 0, text[ch]);

		
		xpos += (advance * scale);
		if (text[ch + 1])
			xpos += scale * stbtt_GetCodepointKernAdvance(&font, text[ch], text[ch + 1]);
		++ch;

		/*
		for (j = 0; j < 20; ++j) {
			for (i = 0; i < 78; ++i)
				putchar(" .:ioVM@"[screen2[j][i] >> 5]);
			putchar('\n');
		}


		for (j = 0; j < 20; ++j) {
			for (i = 0; i < 78; ++i)
				screen2[j][i] = 204;
		}

		std::cout << "\n" << std::endl;
		*/
	}

	for (j = 0; j < 20; ++j) {
		for (i = 0; i < 78; ++i)
			putchar(" .:ioVM@"[screen[j][i] >> 5]);
		putchar('\n');
	}
		
	return 0;
}

char* AllocateAndGetCharArray(MemoryArena* memoryArena, GameAssets* ga, char* temp, int numBytes)
{
	char* charPtr = ga->stringHandles[ga->numStrings++];
	charPtr = PushArray(memoryArena, numBytes, char);
	memcpyDiy(temp, charPtr, numBytes);
	return charPtr;
}

void AllocateGameAssets(MemoryArena* memoryArena, GameAssets* ga)
{

	// replace this with pushing it to the memory arena.
	ga->maxAssets = 256;
	ga->numAssets = 0;
	ga->assets = PushArray(memoryArena, ga->maxAssets, Asset);

	// the 0th handle is just the null asset.
	ga->maxAssetHandles = ga->maxAssets;
	ga->numAssetHandles = 0;
	ga->masterAssetHandleTable = PushArray(memoryArena, ga->maxAssetHandles, AssetHandle);

	ga->numMasterTags = 0;

	ga->maxStrings = 50;
	ga->numStrings = 0;
	ga->stringHandles = PushArray(memoryArena, ga->maxStrings, char*);

	// simulating this process of loading pack file off disk 
	BeginAssetFamily(ga, AssetFamilyType::None);
	char temp[] = "./Assets/error.bmp";
	AddBitmapAsset(ga, AllocateAndGetCharArray(memoryArena, ga, temp, ArrayCount(temp)));
	EndAssetFamily(ga);

	BeginAssetFamily(ga, AssetFamilyType::Default);
	char temp1[] = "./Assets/white.bmp";
	AddBitmapAsset(ga, AllocateAndGetCharArray(memoryArena, ga, temp1, ArrayCount(temp1)));
	EndAssetFamily(ga);

	BeginAssetFamily(ga, AssetFamilyType::Wall);
	char temp2[] = "./Assets/wall2.bmp";
	AddBitmapAsset(ga, AllocateAndGetCharArray(memoryArena, ga, temp2, ArrayCount(temp2)));
	char temp3[] = "./Assets/wall1.bmp";
	AddBitmapAsset(ga, AllocateAndGetCharArray(memoryArena, ga, temp3, ArrayCount(temp3)));
	EndAssetFamily(ga);

	BeginAssetFamily(ga, AssetFamilyType::Chessboard);
	char temp4[] = "./Assets/chess.bmp";
	AddBitmapAsset(ga, AllocateAndGetCharArray(memoryArena, ga, temp4, ArrayCount(temp4)));
	EndAssetFamily(ga);

	char temp5[] = "./Assets/arial.ttf";
	LoadedFont loadedFont = CreateEmptyLoadedFont(memoryArena, AllocateAndGetCharArray(memoryArena, ga, temp5, ArrayCount(temp5)));

	BeginAssetFamily(ga, AssetFamilyType::FontGlyph);
	for (int i = 'a'; i <= 'z'; i++)
	{
		AddCharacterAsset(ga, &loadedFont, i);
	}
	
	for (int i = 'A'; i <= 'Z'; i++)
	{
		AddCharacterAsset(ga, &loadedFont, i);
	}
	
	for (int i = '0'; i <= '9'; i++)
	{
		AddCharacterAsset(ga, &loadedFont, i);
	}
	AddCharacterAsset(ga, &loadedFont, '.');
	AddCharacterAsset(ga, &loadedFont, ':');
	AddCharacterAsset(ga, &loadedFont, ' ');
	AddCharacterAsset(ga, &loadedFont, '/');
	AddCharacterAsset(ga, &loadedFont, '\\');
	AddCharacterAsset(ga, &loadedFont, '|');
	AddCharacterAsset(ga, &loadedFont, '-');
	AddCharacterAsset(ga, &loadedFont, '+');
	AddCharacterAsset(ga, &loadedFont, '!');

	EndAssetFamily(ga);

	BeginAssetFamily(ga, AssetFamilyType::Font);
	AddFontAsset(ga, &loadedFont);
	EndAssetFamily(ga);


	// Load all assets into memory

	// texture 0
	for (int i = 0; i < ga->numAssets; i++)
	{
		Asset* asset = &ga->assets[i];

		if (asset->type == AssetDataFormatType::Bitmap)
		{
			BitmapId bitmapId = { (uint32)i };
			// std::cout << bitmapId.value << std::endl;
			LoadBitmapToMemory(ga, bitmapId);
		}
		else if (asset->type == AssetDataFormatType::Font)
		{
			// we already have one
			//	LoadFontToMemory(ga, bitmapId);
		}
		else if (asset->type == AssetDataFormatType::FontGlyph)
		{
			BitmapId bitmapId = { (uint32)i };
			// std::cout << "i " << i << ", bitmapId.value " << bitmapId.value << std::endl;
			LoadGlyphBitmapToMemory(memoryArena, ga, &loadedFont, bitmapId);
		
			/*
			LoadedBitmap* bitmap = GetBitmap(ga, bitmapId);
			std::cout << "i " << i << ", bitmap value " << bitmapId.value << " " << bitmap->textureHandle << std::endl;
			int a = 1;
			*/
		}
	}

	/*
	
	for (int i = 0; i < ga->numAssets; i++)
	{
		Asset* asset = &ga->assets[i];

		if (asset->type == AssetDataFormatType::Bitmap || asset->type == AssetDataFormatType::FontGlyph)
		{
			BitmapId bitmapId = { i };

	
			LoadedBitmap* bitmap = GetBitmap(ga, bitmapId);
			std::cout << "i " << i << ", bitmap value " << bitmapId.value << " " << bitmap->textureHandle << std::endl;

			//		LoadBitmapToMemory(ga, bitmapId);
		}
	}
	*/
	// STBtest();
}

