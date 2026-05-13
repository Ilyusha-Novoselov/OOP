#include <DistributionFactory.hxx>

#include <stdexcept>


DistributionFactory & DistributionFactory::Instance()
{
    static DistributionFactory anInstance;
    return anInstance;
}

bool DistributionFactory::RegisterDistribution(const std::string& theName, CreatorCallback theCreator)
{
    return myCallbacks.insert({ theName, theCreator }).second;
}

bool DistributionFactory::UnregisterDistribution(const std::string& theName)
{
    return myCallbacks.erase(theName) > 0;
}

IDistribution* DistributionFactory::CreateDistribution(const std::string& theName)
{
    auto anIter = myCallbacks.find(theName);
    if (anIter == myCallbacks.end()) {
        throw std::runtime_error("Unknown distribution type: " + theName);
    }
    return anIter->second();
}