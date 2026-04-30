#ifndef _Laba2_IObserver_Header
#define _Laba2_IObserver_Header


struct IObserver
{
    virtual ~IObserver() = default;

    virtual void Update() = 0;
};

#endif
