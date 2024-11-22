#pragma once

#include "cstream.h"
#include <iostream>

class bufcstream : public cstream {

	BYTE* data = nullptr;
	int size;
	int pos;

public:
	bool is_open() {
		return data == nullptr;
	}

	bufcstream(const BYTE* Data, int Size) :size(Size), pos(0) {
		data = new BYTE[size];
		memcpy(data, Data, size);
	}

	int get() {
		if (pos < size)
			return data[pos++];

		return EOF;
	}

	int getline(BYTE* out, int get_len) {
		int ret_len = std::min(get_len, size - pos);

		if (ret_len == 0) {
			return EOF;
		}
		memcpy(out, data, ret_len);
		return ret_len;
	}


	~bufcstream() {
		delete[] data;
	}

};