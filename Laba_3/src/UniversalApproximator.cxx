#include <UniversalApproximator.hxx>
#include <Normal.hxx>
#include <Uniform.hxx>
#include <MixtureEM.h>

#include <iostream>
#include <cmath>

/* Интерфейсная функция для доступа к элементам Empiric из Си-модуля.
   Убран reinterpret_cast: указатель на Empiric передаётся как есть
   через опакованный тип struct Empiric* в заголовке MixtureEM.h,
   а здесь мы принимаем его обратно корректным static_cast. */
extern "C" double empiricData(struct Empiric* e, int i)
{
    return static_cast<Empiric*>(static_cast<void*>(e))->GetData(i);
}

UniversalApproximator::UniversalApproximator(Empiric& theEmpiric, bool theIsRobust)
    : myData(theEmpiric), myIsRobust(theIsRobust), myAIC(0), myBIC(0), myICL(0), myLL(0), myCompCount(0)
{
    myData.Attach(this, 1);
    Approximate();
}

UniversalApproximator::~UniversalApproximator() {
    myData.Detach(this, 1);
}

void UniversalApproximator::Update() {
    Approximate();
}

void UniversalApproximator::Approximate() {
    if (myData.Size() == 0) return;

    EMResult aRes = { 0 };
    MultiLayerMixture aNewModel;

    if (!myIsRobust) {
        if (mixtureBuilder(static_cast<struct Empiric*>(static_cast<void*>(&myData)),
            static_cast<int>(myData.Size()), 1, 5, 200, &aRes))
        {
            myAIC = aRes.aic;
            myBIC = aRes.bic;
            myICL = aRes.icl;
            myLL = aRes.log_likelihood;
            myCompCount = aRes.components_count;

            for (int i = 0; i < aRes.components_count; i++) {
                if (std::isnan(aRes.weights[i]) || aRes.weights[i] <= 0.0) continue;

                double m = std::isnan(aRes.means[i]) ? 0.0 : aRes.means[i];
                double s = (std::isnan(aRes.sigmas[i]) || aRes.sigmas[i] <= 0.01) ? 0.01 : aRes.sigmas[i];

                Normal aNorm(m, s);
                GeneralDistribution aGen(aNorm);
                aNewModel.Add(aGen, aRes.weights[i]);
            }
        }
    }
    else {
        double aUnifW, aUnifMin, aUnifMax;
        if (robustMixtureBuilder(static_cast<struct Empiric*>(static_cast<void*>(&myData)),
            static_cast<int>(myData.Size()), 1, 5, 200,
            &aRes, &aUnifW, &aUnifMin, &aUnifMax))
        {
            myAIC = aRes.aic;
            myBIC = aRes.bic;
            myICL = aRes.icl;
            myLL = aRes.log_likelihood;
            myCompCount = aRes.components_count;

            MultiLayerMixture aCleanMix;
            double valid_weight_sum = 0.0;

            for (int i = 0; i < aRes.components_count; i++) {
                if (std::isnan(aRes.weights[i]) || aRes.weights[i] <= 0.0) continue;

                double m = std::isnan(aRes.means[i]) ? 0.0 : aRes.means[i];
                double s = (std::isnan(aRes.sigmas[i]) || aRes.sigmas[i] <= 0.01) ? 0.01 : aRes.sigmas[i];

                Normal aNorm(m, s);
                GeneralDistribution aGen(aNorm);
                aCleanMix.Add(aGen, aRes.weights[i]);
                valid_weight_sum += aRes.weights[i];
            }

            if (valid_weight_sum > 0.0) {
                double aCenter = (aUnifMin + aUnifMax) / 2.0;
                double aScale = (aUnifMax - aUnifMin) / 2.0;
                if (aScale < 1e-9) aScale = 1.0;

                Uniform aUnifDist(aCenter, aScale);

                GeneralDistribution aGenClean(aCleanMix);
                GeneralDistribution aGenUnif(aUnifDist);

                aNewModel.Add(aGenClean, 1.0 - aUnifW);
                aNewModel.Add(aGenUnif, aUnifW);
            }
        }
    }

    myModel = aNewModel;
}