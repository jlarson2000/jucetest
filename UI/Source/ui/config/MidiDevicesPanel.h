/**
 * ConfigPanel to configure MIDI devices when running standalone.
 */

#pragma once

#include <JuceHeader.h>

#include "../common/Form.h"
#include "ConfigPanel.h"

class MidiDevicesPanel : public ConfigPanel 
{
  public:
    MidiDevicesPanel(class ConfigEditor*);
    ~MidiDevicesPanel();

    // ConfigPanel overloads
    void load();
    void save();
    void cancel();

  private:

    void render();
    void initForm();

    class Form form;
    class Field* inputField = nullptr;
    class Field* outputField = nullptr;
    

};
