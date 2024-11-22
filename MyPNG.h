#pragma once
#include <fstream>
#include "reversebuf.h"
#include "pixel.h"

typedef unsigned char BYTE;



struct PNGINFOHEADER
{
	int width;
	int height;
	BYTE bitdepth;
	BYTE colorType;
	BYTE compersType;
	BYTE filteringType;
	BYTE interlace;
};


static const int fixedHuffman[4][4]{ // { leftbound,bits,kodsBegin,kodsEnd }
	{0,8,48,191},
	{144,9,400,511},
	{256,7,0,23},
	{280,8,192,199}
};

static const int HuffmanOffset[30][2]{ //{Bits , dist}
	{0,1},
	{0,2},
	{0,3},
	{0,4},
	{1,5},
	{1,7},
	{2,9},
	{2,13},
	{3,17},
	{3,25},
	{4,33},
	{4,49},
	{5,65},
	{5,97},
	{6,129},
	{6,193},
	{7,257},
	{7,385},
	{8,513},
	{8,769},
	{9,1025},
	{9,1537},
	{10,2049},
	{10,3073},
	{11,4097},
	{11,6145},
	{12,8193},
	{12,12289},
	{13,16385},
	{13,24577}
};

static const int HuffmanLen[29][2]{
	{0,3},
	{0,4},
	{0,5},
	{0,6},
	{0,7},
	{0,8},
	{0,9},
	{0,10},
	{1,11},
	{1,13},
	{1,15},
	{1,17},
	{2,19},
	{2,23},
	{2,27},
	{2,31},
	{3,35},
	{3,43},
	{3,51},
	{3,59},
	{4,67},
	{4,83},
	{4,99},
	{4,115},
	{5,131},
	{5,163},
	{5,195},
	{5,227},
	{0,258}
};

unsigned int CRC32_function(unsigned char* buf, unsigned long len);
unsigned int adler32(const unsigned char* buf, size_t buf_length);



class MyPNG {


public:
	MyPNG(const char* fileName);

	int getheight() { return header.height; }
	int getwidth() { return header.width; }
	RGB** getpixels() { return pixels.get(); }
private:
	PNGINFOHEADER header;
	Pixels pixels;

	void ReadChunks(std::ifstream&);
	void ReadIHDRChunk(std::ifstream&);
	void ReadIDATChunk(std::ifstream&, int);
	void ReadPLTEChunk(std::ifstream&, int);


	void ReadNonCopmressedBlocks(reversebuf&);
	void ReadHuffmanFixedCode(reversebuf&,int);
	void ReadHuffmanDynamicCode(reversebuf&, int);
};

void DecodsFromlengs(int*, int*, int);

void getBites(int , unsigned int&  , unsigned int& , int& , reversebuf& );

void readLenAndOffset(unsigned int , unsigned int& ,  int& , int& , int& , reversebuf& );

void reverseInt(int&);
void reverseUInt(unsigned int&);

int find(int*, int*, int,  unsigned int, unsigned int&, int&, reversebuf&);