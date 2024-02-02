
#include <string>
#include <sstream>

#include <JuceHeader.h>

#include "Form.h"
#include "../util/qtrace.h"


Form::Form()
{
}

Form::~Form()
{
}

void Form::add(Field* f, const char* tab, int column)
{
    FieldSet* fields = nullptr;
    
    if (tab == nullptr) {
        // just dump them in the first set
        if (tabs.size() == 0) {
            // note the set won't have a name, should only be doing this
            // if you want no tabs
            fields = new FieldSet();
            tabs.set(0, fields);
        }
        else {
            fields = tabs[0];
        }
    }
    else {
        for (int i = 0 ; i < tabs.size() ; i++) {
            FieldSet* tabset = tabs[i];
            if (tabset->getName() == tab) {
                fields = fields;
                break;
            }
        }

        if (fields == nullptr) {
            fields = new FieldSet();
            fields->setName(tab);
            tabs.add(fields);
        }
    }

    fields->add(f, column);
}

void Form::render()
{
    // todo: support tabs
    for (int i = 0 ; i < tabs.size() ; i++) {
        FieldSet* fieldset = tabs[i];
        fieldset->render();
    }
}

//////////////////////////////////////////////////////////////////////
//
// FormIterator
//
//////////////////////////////////////////////////////////////////////

Form::Iterator::Iterator(Form* argForm)
{
    form = argForm;
    advance();
}

Form::Iterator::~Iterator()
{
}

void Form::Iterator::reset()
{
    tabIndex = 0;
    colIndex = 0;
    fieldIndex = 0;
    nextField = nullptr;
    advance();
}

bool Form::Iterator::hasNext()
{
    return (nextField != nullptr);
}

Field* Form::Iterator::next()
{
    Field* retval = nextField;
    advance();
    return retval;
}

void Form::Iterator::advance()
{
    nextField = nullptr;
    
    while (nextField == nullptr && tabIndex < form->tabs.size()) {
        FieldSet* fieldSet = form->tabs[tabIndex];
        while (colIndex < fieldSet->getColumns()) {
            juce::OwnedArray<Field>* fields = fieldSet->getColumn(colIndex);
            // should not have nulls in here but this is harder to enforce
            if (fields != nullptr && fieldIndex < fields->size()) {
                nextField = (*fields)[fieldIndex];
                fieldIndex++;
            }
            else {
                colIndex++;
                fieldIndex = 0;
            }
        }
        if (nextField == nullptr) {
            tabIndex++;
            colIndex = 0;
            fieldIndex = 0;
        }
    }
}    
