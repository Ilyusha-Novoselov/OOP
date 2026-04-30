#include <DistributionFactory.hxx>
#include <stdexcept>

DistributionFactory& DistributionFactory::Instance()
{
    static DistributionFactory anInstance;
    return anInstance;
}

void DistributionFactory::RegisterDistribution(const std::string& theName, Creator theCreator)
{
    myCreators[theName] = theCreator;
}

IDistribution* DistributionFactory::CreateDistribution(const std::string& theName)
{
    auto anIt = myCreators.find(theName);
    if (anIt != myCreators.end()) {
        return anIt->second();
    }
    throw std::runtime_error("DistributionFactory: Unregistered class name -> " + theName);
}