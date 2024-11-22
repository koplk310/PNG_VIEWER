#pragma once


typedef unsigned char BYTE;

class reversebuf
{
private:
	BYTE* buf;
	int size;   // reserved bytes
	int first;  // position of first byte in buf 
	int last;   // positions of last byte in buf


	void resize(int);
public:
	reversebuf(int size) : size(size), first(0),last(0), buf(new BYTE[size]) {}
	
	void pushline(BYTE* , int );
	int getline(BYTE* , int );
	int get();

};


BYTE ReverseBytes(BYTE);
