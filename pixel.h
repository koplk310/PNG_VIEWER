#pragma once
#include "cstreams.h"

struct RGB {
	BYTE Red;
	BYTE Green;
	BYTE Blue;
};

struct Alfa{
	BYTE scale;
};



class Pixels {
private:
	RGB** truecolor;
	RGB* indexed;
	
	Alfa** alfa;

	int type;
	size_t width;
	size_t height;

	void createTruecolor();
	void createAlfa();
	void createIndexed();

	void clear();
	void clearTruecolor();
	void clearAlfa();
	void clearIndexed();


	void subFilter(BYTE*, int,int);
	void upFilter(BYTE*,BYTE*, int);
	void AverageFilter(BYTE*, BYTE*,int,int);
	void PaethFilter(BYTE*, BYTE*, int,int);

public:
	Pixels(int width = 0, int height =0, int type =0) : 
		width(width), height(height), type(type),
		truecolor(nullptr),alfa(nullptr),indexed(nullptr)
	{
		if (type == 0) {
			return;
		}

		createTruecolor();
		if (type == 4 || type == 6) {
			createAlfa();
		}
		if (type == 3) {
			createIndexed();
		}
	
	}
	
	Pixels(const Pixels &other) {
		clear();
		width = other.width;
		height = other.height;
		type = other.type;
		alfa = other.alfa;
		truecolor = other.truecolor;
		indexed = other.indexed;
	}

	Pixels( Pixels&& other) noexcept {
		clear();
		width = other.width;
		height = other.height;
		type = other.type;
		alfa = other.alfa;
		truecolor = other.truecolor;
		indexed = other.indexed;

		other.truecolor = nullptr;
		other.indexed = nullptr;
		alfa = nullptr;
	}

	Pixels& operator=(const Pixels& other) {
		if (this != &other) {
			clear();
			width = other.width;
			height = other.height;
			type = other.type;
			alfa = other.alfa;
			truecolor = other.truecolor;
			indexed = other.indexed;
		}
		return *this;
	}
	Pixels& operator=(Pixels && other) noexcept{
		if (this != &other) {
			clear();
			width = other.width;
			height = other.height;
			type = other.type;
			alfa = other.alfa;
			truecolor = other.truecolor;
			indexed = other.indexed;

			other.truecolor = nullptr;
			other.indexed = nullptr;
			other.alfa = nullptr;
		}
		return *this;
	}

	void set(BYTE*, int);
	RGB** get() { return truecolor; }
	~Pixels() {
		clear();
	}
	void indexset(BYTE*, int);
};



