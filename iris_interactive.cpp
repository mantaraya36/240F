

#include <string>
#include <fstream>
#include <iostream>

#include "allocore/io/al_App.hpp"
#include "allocore/math/al_Random.hpp"
#include "Gamma/Oscillator.h"
#include "Gamma/Envelope.h"

#include <stdlib.h>

using namespace al;

float data[150 * 4];


#define BLOCK_SIZE 512

class Dust {
public:
	Dust () {
		mCounter = 0;
		mOffset = 11025;
		mRange = 4000;
		mTarget = mOffset + mRange * (rand()/(float) RAND_MAX);
	}

	float operator() () {
		if (mCounter++ == mTarget) {
			mCounter == 0;
			mTarget = mOffset + mRange * (rand()/(float) RAND_MAX);
			return 1.0;
		}
		return 0.0;
	}

private:
	int mCounter;
	int mTarget;
	int mOffset;
	int mRange;
};

class SoundGenerator : public SoundSource {
public:
	SoundGenerator (float decayTime = 0.1) {
		env.decay(decayTime);
	}

	void freq(float freq) {
		sine.freq(freq);
	}

	void operator() () {
		float output;
		if (trigger() != 0.0) {
			env.reset();
			sine.phase(0);
		}
		output = sine() * env() * 0.01;
		writeSample(output);
	}

private:
	Dust trigger;
	gam::Decay<float> env;
	gam::Sine<float> sine;
};

class MyApp: public App {
public:

	Light light;

	AudioScene audioScene;
	StereoSpeakerLayout speakerLayout;
	StereoPanner spatializer;
	Listener *listener;
	SoundGenerator source[150];
	rnd::Random<> random;

	MyApp () :
	    audioScene(BLOCK_SIZE),
	    spatializer(speakerLayout)
	{
		initWindow();

		addSphere(graphics().mesh());
		graphics().mesh().generateNormals();
		for (int i = 0; i < 50; ++i) {
			source[i].freq(150 + random.uniform(-1, 1));
			audioScene.addSource(source[i]);
		}
		for (int i = 0; i < 50; ++i) {
			source[i + 50].freq(700 + random.uniform(-2, 2));
			audioScene.addSource(source[i + 50]);
		}
		for (int i = 0; i < 50; ++i) {
			source[i + 50].freq(1800 + random.uniform(-3, 3));
			audioScene.addSource(source[i + 100]);
		}
		gam::sampleRate(44100);
		audioIO().device(0);
		initAudio(44100, BLOCK_SIZE);
		listener = audioScene.createListener(&spatializer);
		listener->compile();

		for (int i = 0; i < 150; ++i) {
			double ft1 = data[i * 4];
			double ft2 = data[(i * 4)+ 1];
			double ft3 = data[(i * 4)+ 2];
			double ft4 = data[(i * 4)+ 3];
			double x = mapRange(ft1, 4.3, 7.9, -10.0, 10.0);
			double y = mapRange(ft2, 2.0, 4.4, -10.0, 10.0);
			double z = mapRange(ft3, 1.0, 6.9, -10.0, 10.0);
//			double size = mapRange(ft4, 0.1, 2.5, 0.25, 0.5);
			source[i].pos(x, y, z);
			source[i].farClip(2);
		}

		audioScene.usePerSampleProcessing(false);

	}

	virtual void onDraw(Graphics &g) override {
		light();
		light.pos(nav().pos());
		for (int i = 0; i < 150; ++i) {
			double ft1 = data[i * 4];
			double ft2 = data[(i * 4)+ 1];
			double ft3 = data[(i * 4)+ 2];
			double ft4 = data[(i * 4)+ 3];
			double x = mapRange(ft1, 4.3, 7.9, -10.0, 10.0);
			double y = mapRange(ft2, 2.0, 4.4, -10.0, 10.0);
			double z = mapRange(ft3, 1.0, 6.9, -10.0, 10.0);
			double size = mapRange(ft4, 0.1, 2.5, 0.25, 0.5);
			if (i < 50) {
				g.color(1,0,0);
			} else if (i < 100) {
				g.color(0, 1, 0);
			} else {
				g.color(0,0,1);
			}
			g.pushMatrix();
			g.translate(x, y, z);
			g.scale(size);
			g.draw(g.mesh());
			g.popMatrix();
		}
		Vec3d position = nav().pos();
		listener->pos(position.x, position.y, position.z);

	}

	virtual void onSound(AudioIOData &io)
	{
		while(io()) {
			for (int i = 0; i < 150; ++i) {
				source[i]();
			}
		}

		audioScene.render(io);
	}

};


int main(int argc, char *argv[])
{
	std::ifstream file;
	file.open("240F/iris_data.bin",
	                             std::ifstream::in);
	if(file.is_open()) {
		file.read((char *) data, sizeof(float) * 150 * 4);
	}
	std::cout << data[0] << "  " << data[1] << std::endl;

	MyApp().start();
	return 0;
}


