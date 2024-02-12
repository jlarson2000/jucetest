/*
 * Model for persistent UI configuration.
 *
 * This has been reduced from the original UIConfig to think
 * about other ways of doing things.
 *
 */

#pragma once

#include <vector>
#include <memory>

#include "../util/List.h"
#include "../util/Util.h"

#define DEFAULT_REFRESH_INTERVAL 100
#define DEFAULT_MESSAGE_DURATION 2
#define DEFAULT_ALERT_INTERVALS 10

class UILocation
{
  public:

    UILocation() {
        init();
    }
        
    UILocation(const char* name) {
        init();
        setName(name);
    }
        
    ~UILocation() {
        delete mName;
    }
        
    void setName(const char *name) {
        delete mName;
        mName = CopyString(name);
    }

    const char* getName() {
        return mName;
    }

    void setX(int i) { mX = i; };
    int getX() { return mX; };

    void setY(int i) { mY = i; };
    int getY() { return mY; };
        
    void setDisabled(bool b) { mDisabled = b; };
    bool isDisabled() { return mDisabled; };

  private:

    void init() {
        mName = nullptr;
        mX = 0;
        mY = 0;
        mDisabled = false;
    }

    char* mName;
    int mX;
    int mY;
    bool mDisabled;

};

/**
 * Comments are unclear on the usage of this, some indicate
 * that I replaced this with Binding.  You could have more
 * information in here such as function arguments.  Does
 * it belong here?  Didn't have a separate dialog for button
 * bindings.
 */
class UIButton
{
  public:

    UIButton() {
        mName = nullptr;
        mArguments = nullptr;
    }
        
    UIButton(const char* name) {
        mName = CopyString(name);
        mArguments = nullptr;
    }
        
    ~UIButton() {
        delete mName;
        delete mArguments;
    }

    void setName(const char *name) {
        delete mName;
        mName = CopyString(name);
    }
        
    const char* getName() {
        return mName;
    }

    void setArguments(const char* args) {
        delete mArguments;
        mArguments = CopyString(args);
    }

    const char* getArguments() {
        return mArguments;
    }

    
  private:

    char* mName;
    char* mArguments;
};

class UIConfig
{
  public:

    
    UIConfig();
    ~UIConfig();

    // only have one of these, but could support more
    void setName(const char* name);
    const char* getName();
    
    void setRefreshInterval(int i);
    int getRefreshInterval();

    // not used, what was this for?
    void setAlertIntervals(int i);
    int getAlertIntervals();

    void setMessageDuration(int i);
    int getMessageDuration();

    // captured window size, not used yet
    void setWindowWidth(int w) {
        width = w;
    }

    void setWindowHeight(int h) {
        height = h;
    }

    int getWindowWidth() {
        return width;
    }

    int getWindowHeight() {
        return height;
    }

    std::vector<std::unique_ptr<UILocation>>* getLocations();
    void addLocation(UILocation* l);
    void clearLocations();
    
    std::vector<std::unique_ptr<UIButton>>* getButtons();
    void addButton(UIButton* b);
    void clearButtons();
    
    StringList* getParameters();
    void setParameters(StringList* l);

    StringList* getFloatingStrip();
    void setFloatingStrip(StringList* l);

    StringList* getFloatingStrip2();
    void setFloatingStrip2(StringList* l);

    StringList* getDockedStrip();
    void setDockedStrip(StringList* l);

    // what is this?
    const char* getError();

  private:

    void init();
    void checkDisplayComponents();

    char mError[256];   // captured parser error
    char* mName;
    int mRefreshInterval;
    int mAlertIntervals;    // unused, what was this for?
    int mMessageDuration;
    int width;
    int height;
    
    // can't use entity objects here since they are not copy
    // constructable yet, use unique_ptr for now
    std::vector<std::unique_ptr<UILocation>> locations;
    std::vector<std::unique_ptr<UIButton>> buttons;
    
	StringList* mParameters;
	StringList* mFloatingStrip;
	StringList* mFloatingStrip2;
	StringList* mDockedStrip;

};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
