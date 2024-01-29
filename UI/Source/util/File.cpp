/**
 * File utilities using Juce
 */

#include <JuceHeader.h>

/**
 * Read a file entirely into a string.
 * The returned string must be deleted.
 * Should start using String or std::string
 * but until everything gets converted here we are.
 */

char* ReadFile(const char* path)
{
    char* string = nullptr;

    juce::File file (path);
    if (!file.existsAsFile()) {
        DBG("File does not exist");
    }
    else {
        FileInputStream instream (file);
        if (!input.openedOk()) {
            DBG("Problem opening FileInputStream");
        }
        else {
            String content = input.readString();
            string = CopyString(content.getText());
        }
    }

    return string;
}

/**
 * Write a file from a string.
 * Overwrite it if it already exists.
 * Return non-zero if there was an error.
 */
int WriteFile(const char* path, const char* contents)
{
    juce::File file (path);
    

}
