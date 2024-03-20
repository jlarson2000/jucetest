/**
 * Utiltiies for loading Sample files.
 * This could be inside MobiusInterface, but I'd like to
 * keep file handling above to limit Juce and OS awareness.
 * This could also be a service provided by the MobiusContainer
 * which would fit with other things.
 * 
 */
#include <JuceHeader.h>

#include "../util/Trace.h"
#include "../model/SampleConfig.h"

#include "WaveFile.h"
#include "SampleReader.h"

SampleConfig* SampleReader::loadSamples(SampleConfig* src)
{
    SampleConfig* loaded = new SampleConfig();
    if (src != nullptr) {
        Sample *srcSample = src->getSamples();
        while (srcSample != nullptr) {
            const char* filename = srcSample->getFilename();
            if (filename != nullptr) {
                juce::File file(filename);
                if (!file.exists()) {
                    Trace(1, "Sample file not found: %s\n", filename);
                }
                else {
                    Sample* copySample = readWaveFile(file);
                    if (copySample == nullptr) {
                        Trace(1, "Sample data could not be read: %s\n", filename);
                    } 
                    else {
                        loaded->add(copySample);
                    }
                }
            }
            srcSample = srcSample->getNext();
        }
    }

    return loaded;
}

/**
 * Should try to learn juce::AudioFormatReader but
 * punting for now and using my old utilitity.  I know
 * it works and i'm not sure if it was doing stereo
 * sample interleaving in the same way.
 */
Sample* SampleReader::readWaveFile(juce::File file)
{
    Sample* sample = nullptr;
    float* data = nullptr;

    const char* filepath = file.getFullPathName().toUTF8();
	WaveFile* wav = new WaveFile();
	int error = wav->read(filepath);
	if (error) {
		Trace(1, "Error reading file %s %s\n", filepath, 
			  wav->getErrorMessage(error));
	}
	else {
        sample = new Sample(filepath);
        sample->setData(wav->stealData(), wav->getFrames());
	}
	delete wav;

	return sample;
}
