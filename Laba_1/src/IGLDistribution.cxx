#include "IGLDistribution.hxx"

#include <cmath>
#include <string>
#include <numbers>


double IGLDistribution::Density(double theX) const
{
    double aX = (theX - myShift) / myScale;
    double z = std::sqrt(myShape * myShape + 2.0 * myShape * std::abs(aX));

    // Используем Boost, как указано в таблице методички
    double aF = (myShape / std::numbers::pi) * std::exp(myShape) * std::cyl_bessel_k(0, z);
    return aF / myScale;
}

double IGLDistribution::ExpectedValue() const { return myShift; }

double IGLDistribution::Variance() const
{
    // Точная формула дисперсии для IG-L
    return 2.0 * (1.0 + 1.0 / myShape) * myScale * myScale;
}

double IGLDistribution::Asymmetry() const { return 0.0; }

double IGLDistribution::Kurtosis() const
{
    // Точная формула эксцесса для IG-L выведенная через моменты
    double aNumerator = 6.0 * (1.0 + 6.0 / myShape + 15.0 / (myShape * myShape) + 15.0 / (myShape * myShape * myShape));
    double aDenominator = std::pow(1.0 + 1.0 / myShape, 2);
    return (aNumerator / aDenominator) - 3.0;
}

double IGLDistribution::RandNum()
{
    std::normal_distribution<> aNorm(0.0, 1.0);
    double v = aNorm(myEngine);
    double y = v * v;

    double x = 1.0 + y / (2.0 * myShape) - std::sqrt(4.0 * myShape * y + y * y) / (2.0 * myShape);

    std::uniform_real_distribution<> anUnif(0.0, 1.0);
    double u = anUnif(myEngine);
    double eta = (u <= 1.0 / (1.0 + x)) ? x : (1.0 / x);

    std::exponential_distribution<> anExpd(1.0);
    std::bernoulli_distribution aBern(0.5);
    double xi_L = anExpd(myEngine) * (aBern(myEngine) ? 1.0 : -1.0);

    double aTarget = eta * xi_L;
    return myShift + myScale * aTarget;
}

void IGLDistribution::Save(std::ostream& theOut) const
{
    theOut << "IG_L " << myShift << " " << myScale << " " << myShape << "\n";
}

void IGLDistribution::Load(std::istream& theIn)
{
    std::string aName;
    theIn >> aName >> myShift >> myScale >> myShape;
}