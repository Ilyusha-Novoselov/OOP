#include <Normal.hxx>

#include <cmath>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


double Normal::Density(double theX) const
{
    double aX = (theX - myShift) / myScale;
    return std::exp(-aX * aX / 2.0) / (std::sqrt(2.0 * M_PI) * myScale);
}

double Normal::ExpectedValue() const { return myShift; }

double Normal::Variance() const { return myScale * myScale; }

double Normal::Asymmetry() const { return 0.0; }

double Normal::Kurtosis() const { return 0.0; }

double Normal::RandNum()
{
    std::normal_distribution<> d(0.0, 1.0);

    return myShift + myScale * d(myEngine);
}

void Normal::Save(std::ostream& theOut) const
{
    theOut << "Normal " << myShift << " " << myScale << "\n";
}

void Normal::Load(std::istream& theIn)
{
    std::string aName;

    theIn >> aName >> myShift >> myScale;
}