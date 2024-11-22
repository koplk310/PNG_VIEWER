#include "reversebuf.h"
#include <iostream> // for copy
#include <bit>


void reversebuf::resize(int newsize) {
	if (newsize <= size) {
		// shift all elements
		if (first == 0) { return; }

		int cur = 0; // pos
		for (int i = first; i < last; ++i) {
			buf[cur++] = buf[i];
		}
		last -= first;
		first = 0;
	}
	else {
		BYTE* tmp = new BYTE[newsize];
		memcpy(tmp, buf + first, last - first);
		delete[] buf;
		buf = tmp;
		tmp = nullptr;
		
		size = newsize;
	}
}

BYTE ReverseBytes(BYTE in) {
	in = (in & 0xF0) >> 4 | (in & 0x0F) << 4;
	in = (in & 0xCC) >> 2 | (in & 0x33) << 2;
	in = (in & 0xAA) >> 1 | (in & 0x55) << 1;
	return in;
}

void  reversebuf::pushline(BYTE* in, int inlen) {
	if (inlen > size - last) {
		resize(size +  ( inlen - (size - last)) );
	}

	memcpy(buf + last, in, inlen);

	//for reverse BYTES in buf

	for (int i = last; i < last + inlen; ++i) {
		buf[i] = ReverseBytes(buf[i]);
	}

	last += inlen;

}


int reversebuf::getline(BYTE* out, int outlen) {

	int len = std::min(last - first, outlen);

	memcpy(out, buf + first, len);
	first += len;
	return len;

}

int reversebuf::get() {
	if (first == last) {
		return -1;
	}
	return (int)buf[first++];
}
