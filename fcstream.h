#pragma once

#include <iostream>
#include <fstream>
#include "cstream.h"

class ifcstream : public cstream
{
private:
	std::ifstream* inFile;
	BYTE* buf;
	int buf_len;
	int buf_size;
	int buf_pos;



public:

	ifcstream(const char* filename, int size = 128) :buf_size(size), buf(new BYTE[buf_size]),
		buf_len(0), 
		buf_pos(0)
	{
		inFile = new std::ifstream(filename, std::ios_base::binary);
	}

	ifcstream(std::ifstream& InFile, int size = 128) :
		buf_size(size),
		buf(new BYTE[buf_size]),
		buf_len(0),
		buf_pos(0)
	{
		inFile = &InFile ;
	}

	virtual bool is_open() {
		return inFile->is_open();
	}


	int get() {
		if (buf_pos < buf_len) {
			return buf[buf_pos++];
		}

		// filling the buffer 
		buf_len = inFile->read((char *)buf, buf_size).gcount();
		buf_pos = 1;
		return  buf[0];
	}

	int getline(BYTE* out, int get_len) {

		/*
		for (int i = 0; i < 16; ++i) {
			int tmp = inFile.get();
		}*/


		if (get_len < buf_len - buf_pos) {
			memcpy(out, buf + buf_pos, get_len);
			buf_pos += get_len;
			return 0;
		}

		if (buf_len != 0) {
			memcpy(out, buf + buf_pos, buf_len - buf_pos);
			get_len -= buf_len - buf_pos;
		}


		buf_len = inFile->read((char*)buf, buf_size).gcount();
		buf_pos = get_len;

		memcpy(out, buf, get_len);
		return get_len;

	}

	~ifcstream() {
		delete buf;
		inFile->close();
		delete inFile;
	}
};

class ofcstream : public  cstream {
	std::ofstream outFile;

	ofcstream( const char* filename ,cstream* s ) : cstream(s) {
		outFile.open(filename);
	}

	bool is_open() {
		return prev->is_open() && outFile.is_open();
	}

	int get() {
		 int tmp = prev->get();
		 outFile.put(tmp);
		 return tmp;
	}
	void put() {
		int tmp = prev->get();
		outFile.put(tmp);
		return;
	}
	int getline( BYTE* out , int get_len) {
		
		BYTE* tmp = new BYTE[get_len];
		prev->getline(tmp, get_len);
		outFile.write((const char*)tmp, get_len);
		memcpy(out, tmp, get_len);
		return get_len;
	}
	//TODO : REWRITE TO NORMAL LOGIC
	void write(int get_len) {
		BYTE* tmp = new BYTE[get_len];
		prev->getline(tmp, get_len);
		outFile.write((const char*)tmp, get_len);
		return;
	}

};