#include <UniversalApproximator.hxx>
#include <Normal.hxx>
#include <Uniform.hxx>
#include <MixtureEM.h>

#include <iostream>

extern "C" double empiricData(struct Empiric* e, int i) {
    return reinterpret_cast<Empiric*>(e)->GetData(i);
}

UniversalApproximator::UniversalApproximator(Empiric& theEmpiric, bool theIsRobust)
    : myData(theEmpiric), myIsRobust(theIsRobust), myAIC(0), myBIC(0), myLL(0), myCompCount(0)
{
    myData.Attach(this, 1);
}

UniversalApproximator::~UniversalApproximator() {
    myData.Detach(this, 1);
}

void UniversalApproximator::Update() {
    Approximate();
}

void UniversalApproximator::Approximate() {
    if (myData.Size() == 0) return;

    EMResult aRes;
    MultiLayerMixture aNewModel;

    if (!myIsRobust) {
        if (mixtureBuilder(reinterpret_cast<struct Empiric*>(&myData), myData.Size(), 1, 5, 200, &aRes)) {
            myAIC = aRes.aic;
            myBIC = aRes.bic;
            myLL = aRes.log_likelihood;
            myCompCount = aRes.components_count;

            for (int i = 0; i < aRes.components_count; i++) {
                Normal aNorm(aRes.means[i], aRes.sigmas[i]);
                GeneralDistribution aGen(aNorm);
                aNewModel.Add(aGen, aRes.weights[i]);
            }
        }
    }
    else {
        double aUnifW, aUnifMin, aUnifMax;
        if (robustMixtureBuilder(reinterpret_cast<struct Empiric*>(&myData), myData.Size(), 1, 5, 200, &aRes, &aUnifW, &aUnifMin, &aUnifMax)) {
            myAIC = aRes.aic;
            myBIC = aRes.bic;
            myLL = aRes.log_likelihood;
            myCompCount = aRes.components_count;

            MultiLayerMixture aCleanMix;
            for (int i = 0; i < aRes.components_count; i++) {
                Normal aNorm(aRes.means[i], aRes.sigmas[i]);
                GeneralDistribution aGen(aNorm);
                aCleanMix.Add(aGen, aRes.weights[i]);
            }

            Uniform aUnifDist(aUnifMin, aUnifMax - aUnifMin);

            GeneralDistribution aGenClean(aCleanMix);
            GeneralDistribution aGenUnif(aUnifDist);

            aNewModel.Add(aGenClean, 1.0 - aUnifW);
            aNewModel.Add(aGenUnif, aUnifW);
        }
    }

    myModel = aNewModel;
}