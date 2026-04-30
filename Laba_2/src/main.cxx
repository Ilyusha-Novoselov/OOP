#include <LabTester.hxx>
#include <DistributionFactory.hxx>
#include <Normal.hxx>
#include <Uniform.hxx>
#include <IGLDistribution.hxx>

int main()
{
    // ПУНКТ 2.2: Регистрация типов в фабрике-синглтоне
    DistributionFactory::Instance().RegisterDistribution("Normal", []() -> IDistribution* { return new Normal(); });
    DistributionFactory::Instance().RegisterDistribution("Uniform", []() -> IDistribution* { return new Uniform(); });
    DistributionFactory::Instance().RegisterDistribution("IG_L", []() -> IDistribution* { return new IGLDistribution(); });

    LabTester aTester;
    aTester.RunAllTests();

    return 0;
}