#include "Histogram.hxx"

#include <cmath>

Histogram::Histogram(Empiric& theEmpiric, int theBins, int theInterest, bool theFixed)
    : myRef(theEmpiric), myK(theBins), myInterest(theInterest), myIsFixedBounds(theFixed)
{
    myRef.Attach(this, myInterest);
    Update();
}

Histogram::~Histogram()
{
    myRef.Detach(this, myInterest);
}

void Histogram::Update()
{
    if (myRef.Size() == 0) return;

    double aMinVal = myRef.Min();
    double aMaxVal = myRef.Max();

    if (myIsFixedBounds && !myBounds.empty()) {
        aMinVal = myBounds.front();
        aMaxVal = myBounds.back();
    }
    else {
        if (std::abs(aMaxVal - aMinVal) < 1e-9) aMaxVal = aMinVal + 1e-9;
        double aStep = (aMaxVal - aMinVal) / myK;
        myBounds.resize(myK + 1);
        for (int i = 0; i <= myK; ++i) {
            myBounds[i] = aMinVal + i * aStep;
        }
    }

    double aStep = (aMaxVal - aMinVal) / myK;
    myEmpDensity.assign(myK, 0.0);
    const auto& aData = myRef.GetRawData();

    for (double x : aData) {
        int aBin = std::floor((x - aMinVal) / aStep);
        if (aBin < 0) aBin = 0;
        if (aBin >= myK) aBin = myK - 1;
        myEmpDensity[aBin]++;
    }

    for (int i = 0; i < myK; ++i) {
        myEmpDensity[i] /= (aData.size() * aStep);
    }
}

int Histogram::GetBinsCount() const { return myK; }

double Histogram::GetBound(int theIndex) const { return myBounds[theIndex]; }

double Histogram::GetDensity(int theIndex) const { return myEmpDensity[theIndex]; }

double Histogram::GetDensityByValue(double theX) const
{
    if (myK == 0 || myBounds.empty()) return 0.0;

    // Если точка выходит за границы гистограммы, плотность 0
    if (theX < myBounds.front() || theX > myBounds.back()) return 0.0;

    // Ищем, в какой интервал попадает X
    for (int i = 0; i < myK; ++i) {
        if (theX >= myBounds[i] && theX <= myBounds[i + 1]) {
            return myEmpDensity[i];
        }
    }
    return 0.0;
}
