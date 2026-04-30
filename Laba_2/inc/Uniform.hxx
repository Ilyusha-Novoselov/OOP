#ifndef _Laba2_Uniform_Header
#define _Laba2_Uniform_Header

#include <IDistribution.hxx>
#include <IPersistent.hxx>


class Uniform : public IDistribution, public IPersistent
{
public:
    Uniform(double theShift = 0.0, double theScale = 1.0) :
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

    IDistribution* Clone() const override;
    std::string Name() const override;

private:
    double myShift;
    double myScale;
};

#endif
