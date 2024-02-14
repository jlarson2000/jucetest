/**
 * Base model for major operating modes within the engine.
 * These are part of the MobiusState model.
 * A track will always be in one of these modes.
 *
 * Like FunctionDefinition, the engine will subclass this to
 * add mode specific properties and behavior.
 */

#pragma once

#include <vector>

class ModeDefinition : public SystemConstant
{
  public:

	ModeDefinition(const char* name);
	virtual ~ModeDefinition();

    int ordinal;				// internal number for indexing

    //////////////////////////////////////////////////////////////////////
    // Global Function Registry
    //////////////////////////////////////////////////////////////////////

    static std::vector<ModeDefinition*> Modes;
    static void dumpModes();
	static ModeDefinition* getMode(const char* name);

};
