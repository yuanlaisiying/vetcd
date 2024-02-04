#ifndef __LY_UI_EVENT_H__
#define __LY_UI_EVENT_H__

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class ui_event
{
public:
    ui_event()
    {
#ifdef _MSC_VER
        hevent_ = ::CreateEvent(NULL, false, false, NULL);
#else
        //TODO: implement
#endif
    }
    ~ui_event()
    {
#ifdef _MSC_VER
        ::CloseHandle(hevent_);
#else
#endif
    }

public:
    bool Wait(uint32_t nto)
    {
#ifdef _MSC_VER
        uint32_t result = ::WaitForSingleObject(hevent_, nto);
        if (result == WAIT_OBJECT_0)
        {
            return true;
        }
        else
        {
            return false;
        }
#else
        return false;
#endif
    }
    void Set()
    {
#ifdef _MSC_VER
        SetEvent(hevent_);
#else
        
#endif
    }

private:
#ifdef _MSC_VER
    HANDLE hevent_;
#else
#endif
};



#endif // __LY_UI_EVENT_H__
