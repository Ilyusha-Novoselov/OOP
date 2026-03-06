#include <Empiric.hxx>

#include <IObserver.hxx>

#include <algorithm>
#include <numeric>


void Empiric::Attach(IObserver* theObserver, int theInterest)
{
    myObservers.push_front(std::make_pair(theObserver, theInterest));
}

void Empiric::Detach(IObserver* theObserver, int theInterest)
{
    myObservers.remove_if([&](const std::pair<IObserver*, int>& theP) {
        return theP.first == theObserver && theP.second == theInterest;
    });
}

void Empiric::Notify(int theInterest)
{
    for (const auto& anElem : myObservers) {
        if (anElem.second == theInterest) {
            anElem.first->Update();
        }
    }
}

void Empiric::AddData(double theX)
{
    myData.emplace_back(theX);
    Notify(0); // Событие "0" - автоматическое обновление
}

double Empiric::GetData(size_t theIndex) const { return myData.at(theIndex); }

size_t Empiric::Size() const { return myData.size(); }

const std::vector<double>& Empiric::GetRawData() const { return myData; }

double Empiric::Min() const
{
    if (myData.empty()) {
        return 0.0;
    }

    return *std::min_element(myData.begin(), myData.end());
}

double Empiric::Max() const
{
    if (myData.empty()) {
        return 0.0;
    }

    return *std::max_element(myData.begin(), myData.end());
}

double Empiric::Mean() const
{
    if (myData.empty()) {
        return 0.0;
    }

    double aSum = std::accumulate(myData.begin(), myData.end(), 0.0);
    return aSum / myData.size();
}

double Empiric::Variance() const
{
    if (myData.size() < 2) {
        return 0.0;
    }

    double aMean = Mean();
    double aSum = 0.0;
    for (double x : myData) {
        aSum += (x - aMean) * (x - aMean);
    }

    return aSum / myData.size();
}

double Empiric::Asymmetry() const
{
    if (myData.size() < 3) {
        return 0.0;
    }

    double aMean = Mean();
    double aVar = Variance();

    // Защита от деления на ноль, если все числа одинаковые
    if (std::abs(aVar) < 1e-9) return 0.0;

    double aSum = 0.0;
    for (double x : myData) {
        aSum += pow(x - aMean, 3.0);
    }

    double m3 = aSum / myData.size(); // Центральный момент 3-го порядка

    // Формула: m3 / (sigma^3)
    return m3 / pow(sqrt(aVar), 3.0);
}

double Empiric::Kurtosis() const
{
    if (myData.size() < 4) {
        return 0.0;
    }

    double aMean = Mean();
    double aVar = Variance();

    if (std::abs(aVar) < 1e-9) return 0.0;

    double aSum = 0.0;
    for (double x : myData) {
        aSum += pow(x - aMean, 4.0);
    }

    double m4 = aSum / myData.size(); // Центральный момент 4-го порядка

    // Формула: m4 / (sigma^4) - 3 (минус 3 нужен для эксцесса)
    return (m4 / (aVar * aVar)) - 3.0;
}
