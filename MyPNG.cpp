#include "MyPNG.h"

void reverseInt(int& x) {
	union {
		int num;
		char c[4];
	}tmp;
	tmp.num = x;

	std::swap(tmp.c[0], tmp.c[3]);
	std::swap(tmp.c[1], tmp.c[2]);
	x = tmp.num;
}
void reverseUInt(unsigned int& x) {
	union {
		unsigned int num;
		char c[4];
	}tmp;
	tmp.num = x;

	tmp.c[0] = ReverseBytes((BYTE)tmp.c[0]);
	tmp.c[1] = ReverseBytes((BYTE)tmp.c[1]);
	tmp.c[2] = ReverseBytes((BYTE)tmp.c[2]);
	tmp.c[3] = ReverseBytes((BYTE)tmp.c[3]);
	std::swap(tmp.c[0], tmp.c[3]);
	std::swap(tmp.c[1], tmp.c[2]);
	x = tmp.num;
}

unsigned int CRC32_function(unsigned char* buf, unsigned long len)
{
	unsigned long crc_table[256];
	unsigned long crc;
	for (int i = 0; i < 256; i++)
	{
		crc = i;
		for (int j = 0; j < 8; j++)
			crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;
		crc_table[i] = crc;
	};
	crc = 0xFFFFFFFFUL;
	while (len--)
		crc = crc_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);
	return crc ^ 0xFFFFFFFFUL;
}
unsigned int adler32(const unsigned char* buf, size_t buf_length)
{
	uint32_t s1 = 1;
	uint32_t s2 = 0;

	while (buf_length--)
	{
		s1 = (s1 + *(buf++)) % 65521;
		s2 = (s2 + s1) % 65521;
	}
	return (s2 << 16) + s1;
}

MyPNG::MyPNG(const char* filename) : header() {
	std::ifstream inFile(filename, std::ios_base::binary);
	if (!inFile) {
		throw "Can't open file";
	}

	char signature[9]{'\0'};
	inFile.read(signature, 8);

	char PNGsignature[] = { 137,80,78,71,13,10,26,10 };

	if (memcmp(signature,PNGsignature,8) != 0) {
		throw "not A png file";
	}

	ReadChunks(inFile);
}

void MyPNG::ReadChunks(std::ifstream& inFile) {

	
	while (inFile.peek() != EOF) {
		int chunklength = 0;
		char chunkType[5]{'\0'};

		inFile.read(reinterpret_cast<char*>(&chunklength), 4); // read 4 bytes as int
		reverseInt(chunklength);

		inFile.read(chunkType, 4); 

		if ( strcmp(chunkType, "IHDR") == 0) {
			ReadIHDRChunk(inFile);
		}
		else if (strcmp(chunkType, "IDAT") == 0) {
			ReadIDATChunk(inFile,chunklength);
		}
		else if(strcmp(chunkType,"IEND") == 0) {
			break;
		}
		else if (strcmp(chunkType, "PLTE") == 0) {
			ReadPLTEChunk(inFile, chunklength);
		}


		else  {
			inFile.seekg(chunklength +4, std::ios_base::cur);
		}




	}


}



void MyPNG::ReadIHDRChunk(std::ifstream& inFile) {

	BYTE crcbuf[17]{73,72,68,82};
	inFile.read((char *)crcbuf + 4, 13);

	int crc = 0;
	inFile.read(reinterpret_cast<char*>(&crc), 4); // read 4 bytes as int
	reverseInt(crc);
	
	if (crc != CRC32_function(crcbuf, 17)) {
		throw "Error crc";
	}


	inFile.seekg(-17, std::ios_base::cur);

	inFile.read(reinterpret_cast<char*>(&header.width), 4); // read 4 bytes as int
	inFile.read(reinterpret_cast<char*>(&header.height), 4); // read 4 bytes as int
	reverseInt(header.width);
	reverseInt(header.height);
	header.bitdepth =  inFile.get();
	header.colorType=  inFile.get();
	header.compersType =  inFile.get();
	header.filteringType = inFile.get();
	header.interlace = inFile.get();


	if (header.compersType != 0 || header.filteringType != 0) {
		throw "not a PNG file";
	}

	pixels = std::move(Pixels(header.width, header.height, header.colorType));
	
	inFile.seekg(4, std::ios_base::cur);

}


#include "cstreams.h"
void MyPNG::ReadIDATChunk(std::ifstream& inFile, int datalenght) {
	if (datalenght < 2) {
		return;
	}


	BYTE* tbuf = new BYTE[datalenght + 10]{'I','D','A','T'};
	inFile.read((char*)tbuf + 4, datalenght );

	int crc = 0;
	inFile.read(reinterpret_cast<char*>(&crc), 4); // read 4 bytes as int
	reverseInt(crc);


	if (crc != CRC32_function(tbuf, datalenght + 4)) {
		throw "Error crc";
	}



	int cmf = tbuf[4]; // Compression method and flags
	int flg = tbuf[5]; // Flags

	

	//unsigned int adler = (((unsigned int)tbuf[datalenght ] << 24)| ((unsigned int)tbuf[datalenght + 1] << 16)
	//					| ((unsigned int)tbuf[datalenght + 2] <<  8) | ((unsigned int)tbuf[datalenght + 3]));
	
	/*	if (adler != adler32(tbuf + 4, datalenght)) {
		throw "Error adler32 ";
	}*/

	
	datalenght -= 2;
	reversebuf buf(datalenght );
	buf.pushline(tbuf + 6, datalenght );
	delete[] tbuf;
	

	while (true) {

		int cur = buf.get();
		bool isEnd = ((cur & 128) == 0) ? false : true;
		int type = (cur & 96) >> 5;
		cur &= 31; // clear use bits

		if (type == 0) {
			ReadNonCopmressedBlocks(buf);
		}
		else if(type == 2){
			ReadHuffmanFixedCode(buf,cur);
		}
		else
		{
			ReadHuffmanDynamicCode(buf, cur);
		}


		if (isEnd) { break; }
	}



	return;

}

void MyPNG::ReadPLTEChunk(std::ifstream& inFile, int datalenght) {

	BYTE* buf = new BYTE[datalenght +4 ]{'P','L','T','E'};
	inFile.read((char*)(buf + 4), datalenght);
	int crc = 0;
	inFile.read(reinterpret_cast<char*>(&crc), 4); // read 4 bytes as int
	reverseInt(crc);

	if (crc != CRC32_function(buf, datalenght+4)) {
		throw "Error crc";
	}

	pixels.indexset(buf + 4, datalenght);

	delete[] buf;
}

void MyPNG::ReadNonCopmressedBlocks(reversebuf& buf) {
	int len = buf.get();
	len <<= 8;
	len |= buf.get();
	int nlen = buf.get();
	nlen <<= 8;
	nlen |= buf.get();

	BYTE* out = new BYTE[len];
	buf.getline(out, len);
	for (int i = 0; i < len; ++i) {
		out[i] = ReverseBytes(out[i]);
	}
	pixels.set(out, len);
	delete[] out;
}

void MyPNG::ReadHuffmanFixedCode(reversebuf& buf, int get) {
	unsigned int cur = get;
	unsigned int reserved = 0;
	int reservedBits = 0;
	int nowUsedBits = 5;
	int expectBits = 2;

	int decodeSize = 1024;
	int decodeLen = 0;
	BYTE* decodBuf = new BYTE[decodeSize]{0};

	while(true) {
		getBites(expectBits, cur, reserved, reservedBits, buf);

		if (fixedHuffman[2][2] <= cur && cur <= fixedHuffman[2][3]) { // len -  offset
			int value = cur - fixedHuffman[2][2] + fixedHuffman[2][0];

			if (value == 256) { break; 	}

			int len;
			int offset;
			readLenAndOffset(value,reserved,reservedBits,len,offset,buf);

			for (int i = 0, st = decodeLen - offset , fn = decodeLen  , scan = st; i < len; ++i) {

				decodBuf[decodeLen++] = decodBuf[scan];
				++scan;
				if (scan >= fn) {
					scan = st;
				}

			}

			cur = 0;
			expectBits = 7;
			continue;
		}
		getBites(1, cur, reserved, reservedBits, buf);
		if (fixedHuffman[0][2] <= cur && cur <= fixedHuffman[0][3]) { // BYTE
			int value = cur - fixedHuffman[0][2] + fixedHuffman[0][0];
			decodBuf[decodeLen++] = BYTE(value);

			cur = 0;
			expectBits = 7;
			continue;
		}
		else if (fixedHuffman[3][2] <= cur && cur <= fixedHuffman[3][3]) { //  len -  offset
			int value = cur - fixedHuffman[3][2] + fixedHuffman[3][0];
			int len;
			int offset;
			readLenAndOffset(value, reserved, reservedBits, len, offset, buf);

			for (int i = 0, st = decodeLen - offset , fn = decodeLen , scan = st; i < len; ++i) {

				decodBuf[decodeLen++] = decodBuf[scan];
				++scan;
				if (scan >= fn) {
					scan = st;
				}

			}

			cur = 0;
			expectBits = 7;
			continue;
		}
		getBites(1, cur, reserved, reservedBits, buf); // BYTE
		int value = cur - fixedHuffman[1][2] + fixedHuffman[1][0];

		decodBuf[decodeLen++] = BYTE(value);
		cur = 0;
		expectBits = 7;
	}

	pixels.set(decodBuf, decodeLen);

}


#include <algorithm>
void MyPNG::ReadHuffmanDynamicCode(reversebuf& buf, int get) {

	unsigned int reserved = 0;
	int reservedBits = 0;

	int HLIT = (ReverseBytes(get) >> 3) + 257;
	int HDIST = 0;
	int HCLEN = 0;

	reserved = buf.get();
	
	HDIST = ReverseBytes(reserved >> 3)>>3 ;
	HDIST += 1;

	reserved &= 7;
	
	
	HCLEN = (reserved<<1);
	reserved = buf.get();

	HCLEN += (reserved >> 7);
	HCLEN = ReverseBytes(HCLEN) >> 4;
	HCLEN += 4;

	reserved &= 127;
	reservedBits = 7;

	unsigned int cur = 0;

	int lens[20] = {0};
	
	
	for (int i = 0; i < HCLEN; ++i) {
		int tmp[] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
		cur = 0;
		getBites(3, cur, reserved, reservedBits, buf);
		reverseUInt(cur);
		cur >>= (32-3);

		lens[tmp[i]] = cur;
	}

	int decoded[19] = { -1};
	DecodsFromlengs(lens, decoded,19);

	int* alf = new int[HLIT + 1] {-1};
	int* alflen = new int[HLIT + 1] {-1};
	int* offsetBuf = new int[HDIST + 1] {-1};
	int* offsetlen = new int[HDIST + 1] {-1};

	
	int pos = 0;
	while (pos < HLIT) {

		int res = find(decoded, lens, 19, cur, reserved, reservedBits, buf);

		if (0  <= res && res <= 15) {
			alflen[pos] = res;
			++pos;
		}
		else if( res == 16) {
			cur = 0;
			getBites(2, cur, reserved, reservedBits, buf);
			reverseUInt(cur);
			cur >>= (32 - 2);
			int prev = alflen[pos - 1];
			for (int i = 0; i < 3 + cur;++i) {
				alflen[pos] = prev;
				++pos;
			}

		}
		else if(res == 17){
			cur = 0;
			getBites(3, cur, reserved, reservedBits, buf);
			reverseUInt(cur);
			cur >>= (32 - 3);
			for (int i = 0; i < 3 + cur; ++i) {
				alflen[pos] = 0;
				++pos;
			}
		}
		else {
			cur = 0;
			getBites(7, cur, reserved, reservedBits, buf);
			reverseUInt(cur);
			cur >>= (32 - 7);
			for (int i = 0; i < 11 + cur; ++i) {
				alflen[pos] = 0;
				++pos;
			}
		}

	}

	pos = 0;
	while (pos < HDIST) {
		int res = find(decoded, lens, 19, cur, reserved, reservedBits, buf);
		if (0 <= res && res <= 15) {
			offsetlen[pos] = res;
			++pos;
		}
		else if (res == 16) {
			cur = 0;
			getBites(2, cur, reserved, reservedBits, buf);
			reverseUInt(cur);
			cur >>= (32 - 2);
			int prev = offsetlen[pos - 1];
			for (int i = 0; i < 3 + cur; ++i) {
				offsetlen[pos] = prev;
				++pos;
			}

		}
		else if (res == 17) {
			cur = 0;
			getBites(3, cur, reserved, reservedBits, buf);
			reverseUInt(cur);
			cur >>= (32 - 3);
			for (int i = 0; i < 3 + cur; ++i) {
				offsetlen[pos] = 0;
				++pos;
			}
		}
		else {
			cur = 0;
			getBites(7, cur, reserved, reservedBits, buf);
			reverseUInt(cur);
			cur >>= (32 - 7);
			for (int i = 0; i < 11 + cur; ++i) {
				offsetlen[pos] = 0;
				++pos;
			}
		}

	}
	
	DecodsFromlengs(alflen, alf, HLIT);
	DecodsFromlengs(offsetlen, offsetBuf, HDIST);

	/*
	for (int t = 0; t < HLIT; ++t) {
		std::cout << t << ": " << alflen[t] << " -- " << alf[t]<<std::endl;
	}
	for (int t = 0; t < HDIST; ++t) {
		std::cout << t << ": " << offsetlen[t] << " -- " << offsetBuf[t] << std::endl;
	}*/



	BYTE* decodedBuf = new BYTE[  (header.height * header.width * 4) + (header.height+1)];
	pos = 0;
	while (true) {

		int res = find(alf, alflen, HLIT, cur, reserved, reservedBits, buf);
		if (res < 256) {
			decodedBuf[pos] = (BYTE)res;
			++pos;
		}
		else if(res == 256){
			break;
		}
		else{
			res -= 257;
			cur = 0;
			getBites(HuffmanLen[res][0], cur, reserved, reservedBits, buf);
			reverseUInt(cur);
			cur >>= (32 - HuffmanLen[res][0]);
			int tlen = HuffmanLen[res][1] + cur;

			cur = 0;
			int tx = find(offsetBuf, offsetlen, HDIST, cur, reserved, reservedBits, buf);
			int distnace =  HuffmanOffset[tx][1];
			
			unsigned int tmp = 0;
			getBites(HuffmanOffset[tx][0], tmp, reserved, reservedBits, buf);
			reverseUInt(tmp);
			tmp >>= (32 - HuffmanOffset[tx][0]);
			distnace += tmp;

			for (int i = 0, st = pos   - distnace, fn = pos, scan = st; i < tlen; ++i) {

				decodedBuf[pos] = decodedBuf[scan];
				++pos;
				++scan;
				if (scan >= fn) {
					scan = st;
				}

			}

		}

	}

	pixels.set(decodedBuf, pos);

	delete[] decodedBuf;
	delete[] alf;
	delete[] alflen;
	delete[] offsetBuf;
	delete[] offsetlen;
}

void getBites(int expectBits, unsigned int& cur, unsigned int& reserved, int& reservedBits, reversebuf& buf) {

	if (reservedBits == 0) {
		reserved = 0;
	}

	if (expectBits == reservedBits) {
		cur <<= expectBits;
		cur |= reserved;
		reserved = 0;
		reservedBits = 0;
		return;
	}

	while (reservedBits < expectBits) {
		reserved <<= 8;
		reserved |= buf.get();
		reservedBits += 8;
	}

	if (expectBits == reservedBits) {
		cur <<= expectBits;
		cur |= reserved;
		reserved = 0;
		reservedBits = 0;
		return;
	}

	cur <<= expectBits;

	cur |= (reserved >> (reservedBits - expectBits));

	reservedBits -= expectBits;
	reserved = (reserved << (32 - reservedBits)) >> (32 - reservedBits);

	if (reservedBits == 0) {
		reserved = 0;
	}

}


void readLenAndOffset(unsigned int value, unsigned int& reserved ,  int& reservedBits, int &len , int &offset ,reversebuf& buf) {
	value -= 257;

	unsigned int cur = 0;
	getBites(HuffmanLen[value][0], cur, reserved, reservedBits, buf);
	reverseUInt(cur);
	cur >>= (32 - HuffmanLen[value][0]);
	len = HuffmanLen[value][1] + cur;

	cur = 0;
	getBites(5, cur, reserved, reservedBits, buf);
	offset = HuffmanOffset[cur][1];
	

	unsigned int tmp = 0;
	getBites(HuffmanOffset[cur][0], tmp, reserved, reservedBits, buf);
	reverseUInt(tmp);
	tmp >>= (32 - HuffmanOffset[cur][0]);
	offset += tmp;

}

void DecodsFromlengs(int* in, int* out,const int len){

	int* bl_count  = new int[len + 30] {0};
	int MAX_BITS = 0;

	for (int i = 0; i < len; ++i) {
		if (in[i] != -1) {
			++bl_count[in[i]];
		}
		MAX_BITS = std::max(MAX_BITS, in[i]);
	}

	int* next_code = new int[MAX_BITS + 1] {0};
	//int max_code = 0;

	bl_count[0] = 0;
	int code = 0;
	for (int bits = 1; bits <= MAX_BITS; ++bits) {
		code = (code + bl_count[bits - 1]) << 1;
		next_code[bits] = code;
		//max_code = std::max(code, max_code);
	}


	for (int n = 0; n < len; ++n) {
		int tlen = in[n];
		if (tlen != 0 ) {
			out[n] = next_code[tlen];
			++next_code[tlen];
		}
	}

	delete[] bl_count;
	delete[] next_code;
}


int find(int* decoded, int* lens, int size, unsigned int cur, unsigned int& reserved, int& reservedBits, reversebuf& buf) {
	int res = -1;
	cur = 0;
	int usedBites = 0;
	while (res < 0) {
		getBites(1, cur, reserved, reservedBits, buf);
		++usedBites;
		for (int i = 0; i < size; ++i) {
			if (usedBites == lens[i] && decoded[i] == cur) {
				cur = 0;
				res = i;
				return res;
			}
		}
		if (usedBites == 30) {
			return 256;
		}

	}
}