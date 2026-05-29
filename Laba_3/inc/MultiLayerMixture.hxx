#ifndef _Laba3_MultiLayerMixture_Header
#define _Laba3_MultiLayerMixture_Header

#include <IDistribution.hxx>
#include <IPersistent.hxx>
#include <GeneralDistribution.hxx>

#include <vector>
#include <utility>
#include <random>


class MultiLayerMixture : public IDistribution, public IPersistent
{
public:
    MultiLayerMixture() = default;
    
    void Add(const GeneralDistribution& theDist, double theWeight);
    void Remove(size_t theIndex);
    GeneralDistribution& Component(size_t theIndex);
    const GeneralDistribution& Component(size_t theIndex) const;
    
    size_t Size() const;
    double GetWeight(size_t theIndex) const;
    
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

private:
    void NormalizeWeights();
    void RebuildDiscreteDistribution();

    std::vector<GeneralDistribution> myComponents;
    std::vector<double> myWeights;
    std::discrete_distribution<int> myDiscreteDist;
};

#endif