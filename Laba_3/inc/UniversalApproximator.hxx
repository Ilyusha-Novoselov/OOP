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

    /* Критерии качества модели (п. 2.2.1 методички: BIC, AIC, ICL) */
    double GetAIC() const { return myAIC; }
    double GetBIC() const { return myBIC; }
    double GetICL() const { return myICL; }
    double GetLogLikelihood() const { return myLL; }
    int    GetComponentCount() const { return myCompCount; }

private:
    Empiric& myData;
    MultiLayerMixture myModel;
    bool              myIsRobust;

    double myAIC;
    double myBIC;
    double myICL;
    double myLL;
    int    myCompCount;
};

#endif