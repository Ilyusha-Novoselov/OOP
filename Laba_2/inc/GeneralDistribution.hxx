#ifndef _Laba2_GeneralDistribution_Header
#define _Laba2_GeneralDistribution_Header

#include <IDistribution.hxx>
#include <IPersistent.hxx>
#include <iostream>

// ПУНКТ 2.3: Класс "общего распределения" (Конверт/Письмо)
class GeneralDistribution : public IDistribution, public IPersistent
{
public:
    GeneralDistribution(std::istream& theIn);
    GeneralDistribution(const GeneralDistribution& theD0);
    GeneralDistribution(const IDistribution& theD0);
    ~GeneralDistribution() override;

    GeneralDistribution& operator=(const GeneralDistribution& theD0);

    IDistribution* Clone() const override;
    std::string Name() const override;

    double Density(double theX) const override;
    double ExpectedValue() const override;
    double Variance() const override;
    double Asymmetry() const override;
    double Kurtosis() const override;
    double RandNum() override;

    void Save(std::ostream& theOut) const override;
    void Load(std::istream& theIn) override;

    IDistribution& Component();
    const IDistribution& Component() const;

private:
    IDistribution* myLetter;
};

#endif