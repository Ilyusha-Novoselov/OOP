#ifndef _Laba1_IDistribution_Header
#define _Laba1_IDistribution_Header

#include <random>


struct IDistribution
{
    virtual ~IDistribution() = default;

    virtual double Density(double theX) const = 0;
    virtual double ExpectedValue() const = 0;
    virtual double Variance() const = 0;
    virtual double Asymmetry() const = 0;
    virtual double Kurtosis() const = 0;
    virtual double RandNum() = 0;

protected:
    static std::default_random_engine myEngine;
};

#endif
