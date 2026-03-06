#ifndef _Laba1_IGLDistribution_Header
#define _Laba1_IGLDistribution_Header

#include <IDistribution.hxx>
#include <IPersistent.hxx>


class IGLDistribution : public IDistribution, public IPersistent
{
public:
    IGLDistribution(double theShift = 0.0, double theScale = 1.0, double theShape = 2.0) :
        myShift(theShift),
        myScale(theScale > 0 ? theScale : 1.0),
        myShape(theShape)
    {};

    double Density(double x) const override;
    double ExpectedValue() const override;
    double Variance() const override;
    double Asymmetry() const override;
    double Kurtosis() const override;
    double RandNum() override;

    void Save(std::ostream& out) const override;
    void Load(std::istream& in) override;

private:
    double myShift;
    double myScale;
    double myShape;

    std::normal_distribution<> myNorm;
    std::uniform_real_distribution<> myUnif;
    std::exponential_distribution<> myExp;
    std::bernoulli_distribution myBern;
};

#endif
