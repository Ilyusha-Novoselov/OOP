#ifndef _Laba1_IPersistent_Header
#define _Laba1_IPersistent_Header

#include <iostream>


struct IPersistent
{
    virtual ~IPersistent() = default;

    virtual void Save(std::ostream& theOut) const = 0;
    virtual void Load(std::istream& theIn) = 0;
};

#endif
