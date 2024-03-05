/**
 * ConfigPanel to edit track setups
 */

#pragma once

#include <JuceHeader.h>

#include "../../model/Setup.h"
#include "../common/Form.h"
#include "../common/SimpleRadio.h"
#include "../common/SimpleButton.h"
#include "ConfigPanel.h"

class SetupPanel : public ConfigPanel, public SimpleRadio::Listener, public juce::Button::Listener
{
  public:
    SetupPanel(class ConfigEditor *);
    ~SetupPanel();

    // ConfigPanel overloads
    void load();
    void save();
    void cancel();

    // ObjectSelector overloads
    void selectObject(int ordinal);
    void newObject() override;
    void deleteObject() override;
    void revertObject() override;
    void renameObject(juce::String) override;

    void radioSelected(class SimpleRadio* r, int index);
    void buttonClicked(juce::Button* b);
    
  private:

    void render();
    void initForm();
    void addField(const char* tab, class UIParameter* p);
    class Field* buildResetablesField();
    
    void loadSetupFields();
    void saveSetupFields();
    Setup* getSelectedSetup();
    
    bool active = false;
    juce::OwnedArray<Setup> setups;
    // this will be in FormPanel's OwnedArray
    SimpleRadio* trackSelector = nullptr;
    SimpleButton* initButton = nullptr;
    SimpleButton* initAllButton = nullptr;
    SimpleButton* captureButton = nullptr;
    SimpleButton* captureAllButton = nullptr;
    int selectedSetup = 0;
    int selectedTrack = 0;

    Form form;
};
