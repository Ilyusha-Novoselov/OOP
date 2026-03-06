#ifndef _Laba1_IObserver_Header
#define _Laba1_IObserver_Header


struct IObserver
{
    virtual ~IObserver() = default;

    virtual void Update() = 0;
};

#endif
