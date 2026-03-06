#ifndef _Laba1_Histogram_Header
#define _Laba1_Histogram_Header

#include "IObserver.hxx"
#include "Empiric.hxx"

#include <vector>

class Histogram : public IObserver
{
public:
    Histogram(Empiric& theEmpiric, int theBins, int theInterest = 0, bool theFixed = true);
    ~Histogram();

    // Запрещаем копирование, чтобы избежать повисших ссылок и двойных отписок
    Histogram(const Histogram&) = delete;
    Histogram& operator=(const Histogram&) = delete;

    void Update() override;

    int GetBinsCount() const;
    double GetBound(int theIndex) const;
    double GetDensity(int theIndex) const;

    // Метод для получения "слепка" текущих данных
    std::vector<double> GetDensities() const { return myEmpDensity; }

private:
    Empiric& myRef;
    int myK;
    int myInterest;
    std::vector<double> myBounds;
    std::vector<double> myEmpDensity;
    bool myIsFixedBounds;
};

#endif