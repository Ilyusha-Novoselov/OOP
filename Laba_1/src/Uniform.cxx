#include <Uniform.hxx>

#include <string>


double Uniform::Density(double theX) const
{
    double aX = (theX - myShift) / myScale;

    if (aX >= -1.0 && aX <= 1.0) {
        return 1.0 / (2.0 * myScale);
    }

    return 0.0;
}

double Uniform::ExpectedValue() const { return myShift; }

double Uniform::Variance() const { return (myScale * myScale) / 3.0; }

double Uniform::Asymmetry() const { return 0.0; }

double Uniform::Kurtosis() const { return -1.2; }

double Uniform::RandNum()
{
    return myShift + myScale * myDist(myEngine);
}

void Uniform::Save(std::ostream& theOut) const
{
    theOut << "Uniform " << myShift << " " << myScale << "\n";
}

void Uniform::Load(std::istream& theIn)
{
    std::string aName;

    theIn >> aName >> myShift >> myScale;
}
