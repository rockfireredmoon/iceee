#pragma once

#ifndef RANDOM_H
#define RANDOM_H

#include <random>

class RandomManager
{
public:
	RandomManager();
	~RandomManager();

	std::default_random_engine randomEngine;

	int RandInt_32bit(int min, int max);
	int RandInt(int min, int max);
	int RandMod(int max);
	int RandI(int max);
	int RandModRng(int min, int max);
	double RandDbl(double min, double max);
};

extern RandomManager g_RandomManager;

#endif // RANDOM_H
