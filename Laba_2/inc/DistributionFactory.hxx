#ifndef _Laba2_DistributionFactory_Header
#define _Laba2_DistributionFactory_Header

#include <IDistribution.hxx>

#include <unordered_map>
#include <string>
#include <functional>

// ПУНКТ 2.2: Фабрика-синглтон для создания объектов
class DistributionFactory
{
public:
    using Creator = std::function<IDistribution*()>;

    static DistributionFactory& Instance();

    void RegisterDistribution(const std::string& theName, Creator theCreator);
    IDistribution* CreateDistribution(const std::string& theName);

    DistributionFactory(const DistributionFactory&) = delete;
    DistributionFactory& operator=(const DistributionFactory&) = delete;

private:
    DistributionFactory() = default;
    ~DistributionFactory() = default;

    std::unordered_map<std::string, Creator> myCreators;
};

#endif