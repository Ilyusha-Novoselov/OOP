#ifndef _Laba3_UniversalApproximator_Header
#define _Laba3_UniversalApproximator_Header

#include <Empiric.hxx>
#include <MultiLayerMixture.hxx>
#include <IObserver.hxx>

class UniversalApproximator : public IObserver
{
public:
    UniversalApproximator(Empiric& theEmpiric, bool theIsRobust = false);
    ~UniversalApproximator();

    void Update() override;
    void Approximate();

    MultiLayerMixture& GetModel() { return myModel; }

    // Геттеры для критериев качества модели
    double GetAIC() const { return myAIC; }
    double GetBIC() const { return myBIC; }
    double GetLogLikelihood() const { return myLL; }
    int GetComponentCount() const { return myCompCount; }

private:
    Empiric& myData;
    MultiLayerMixture myModel;
    bool myIsRobust;

    double myAIC;
    double myBIC;
    double myLL;
    int myCompCount;
};

#endif