/**
 * Temporary class encapsulating a simulation
 * of the operation of an actual kernel.
 *
 * Factored out of MobiusShell so we can have a cleaner
 * separation and make it easier to rip out.
 */

#pragma once

class Simulator
{
  public:

    Simulator(class MobiusShell* shell);
    ~Simulator();

    void setListener(class MobiusListener* l);
    
    // ownership is retained by the shell, could also
    // just call back for this
    void initialize(class MobiusConfig* config);

    void doAction(class UIAction* action);
    int getParameter(class UIParameter* p, int tracknum);

    void simulateInterrupt(float* input, float* output, int frames);
    void test();

  private:

    class MobiusShell* shell = nullptr;
    class MobiusConfig* configuration = nullptr;
    class MobiusState* state = nullptr;
    class MobiusListener* listener = nullptr;
    
    void initState();
    void globalReset();
    void doReset(class UIAction* action);
    void reset(class MobiusLoopState* loop);

    void doRecord(class UIAction* action);

    void play(class MobiusLoopState* loop, int bufferFrames);
    void notifyBeatListeners(class MobiusLoopState* loop, long bufferFrames);
    
    void simulateEvents();
    class MobiusEventState* simulateEvent(class MobiusLoopState* loop, class UIEventType* type, int q);

    class MobiusTrackState* getTargetTrack(class UIAction* action);
    void doSwitch(class UIAction* action, int next);

    // Parameters
    class Setup* getActiveSetup();
    class SetupTrack* getSetupTrack(int tracknum);
    class Preset* getTrackPreset(class SetupTrack* track);
    class Preset* getTrackPreset(int tracknum);

};

