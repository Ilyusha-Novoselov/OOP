#ifndef _Laba2_IDistribution_Header
#define _Laba2_IDistribution_Header

#include <random>
#include <string>

struct IDistribution
{
    virtual ~IDistribution() = default;

    virtual double Density(double theX) const = 0;
    virtual double ExpectedValue() const = 0;
    virtual double Variance() const = 0;
    virtual double Asymmetry() const = 0;
    virtual double Kurtosis() const = 0;
    virtual double RandNum() = 0;

    // ПУНКТ 2.1: Функция клонирования и функция возврата имени класса
    virtual IDistribution* Clone() const = 0;
    virtual std::string Name() const = 0;

protected:
    static std::default_random_engine myEngine;
};

#endif