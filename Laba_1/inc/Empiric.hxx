#ifndef _Laba1_Empiric_Header
#define _Laba1_Empiric_Header

#include <vector>
#include <forward_list>
#include <utility>


class IObserver;

class Empiric
{
public:
    void Attach(IObserver* theObserver, int theInterest);
    void Detach(IObserver* theObserver, int theInterest);
    void Notify(int theInterest = 1);

    void AddData(double theX);
    double GetData(size_t theIndex) const;
    size_t Size() const;
    const std::vector<double>& GetRawData() const;

    double Min() const;
    double Max() const;

    double Mean() const;
    double Variance() const;

    double Asymmetry() const;
    double Kurtosis() const;

private:
    std::vector<double> myData;
    std::forward_list<std::pair<IObserver*, int>> myObservers;
};

#endif
