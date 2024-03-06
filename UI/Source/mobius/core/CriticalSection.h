
#pragma once

class CriticalSection
{
  public:

    CriticalSection() {
    }

    ~CriticalSection() {
    }
    
    void setSpin(int spin) {
    }

	void enter() {
    }
    
	void enter(const char* reason) {
    }
            
	void leave() {
    }
    
	void leave(const char* reason) {
    }
};
