#ifndef _Laba2_GeneralDistribution_Header
#define _Laba2_GeneralDistribution_Header

#include <IDistribution.hxx>
#include <IPersistent.hxx>

#include <iostream>


class GeneralDistribution : public IDistribution, public IPersistent
{
public:
    GeneralDistribution(std::istream& theIn);
    GeneralDistribution(const GeneralDistribution& theOther);
    GeneralDistribution(const IDistribution& theOther);
    ~GeneralDistribution();

    GeneralDistribution& operator=(const GeneralDistribution& theOther);

    double Density(double theX) const override;
    double ExpectedValue() const override;
    double Variance() const override;
    double Asymmetry() const override;
    double Kurtosis() const override;
    double RandNum() override;

    IDistribution* Clone() const override;
    std::string Name() const override;

    void Save(std::ostream& theOut) const override;
    void Load(std::istream& theIn) override;

    IDistribution& Component();
    const IDistribution& Component() const;

private:
    IDistribution* myLetter;
};

#endif
