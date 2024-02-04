/**
 * Random debugging shit to figure out Juce.
 */

#pragma once

#include <JuceHeader.h>

#include "../util/Trace.h"
#include "../util/Vbuf.h"

#include "JuceUtil.h"

/**
 * Dump a component hierarchy.
 * Would be nice to figure out typeid or dynamic cast or
 * some fucking way to get the text class name like debuggers do.
 */
void JuceUtil::dumpComponent(juce::Component* c, int indent)
{
    juce::String s;

    for (int i = 0 ; i < indent ; i++)
      s += " ";

    if (c->getName().length() > 0)
      s += c->getName();
    else
      s += "???";
    
    s += ": ";
    s += c->getX();
    s += " ";
    s += c->getY();
    s += " ";
    s += c->getWidth();
    s += " ";
    s += c->getHeight();
    if (c->isVisible()) {
        s += " visible";
    }
    s += "\n";

    trace(s.toUTF8());

    indent += 2;

    const juce::Array<juce::Component*>& children = c->getChildren();
    for (int i = 0 ; i < children.size() ; i++) {
        juce::Component* child = children[i];
        dumpComponent(child, indent);
    }
}

