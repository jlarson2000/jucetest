/**
 * Temporary model mapping functions.
 */

#include "../../model/UIEventType.h"
#include "../../model/FunctionDefinition.h"
#include "../../model/UIParameter.h"
#include "../../model/ModeDefinition.h"
#include "../../model/MobiusConfig.h"
#include "../../model/Setup.h"
#include "../../model/Preset.h"

#include "Event.h"
#include "Function.h"
#include "Parameter.h"
#include "Mode.h"
#include "Mobius.h"

#include "Mapper.h"

//////////////////////////////////////////////////////////////////////
//
// Container Services
//
// Things about the runtime environment that the container needs to provide.
// These can be removed once MobiusContainer propagation within the core
// is finished.
//
//////////////////////////////////////////////////////////////////////

void SleepMillis(int)
{
}
    
bool IsPlugin(class Mobius* m)
{
    return false;
}

//////////////////////////////////////////////////////////////////////.
//
// Constant Objects
//
//////////////////////////////////////////////////////////////////////.

/**
 * Used by EventManager::getEventSummary to build the MobiusState.
 * 
 * Core EventType objects are strewn about all over the Function implementations
 * and have behavior methods with hidden core logic.
 * The UI needs to display these and have some basic information about them.
 * UIEventType is used only in MobiusState
 *
 * Ordinal mapping isn't possible because core events aren't maintained
 * in a static array and assigned ordinals.  I'll start by searching by
 * name which sucks, but we don't ususally have too many events at one time.
 *
 * This looks like a good candidate for the core subclassing the UIEventType
 * to add it's extra behavior.  Could also use a visitor pattern to avoid
 * the name search.
 */
UIEventType* MapEventType(EventType* src)
{
    return (src != nullptr) ? UIEventType::find(src->name) : nullptr;
}

/**
 * Used by EventManager::getEventSummary to build the MobiusState
 *
 * Core Event objects can reference a Function and we want to display
 * The function name associated with the event.
 *
 * This one can use ordinals when we get the mapping vector built out.
 */
class FunctionDefinition* MapFunction(class Function* src)
{
    return (src != nullptr) ? FunctionDefinition::find(src->name) : nullptr;
}

/**
 * Used by Track::resetParameteters to selectively reset parameters
 * stored in the Track-specific Setup copy after the Reset function.
 * This is done by calling this Setup method:
 *
 * if (global || setup->isResetable(MapParameter(InputLevelParameter))) {
 *
 * The Setup model was changed to use UIParameter for isResetable.
 * It maintains a StringList of the parameter names that should be sensitive
 * to reset and just looks for the name in that list.  This list is user
 * specified and can be different in each Setup so it can't go on the Parameter object.
 *
 * Could use ordinals here, but it is only done on Reset and the list is not typically
 * long.  Still, use ordinal mapping when the arrays are ready.
 *
 * This was only used for a few thigns related to Setup::isResetRetains,
 * don't need it now.
 */
class UIParameter* MapParameter(class Parameter* src)
{
    return (src != nullptr) ? UIParameter::find(src->name) : nullptr;
}

/**
 * This is used by Loop to map a core MobiusMode into a UI ModeDefinition.
 * Used only to build the MobiusState.
 *
 * MobiusMode has internal methods like invoke() so the model can't be shared.
 * They are maintained in an array so we could use ordinal mapping with some work.
 * They are simple enough that subclassing may be possible.
 */
class ModeDefinition* MapMode(class MobiusMode* mode)
{
    return (mode != nullptr) ? ModeDefinition::find(mode->name) : nullptr;
}

//////////////////////////////////////////////////////////////////////
//
// Files
//
// Used mostly by Project
// There were lots of uses of Audio->write in debugging code that
// was commented out.
//
// Move all of this to MobiusContainer which will also want control
// over the full path so we don't have absolute paths or assumptions
// about current working directory in the core.
//
//////////////////////////////////////////////////////////////////////

/**
 * Write the contents of an Audio to a file
 */
void WriteAudio(class Audio* a, const char* path)
{
}

/**
 * Write a file with the given content.
 * Used by Project, I think to store the Project XML.
 */
void WriteFile(const char* path, const char* content)
{
}

//////////////////////////////////////////////////////////////////////
//
// Active Structure
//
// The old MobiusConfig model maintained lists of structures: Preset,
// Setup, BindingConfig.  One of these was considered "current" meaning
// it would be active on startup, and after Global Reset.  This was modeled
// by having MobiusConfig keep a pointer to one of the objects on it's
// structure list.  This was transient and not saved in the XML.  And yes,
// it caused awkward problems with object ownership assumptions.
//
// The new model only stores the name of the active structure, which can
// be assumed to match one of the objects on a list.  This is bad
// because changing the active structure at runtime now requires copying
// the name string whereas before we just stored a pointer to something
// that already existed.  It would be better to reference objects by
// ordinal, which is easy internally but it makes the XML less readable.
//
// It would be possible to cache the results of a name search inside the
// container object so we can reuse it, but I'm punting on that since this
// is uncommon and will likely change once we move to vectors with ordinals.
//
// Beyond the "active" concept, some code looks up things by name and ordinal.
// I took those methods out of MobiusConfig for some reason, I guess just
// to get a better handle on where they were called and what sort of optimized
// search structure would be needed.
//
// Also note that old code called the "active" current "current" and the
// name was changed in the new model to get getActiveSomething.  The
// names of the mapper method are reflecting the old Current name.
//
//////////////////////////////////////////////////////////////////////

/**
 * Used by Mobius, ParameterGlobal, Project, Track
 */
Setup* GetCurrentSetup(MobiusConfig* config)
{
    return config->getStartingSetup();
}

/**
 * Name/number search for the Setup list.
 * These are normal but speed isn't that important.
 */
Setup* GetSetup(MobiusConfig* config, const char* name)
{
    return (Setup*)(Structure::find(config->getSetups(), name));
}

Setup* GetSetup(MobiusConfig* config, int number)
{
    return (Setup*)(Structure::get(config->getSetups(), number));
}

/**
 * Used only by SetupNameParameterType::getHigh
 */
int GetSetupCount(MobiusConfig* config)
{
    return Structure::count(config->getSetups());
}

/**
 * Used by Track, formerly by PresetDialog
 *
 * This is a weird one because the notion of a "current" preset is not
 * well defined.  Each SetupTrack has an "active" preset name because
 * each track can have a different active preset.  There was nothing
 * in the MobiusConfig XML for a global default preset but it did have
 * a Preset pointer that would be set with setCurrentPreset by
 * by something else.  If you called getCurrentPreset and mPreset
 * wasn't set, it would bootstrap it to be the first one on the list.
 *
 * Track calls this at various points, apparently when it can't find
 * a Preset referenced by the SetupTrack and it just wants to pick
 * the default one:
 * 
 *  if (this == mMobius->getTrack()) {
 *        // current track follows the lingering selection
 *        // newPreset = config->getCurrentPreset();
 *        newPreset = GetCurrentPreset(config);
 *
 * "lingering selection" probably means when Presets were selected
 * from the main menu.
 *
 * This needs to be made more formal, with a "default preset" stored
 * permanently in the MobiusConfig and SetupTrack::activePreset
 * being optional.  Right now, I think it just starts up with the first
 * preset always until you change it with the menu.
 */ 
Preset* GetCurrentPreset(MobiusConfig* config)
{
    return config->getPresets();
}

/**
 * Used by Mobius, ParameterTrack, Track, Script
 *
 * Finds a Preset by name and in a few cases by number.
 * These are normal since we store names in config objects and
 * reference them by name in scripts.
 *
 * Speed shouldn't be of great importance here, we mostly do this
 * during initialization or after reset with names stored in
 * SetupTrack.  Assimilating a preset change is fairly heavy
 * anyway so optimizing the searching isn't much added.  Ordinal
 * lookup can use a vector eventually.
 */
Preset* GetPreset(MobiusConfig* config, const char* name)
{
    return (Preset*)(Structure::find(config->getPresets(), name));
}

Preset* GetPreset(MobiusConfig* config, int number)
{
    return (Preset*)(Structure::get(config->getPresets(), number));
}

/**
 * Used only by TrackPresetParameterType::getHigh
 */
int GetPresetCount(MobiusConfig* config)
{
    return Structure::count(config->getPresets());
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
