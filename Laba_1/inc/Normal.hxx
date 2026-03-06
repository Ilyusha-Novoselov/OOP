#ifndef _Laba1_Normal_Header
#define _Laba1_Normal_Header

#include <IDistribution.hxx>
#include <IPersistent.hxx>


class Normal : public IDistribution, public IPersistent
{
public:
    Normal(double theShift = 0.0, double theScale = 1.0) :
        myShift(theShift),
        myScale(theScale > 0 ? theScale : 1.0)
    {};

    double Density(double theX) const override;
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

    std::normal_distribution<> myDist;
};

#endif
