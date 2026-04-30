#include <Normal.hxx>

#include <cmath>
#include <string>
#include <numbers>


double Normal::Density(double theX) const
{
    double aX = (theX - myShift) / myScale;
    return std::exp(-aX * aX / 2.0) / (std::sqrt(2.0 * std::numbers::pi) * myScale);
}

double Normal::ExpectedValue() const { return myShift; }

double Normal::Variance() const { return myScale * myScale; }

double Normal::Asymmetry() const { return 0.0; }

double Normal::Kurtosis() const { return 0.0; }

double Normal::RandNum()
{
    return myShift + myScale * myDist(myEngine);
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

IDistribution* Normal::Clone() const { return new Normal(*this); }

std::string Normal::Name() const { return "Normal"; }
