/*
 * Utility to examine two Audio files and analyze the differences.
 * Designed for the unit tests so we can allow some degree of
 * slight differences due to floatng point math roundoffs but
 * still detect large anomolies.
 *
 * Partial results are traced, full results are saved to a file.
 *
 */

#pragma once

#include <JuceHeader.h>

class AudioDifferencer
{
  public:

    AudioDifferencer();
    ~AudioDifferencer();

    void diff(class UnitTests* units, class KernelEvent* e);

    void test(class UnitTests* units);
    
  private:
    
    void doTest(class Audio* a1, class Audio* a2);
    void doTest2(Audio* a1, Audio* a2);

    void diffAudio(const char* path1, Audio* a1,
                   const char* path2, Audio* a2,
                   bool reverse);

};
