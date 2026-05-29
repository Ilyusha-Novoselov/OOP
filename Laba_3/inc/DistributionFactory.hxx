#ifndef _Laba3_DistributionFactory_Header
#define _Laba3_DistributionFactory_Header

#include <IDistribution.hxx>

#include <functional>
#include <string>
#include <unordered_map>


class DistributionFactory
{
public:
    using CreatorCallback = std::function<IDistribution* ()>;

    static DistributionFactory& Instance();

    // Запрет копирования и перемещения синглтона
    DistributionFactory(const DistributionFactory&) = delete;
    DistributionFactory& operator=(const DistributionFactory&) = delete;

    bool RegisterDistribution(const std::string& theName, CreatorCallback theCreator);
    bool UnregisterDistribution(const std::string& theName);
    IDistribution* CreateDistribution(const std::string& theName);

private:
    DistributionFactory() = default;
    ~DistributionFactory() = default;

    std::unordered_map<std::string, CreatorCallback> myCallbacks;
};

#endif
