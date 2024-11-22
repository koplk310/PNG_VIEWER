#pragma once

typedef unsigned char BYTE;



class cstream {

protected:
	cstream* prev;

public:
	cstream(cstream* s = nullptr) : prev(s) {}
	virtual bool is_open() = 0;
	virtual int get() = 0;
	virtual int getline(BYTE*, int) = 0;

};


