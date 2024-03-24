/**
 * Not sure why this is necessary but even with clean classes files,
 * getting unresolved link errors on a few things, seemingly pulled in
 * buy the .obj files we bring over from the real build tree.
 *
 * Since we can't rebuild that until mobu runs, add this to the mobu build
 * just to get it to link.
 *
 * need to sort out why this was necessary.
 *
 * Once everything is back in sync, this should not be necessary
 */

#include "UIParameter.h"

#if 0
UIParameter* UIParameterActiveSetup = nullptr;
UIParameter* UIParameterActiveOverlay = nullptr;
UIParameter* UIParameterLoopCount = nullptr;
UIParameter* UIParameterActiveTrack = nullptr;
UIParameter* UIParameterPreset = nullptr;
UIParameter* UIParameterGroup = nullptr;
#endif

