#ifndef _Laba3_Iterators_Header
#define _Laba3_Iterators_Header

#include <MultiLayerMixture.hxx>
#include <functional>
#include <typeinfo>

// Поверхностный итератор
class ShallowMixtureIterator
{
public:
    ShallowMixtureIterator(MultiLayerMixture& theMix);
    void First();
    void Next();
    bool IsDone() const;
    std::pair<GeneralDistribution&, double> CurrentItem();

private:
    MultiLayerMixture* myMix;
    size_t myIter;
};

// Глубокий итератор
class DeepMixtureTraverser
{
public:
    DeepMixtureTraverser(MultiLayerMixture& theMix, const std::function<bool(GeneralDistribution&, double)>& theFunc);
    DeepMixtureTraverser(MultiLayerMixture& theMix);
    
    bool Traverse(const std::function<bool(GeneralDistribution&, double)>& theFunc);
    bool Traverse();

private:
    void Execute(ShallowMixtureIterator theIter, double theProb);

    MultiLayerMixture& myRoot;
    std::function<bool(GeneralDistribution&, double)> myProcessItem;
    bool myResult;
};

#endif