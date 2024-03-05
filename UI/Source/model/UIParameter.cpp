/**
 * Base implementation of UIParameter
 * This file is NOT generated
 * Generated subclasses and code are found in UIParameterClasses.cpp
 */

#include <JuceHeader.h>

// some string utils
#include "../util/Util.h"
// for StringList
#include "../util/List.h"

// temporary for dynamic parameters
#include "../model/MobiusConfig.h"
#include "../model/Structure.h"
#include "../model/Preset.h"
#include "../model/Setup.h"
#include "../model/Binding.h"

#include "UIParameter.h"

UIParameter::UIParameter()
{
}

UIParameter::~UIParameter()
{
}

/**
 * Convert a symbolic parameter value into an ordinal.
 * This could support both internal names and display names
 * but it's only using internal names at the moment.
 *
 * This cannot be used for type=string.
 * For type=structure you must use getDynamicEnumOrdinal
 */
int UIParameter::getEnumOrdinal(const char* value)
{
	int ordinal = -1;

	if (value != nullptr) {
		for (int i = 0 ; values[i] != nullptr ; i++) {
			if (StringEqualNoCase(values[i], value)) {
				ordinal = i;
				break;
			}
		}
    }
    return ordinal;
}

/**
 * Convert an ordinal into the symbolic enumeration name.
 * This almost always comes from an (int) cast of
 * a zero based enum value, but check the range.
 * Will be simpler with vector.
 */ 
const char* UIParameter::getEnumName(int ordinal)
{
    const char* name = nullptr;

    if (values != nullptr) {
        int max = 0;
        while (values[max] != nullptr) {
            max++;
        }
        if (ordinal >= 0 && ordinal < max)
          name = values[ordinal];
    }
    return name;
}

/**
 * Calculate the maximum ordinal for a structure parameter.
 * Temporary until we get Query fleshed out.
 *
 * We need to deal with all type=structure parameters plus
 * a few integer parameters.
 *
 *   MobiusConfig::activeSetup
 *   MobiusConfig::activeOverlay
 *   SetupTrack::preset
 *
 *   MobiusConfig::activeTrack
 *   Preset::loopCount
 *   SetupTrack::group
 *   SetupTrack::preset
 * 
 */
int UIParameter::getDynamicHigh(MobiusConfig* container)
{
    int dynamicHigh = 0;
    
    // we'll be starting to have this pattern in several places
    // move to lambdas!
    
    if (this == UIParameterActiveTrack) {
        return container->getTracks();
    }
    else if (this == UIParameterLoopCount) {
        return container->getMaxLoops();
    }
    else if (this == UIParameterGroup) {
        return container->getTrackGroups();
    }
    else if (this == UIParameterPreset) {
        return Structure::count(container->getPresets());
    }
    else if (this == UIParameterActiveSetup) {
        return Structure::count(container->getSetups());
    }
    else if (this == UIParameterActiveOverlay) {
        // this is a weird one, we keep overlays on the same list
        // as the master binding set which cannot be deleted
        // this coincidentlly helps with the "none" ordinal problem
        // because ordinal zero will be the master binding set meaning
        // there is no overlay
        return Structure::count(container->getBindingSets());
    }
    else {
        // must be a static parameter, just return the static high
        dynamicHigh = high;
    }

    return dynamicHigh;
}

/**
 * There is a really messy problem with BindingSets and "overlays"
 * about consnstency between ordinals and the names, since the
 * master binding set is on the list with ordinal zero.
 * Don't have the energy for this right now but need to get back to this.
 */
StringList* UIParameter::getStructureNames(MobiusConfig* container)
{
    StringList* names = nullptr;
    Structure* list = nullptr;
    
    if (this == UIParameterPreset) {
        list = container->getPresets();
    }
    else if (this == UIParameterActiveSetup) {
        list = container->getSetups();
    }
    else if (this == UIParameterActiveOverlay) {
        list = container->getBindingSets();
    }

    if (list != nullptr) {
        names = new StringList();
        while (list != nullptr) {
            names->add(list->getName());
            list = list->getNext();
        }
    }

    return names;
}

int UIParameter::getStructureOrdinal(MobiusConfig* container, const char* name)
{
    int ordinal = -1;
    Structure* list = nullptr;

    // obviously could factor out and share this little dance
    // could also move this to Structure
    if (this == UIParameterPreset) {
        list = container->getPresets();
    }
    else if (this == UIParameterActiveSetup) {
        list = container->getSetups();
    }
    else if (this == UIParameterActiveOverlay) {
        list = container->getBindingSets();
    }

    if (list != nullptr) {
        ordinal = Structure::getOrdinal(list, name);
    }

    return ordinal;
}

const char* UIParameter::getStructureName(MobiusConfig* container, int ordinal)
{
    const char* name = nullptr;
    Structure* list = nullptr;

    // obviously could factor out and share this little dance
    // could also move this to Structure
    if (this == UIParameterPreset) {
        list = container->getPresets();
    }
    else if (this == UIParameterActiveSetup) {
        list = container->getSetups();
    }
    else if (this == UIParameterActiveOverlay) {
        list = container->getBindingSets();
    }

    Structure* s = Structure::get(list, ordinal);
    if (s != nullptr)
      name = s->getName();

    return name;
}

//////////////////////////////////////////////////////////////////////
//
// Global Parameter Registry
//
//////////////////////////////////////////////////////////////////////

std::vector<UIParameter*> UIParameter::Parameters;

void UIParameter::trace()
{
    for (int i = 0 ; i < Parameters.size() ; i++) {
        UIParameter* p = Parameters[i];
        // printf("Parameter %s %s %s\n", p->getName(), getEnumLabel(p->type), getEnumLabel(p->scope));
        printf("Parameter %s\n", p->getName());
    }
}

/**
 * Find a Parameter by name
 * This doesn't happen often so we can do a liner search.
 */
UIParameter* UIParameter::find(const char* name)
{
	UIParameter* found = nullptr;
	
	for (int i = 0 ; i < Parameters.size() ; i++) {
		UIParameter* p = Parameters[i];
		if (StringEqualNoCase(p->getName(), name)) {
            found = p;
            break;
        }
	}
	return found;
}

/**
 * Find a parameter by it's display name.
 * I believe this is used only by the Setup editor.
 */
UIParameter* UIParameter::findDisplay(const char* name)
{
	UIParameter* found = nullptr;
	
	for (int i = 0 ; i < Parameters.size() ; i++) {
		UIParameter* p = Parameters[i];
		if (StringEqualNoCase(p->getDisplayName(), name)) {
			found = p;
			break;	
		}
	}
	return found;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
