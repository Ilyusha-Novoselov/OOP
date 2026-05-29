#include <MultiLayerMixture.hxx>
#include <DistributionFactory.hxx>
#include <stdexcept>
#include <numeric>
#include <cmath>

void MultiLayerMixture::Add(const GeneralDistribution& theDist, double theWeight) {
    if (theWeight < 0.0) throw std::invalid_argument("Weight must be non-negative.");
    myComponents.push_back(theDist);
    myWeights.push_back(theWeight);
    NormalizeWeights();
}

void MultiLayerMixture::Remove(size_t theIndex) {
    if (theIndex >= myComponents.size()) throw std::out_of_range("Index out of range.");
    myComponents.erase(myComponents.begin() + theIndex);
    myWeights.erase(myWeights.begin() + theIndex);
    if (!myComponents.empty()) NormalizeWeights();
}

GeneralDistribution& MultiLayerMixture::Component(size_t theIndex) { return myComponents.at(theIndex); }
const GeneralDistribution& MultiLayerMixture::Component(size_t theIndex) const { return myComponents.at(theIndex); }
size_t MultiLayerMixture::Size() const { return myComponents.size(); }
double MultiLayerMixture::GetWeight(size_t theIndex) const { return myWeights.at(theIndex); }

void MultiLayerMixture::NormalizeWeights() {
    double aSum = std::accumulate(myWeights.begin(), myWeights.end(), 0.0);
    if (aSum <= 0.0) throw std::invalid_argument("Sum of weights must be positive.");
    for (double& w : myWeights) w /= aSum;
    RebuildDiscreteDistribution();
}

void MultiLayerMixture::RebuildDiscreteDistribution() {
    myDiscreteDist = std::discrete_distribution<int>(myWeights.begin(), myWeights.end());
}

double MultiLayerMixture::Density(double theX) const {
    double aRes = 0.0;
    for (size_t i = 0; i < myComponents.size(); i++) {
        aRes += myWeights[i] * myComponents[i].Density(theX);
    }
    return aRes;
}

double MultiLayerMixture::ExpectedValue() const {
    double aRes = 0.0;
    for (size_t i = 0; i < myComponents.size(); i++) {
        aRes += myWeights[i] * myComponents[i].ExpectedValue();
    }
    return aRes;
}

double MultiLayerMixture::Variance() const {
    double aMean = ExpectedValue();
    double aRes = 0.0;
    for (size_t i = 0; i < myComponents.size(); i++) {
        double m = myComponents[i].ExpectedValue();
        double v = myComponents[i].Variance();
        aRes += myWeights[i] * (v + (m - aMean) * (m - aMean));
    }
    return aRes;
}

double MultiLayerMixture::Asymmetry() const {
    double aMean = ExpectedValue();
    double aVar = Variance();
    if (aVar < 1e-9) return 0.0;
    
    double m3 = 0.0;
    for (size_t i = 0; i < myComponents.size(); i++) {
        double m = myComponents[i].ExpectedValue();
        double v = myComponents[i].Variance();
        double s = myComponents[i].Asymmetry();
        double aComp3rd = s * std::pow(v, 1.5);
        double d = m - aMean;
        m3 += myWeights[i] * (aComp3rd + 3.0 * d * v + d * d * d);
    }
    return m3 / std::pow(aVar, 1.5);
}

double MultiLayerMixture::Kurtosis() const {
    double aMean = ExpectedValue();
    double aVar = Variance();
    if (aVar < 1e-9) return 0.0;
    
    double m4 = 0.0;
    for (size_t i = 0; i < myComponents.size(); i++) {
        double m = myComponents[i].ExpectedValue();
        double v = myComponents[i].Variance();
        double s = myComponents[i].Asymmetry();
        double k = myComponents[i].Kurtosis();
        
        double aComp4th = (k + 3.0) * v * v;
        double aComp3rd = s * std::pow(v, 1.5);
        double d = m - aMean;
        
        m4 += myWeights[i] * (aComp4th + 4.0 * d * aComp3rd + 6.0 * d * d * v + d * d * d * d);
    }
    return m4 / (aVar * aVar) - 3.0;
}

double MultiLayerMixture::RandNum() {
    if (myComponents.empty()) throw std::runtime_error("Mixture is empty");
    int anIndex = myDiscreteDist(myEngine);
    return myComponents[anIndex].RandNum();
}

IDistribution* MultiLayerMixture::Clone() const { return new MultiLayerMixture(*this); }

std::string MultiLayerMixture::Name() const { return "MultiLayerMixture"; }

void MultiLayerMixture::Save(std::ostream& theOut) const {
    theOut << myComponents.size() << "\n";
    for (size_t i = 0; i < myComponents.size(); ++i) {
        theOut << myWeights[i] << "\n";
        myComponents[i].Save(theOut);
    }
}

void MultiLayerMixture::Load(std::istream& theIn) {
    size_t aCount;
    theIn >> aCount;
    myComponents.clear();
    myWeights.clear();
    
    for (size_t i = 0; i < aCount; ++i) {
        double w;
        theIn >> w;
        GeneralDistribution aDist(theIn);
        myComponents.push_back(aDist);
        myWeights.push_back(w);
    }
    if (!myComponents.empty()) NormalizeWeights();
}

namespace {
    IDistribution* CreateMixture() { return new MultiLayerMixture(); }
    const bool anIsRegisteredMix = DistributionFactory::Instance().RegisterDistribution("MultiLayerMixture", CreateMixture);
}