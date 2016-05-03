
#include <iostream>
#include <fstream>
#include <mutex>

#include "allocore/io/al_AudioIO.hpp"
#include "Gamma/Oscillator.h"

using namespace al;

gam::Sine<> sine(440);
float freq = 440.0;
std::mutex freqMutex;

std::vector<float> data;
int counter;
int dataIndex;


void cb(AudioIOData &io)
{
//	if (freqMutex.try_lock()) {
//		sine.freq(freq);
//		freqMutex.unlock();
//	}
	while (io() ) {
		float val = 0.1 * sine();
		io.out(0) = val;
		counter++;
		if (counter == 22050) {
			sine.freq(data[dataIndex++]);
			if (dataIndex >= data.size()) {
				dataIndex = 0;
			}
			counter = 0;
		}
	}
}

int main(int argc, char *argv[])
{
//	gam::Domain::master().spu(44100);
	gam::sampleRate(44100);
	AudioIO io(1024, 44100.0, cb, NULL, 2,  0);

	float newFreq = freq;
	counter = 0;
	dataIndex = 0;
	io.start();

	std::cout << "Press '0' to quit." << std::endl;

	std::ifstream read("240F/gs10.txt");

	float value;
	while(read >> value) {
		data.push_back(value * 200.0);
	}

	while (newFreq != 0) {
		std::cin >> newFreq;
		freqMutex.lock();
		freq = newFreq;
		freqMutex.unlock();
	}

	return 0;
}

