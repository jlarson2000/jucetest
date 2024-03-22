

#include <JuceHeader.h>

#include "../../model/UIConfig.h"
#include "../../model/MobiusState.h"

#include "../../Supervisor.h"

#include "Colors.h"
#include "StatusArea.h"
#include "AlertElement.h"

AlertElement::AlertElement(StatusArea* area) :
    StatusElement(area, "AlertElement")
{
    Supervisor::Instance->addAlertListener(this);
}

AlertElement::~AlertElement()
{
    Supervisor::Instance->removeAlertListener(this);
}

void AlertElement::configure(UIConfig* config)
{
}

/**
 * AlertListener
 */
void AlertElement::alertReceived(juce::String msg)
{
    alert = msg;
    // 5 seconds
    timeout = 50;
    repaint();
}

/**
 * There is nothing in MobiusState that we need to watch
 * but we will use the udpate call to implement the timeout.
 * Assumig every 1/10th second
 */
void AlertElement::update(MobiusState* state)
{
    if (timeout > 0) {
        timeout = timeout - 1;
        if (timeout == 0) {
            alert.clear();
            repaint();
        }
    }
}

int AlertElement::getPreferredHeight()
{
    return 20;
}

int AlertElement::getPreferredWidth()
{
    return 400;
}

void AlertElement::resized()
{
}

void AlertElement::paint(juce::Graphics& g)
{
    // borders, labels, etc.
    StatusElement::paint(g);

    juce::Font font = juce::Font(getHeight() * 0.8f);
    g.drawText(alert, 0, 0, getWidth(), getHeight(), juce::Justification::left);
}
