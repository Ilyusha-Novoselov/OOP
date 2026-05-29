#ifndef _Laba3_IObserver_Header
#define _Laba3_IObserver_Header


struct IObserver
{
    virtual ~IObserver() = default;

    virtual void Update() = 0;
};

#endif
