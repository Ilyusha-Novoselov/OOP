#include <Iterators.hxx>

ShallowMixtureIterator::ShallowMixtureIterator(MultiLayerMixture& theMix) : myMix(&theMix), myIter(0) {}

void ShallowMixtureIterator::First() { myIter = 0; }
void ShallowMixtureIterator::Next() { myIter++; }
bool ShallowMixtureIterator::IsDone() const { return myIter >= myMix->Size(); }

std::pair<GeneralDistribution&, double> ShallowMixtureIterator::CurrentItem() {
    return { myMix->Component(myIter), myMix->GetWeight(myIter) };
}


DeepMixtureTraverser::DeepMixtureTraverser(MultiLayerMixture& theMix, const std::function<bool(GeneralDistribution&, double)>& theFunc) 
    : myRoot(theMix), myProcessItem(theFunc), myResult(true) {}

DeepMixtureTraverser::DeepMixtureTraverser(MultiLayerMixture& theMix) 
    : myRoot(theMix), myProcessItem(nullptr), myResult(true) {}

bool DeepMixtureTraverser::Traverse(const std::function<bool(GeneralDistribution&, double)>& theFunc) {
    myProcessItem = theFunc;
    return Traverse();
}

bool DeepMixtureTraverser::Traverse() {
    if (myProcessItem) {
        myResult = true;
        Execute(ShallowMixtureIterator(myRoot), 1.0);
    } else {
        throw std::runtime_error("Process item function is not set.");
    }
    return myResult;
}

void DeepMixtureTraverser::Execute(ShallowMixtureIterator theIter, double theProb) {
    for (theIter.First(); !theIter.IsDone(); theIter.Next()) {
        auto aCurr = theIter.CurrentItem();
        
        // Согласно методичке, проверяем через typeid на письмо
        if (typeid(aCurr.first.Component()) == typeid(MultiLayerMixture)) {
            MultiLayerMixture& aNestedMix = dynamic_cast<MultiLayerMixture&>(aCurr.first.Component());
            Execute(ShallowMixtureIterator(aNestedMix), theProb * aCurr.second);
        } else {
            myResult = myProcessItem(aCurr.first, theProb * aCurr.second);
            if (!myResult) break;
        }
    }
}