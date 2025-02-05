#include "Random.h"
#include <chrono>

RandomManager g_RandomManager;

RandomManager :: RandomManager()
{
	randomEngine.seed(std::chrono::system_clock::now().time_since_epoch().count());
}

RandomManager :: ~RandomManager()
{
}

int RandomManager::RandInt_32bit(int min, int max) {
	// Generate a 32 bit random number.

//		/*
//			Explanation:
//			rand() doesn't work well for larger numbers.
//			RAND_MAX is limited to 32767.
//			There are other quirks, where powers of two seem to generate more even
//			distributions of numbers.
//
//			Since smaller numbers have better distribution, use a sequence
//			of random numbers and use those to fill the bits of a larger number.
//		*/
//
//		// RAND_MAX (as defined with a value of 0x7fff) is only 15 bits wide.
//		if(min == max)
//			return min;
//		unsigned long rand_build = (rand() << 15) | rand();
//		//unsigned long rand_build = ((rand() & 0xFF) << 24) | ((rand() & 0xFF) << 16) | ((rand() & 0xFF) << 8) | ((rand() & 0xFF));
//		return min + (rand_build % (max - min + 1));

	// TODO above should not be necessary now
	return RandInt(min, max);
}

int RandomManager::RandInt(int min, int max) {
	//return (int) (((double) rand() / ((double)RAND_MAX + 1) * ((max + 1) - min)) + min);
	std::uniform_int_distribution<int> intDistro(min,max);
	return intDistro(randomEngine);
}

int RandomManager::RandMod(int max) {
	if(max < 2)
		return 0;
//	// Max is exclusive, e.g, max of 10 would give numbers between 0 and 9
//	return rand()%max;
	return RandInt(0, max - 1);
}

int RandomManager::RandI(int max) {
	return RandInt(1, max);
}
int RandomManager::RandModRng(int min, int max) {

	if(min == max)
		return min;
	// Min is inclusive, max is exclusive, e.g, min of 3, max of 10 would give numbers between 3 and 9
	//return(rand()%(max-min)+min);

	return RandInt(min, max - 1);
}
double RandomManager::RandDbl(double min, double max) {
	//return ((double)rand() / ((double)RAND_MAX) * (max - min)) + min;
	std::uniform_real_distribution<double> dblDistro(min,max);
	return dblDistro(randomEngine);
}

