#ifndef _Laba2_IPersistent_Header
#define _Laba2_IPersistent_Header

#include <iostream>


struct IPersistent
{
    virtual ~IPersistent() = default;

    virtual void Save(std::ostream& theOut) const = 0;
    virtual void Load(std::istream& theIn) = 0;
};

#endif
