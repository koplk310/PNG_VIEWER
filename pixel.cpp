#include "pixel.h"


void Pixels::createTruecolor() {
	RGB zero = { 0,0,0 };
	truecolor = new RGB * [height];
	for (size_t i = 0; i < height; i++){
		truecolor[i] = new RGB[width]{zero};
	}
}
void Pixels::createAlfa() {

	Alfa zero = { 0 };
	alfa = new Alfa*[height];
	for (size_t i = 0; i < height; i++) {
		alfa[i] = new Alfa[width]{ zero };
	}
}
void Pixels::createIndexed() {
	RGB zero = { 0,0,0 };
	indexed = new RGB[256]{ zero };
}

void Pixels::clear() {
	clearTruecolor();
	clearAlfa();
	clearIndexed();
}

void Pixels::clearTruecolor() {
	if (truecolor != nullptr) {
		for (size_t i = 0; i < height; i++) {
			delete[] truecolor[i];
		}
		delete[] truecolor;
		truecolor = nullptr;
	}
}
void Pixels::clearAlfa() {
	if (alfa != nullptr) {
		for (size_t i = 0; i < height; i++) {
			delete[] alfa[i];
		}
		delete[] alfa;
		alfa = nullptr;
	}
}
void Pixels::clearIndexed() {
	if (indexed != nullptr) {
		delete[] indexed;
		indexed = nullptr;
	}
}

void Pixels::subFilter(BYTE* in, int inlen,int byteInBlock) {
	for (int i = byteInBlock; i < inlen; i+=byteInBlock) {
		for (int j = 0; j < byteInBlock; ++j) {
			in[i + j] = (BYTE)(((int)in[i + j] + (int)in[ i +j - byteInBlock]) % 256);
		}
	}
}
void Pixels::upFilter(BYTE* cur, BYTE* prev, int len) {
	if (prev == nullptr) { return; }

	for (int i = 0; i < len; ++i) {
		cur[i] = (BYTE)(((int)cur[i] + (int)prev[i]) % 256);
	}
}
void Pixels::AverageFilter(BYTE* cur, BYTE* prev, int len, int byteInBlock) {
	if (prev == nullptr) {
		for (int i = byteInBlock; i < len; i += byteInBlock) {
			for (int j = 0; j < byteInBlock; ++j) {
				cur[i + j] = (BYTE)(((int)cur[i +j] + (int)(cur[ i+j - byteInBlock]) / 2)% 256);
			}
		}
		return;
	}

	for (int j = 0; j < byteInBlock; ++j) {
		cur[j] = (BYTE)(((int)cur[j] + (int)(prev[j ]) / 2) % 256);
	}

	for (int i = byteInBlock; i < len; i += byteInBlock) {
		for (int j = 0; j < byteInBlock; ++j) {
			cur[i+j] = (BYTE)((int)cur[i+j] +
				(((int)cur[i+j - byteInBlock] + (int)prev[i+j]) % 256 / 2)
				) % 256;
		}
	}




}
void Pixels::PaethFilter(BYTE* cur, BYTE* prev, int len , int byteInBlock) {
	int a=0;
	int b=0;
	int c=0;
	int p=0;


	for (int i = 0; i < len; i += byteInBlock) {
		for (int j = 0; j < byteInBlock; ++j) {
			
			if ( j < byteInBlock ) { a = 0; }
			else{ a = (BYTE)(cur[i+j - byteInBlock]); }

			if (prev == nullptr) { b = 0; }
			else { b = (BYTE)(prev[i+j]); }

			if (prev == nullptr || j < byteInBlock) { c = 0; }
			else { c = (BYTE)(prev[i+j - byteInBlock]); }

			p = a + b - c;
			
			int pa = std::abs(p - a );
			int pb = std::abs(p - b );
			int pc = std::abs(p - c );
			int pr = 0;

			if (pa <= pb && pa <= pc) { pr = a; }
			else if (pb <= pc) { pr = b; }
			else { pr = c; }

			cur[i+j] = (BYTE)(((int)cur[i+j] + pr) % 256);
		}
	}

}


void Pixels::set(BYTE* in , int inlen ) {
	
	int byteInBlock = 1;
	bool isIndex = false;
	if (type == 2 || type == 6  ) {
		byteInBlock = 3;
	}
	if (type == 6 || type == 4) {
		++byteInBlock;
	}
	if (type == 3) {
		isIndex = true;
	}


	int tlen = width * byteInBlock;
	BYTE* prev = nullptr;
	BYTE* cur = new BYTE[tlen];
	

	int pos = 0;
	int theight = 0;

	while (pos < inlen ) {
		BYTE filter_type = in[pos++];
		
		memcpy(cur, in + pos, tlen);
		pos += tlen;

		switch (filter_type)
		{
			/*case 0: { // no filter 
				break;
			}*/
			case 1: { // subFilter
				subFilter(cur,tlen,byteInBlock);
				break;
			}
			case 2: {
				upFilter(cur, prev, tlen);
				break;
			}
			case 3: {
				AverageFilter(cur, prev, tlen, byteInBlock);
				break;
			}
			case 4: {
				PaethFilter(cur, prev, tlen, byteInBlock);
				break;
			}

		}
		
		for (int i = 0, j = 0; i < width && j < tlen;++i) {
			if ( byteInBlock >= 3  ) { // truecolor or truecolor + alfa
				truecolor[theight][i].Red = cur[j++];
				truecolor[theight][i].Green = cur[j++];
				truecolor[theight][i].Blue = cur[j++];
			}
			else if(isIndex) {  
				BYTE index = cur[j++];
				truecolor[theight][i] = { indexed[index].Red,indexed[index].Green,indexed[index].Blue };
			}
			else{
				BYTE grayScale = cur[j++];
				truecolor[theight][i] = { grayScale,grayScale,grayScale };
			}

			if (byteInBlock == 2 || byteInBlock == 4) {
				alfa[theight][i].scale = cur[j++];
			}

		}

		++theight;

		std::swap(prev, cur);
		if (cur == nullptr) {
			cur =  new BYTE[tlen];
		}
	}
	delete[] cur;
	delete[] prev;
}


void  Pixels::indexset(BYTE* buf, int inlen) {
	int pos = 0;
	int index = 0;
	while (pos < inlen) {
		indexed[index].Red = buf[pos++];
		indexed[index].Green = buf[pos++];
		indexed[index].Blue = buf[pos++];
		++index;
	}
}

