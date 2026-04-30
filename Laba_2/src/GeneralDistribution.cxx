#include <GeneralDistribution.hxx>
#include <DistributionFactory.hxx>
#include <typeinfo>

GeneralDistribution::GeneralDistribution(std::istream& theIn) : myLetter(nullptr)
{
    Load(theIn);
}

GeneralDistribution::GeneralDistribution(const GeneralDistribution& theD0)
{
    myLetter = theD0.myLetter->Clone();
}

GeneralDistribution::GeneralDistribution(const IDistribution& theD0)
{
    // Защита от эффекта "матрешки" по методическим указаниям
    if (typeid(theD0) == typeid(const GeneralDistribution)) {
        myLetter = dynamic_cast<const GeneralDistribution&>(theD0).myLetter->Clone();
    } else {
        myLetter = theD0.Clone();
    }
}

GeneralDistribution::~GeneralDistribution()
{
    delete myLetter;
}

GeneralDistribution& GeneralDistribution::operator=(const GeneralDistribution& theD0)
{
    if (this != &theD0) {
        delete myLetter;
        myLetter = theD0.myLetter->Clone();
    }
    return *this;
}

IDistribution* GeneralDistribution::Clone() const
{
    return new GeneralDistribution(*this);
}

std::string GeneralDistribution::Name() const
{
    return "GeneralDistribution";
}

double GeneralDistribution::Density(double theX) const { return myLetter->Density(theX); }
double GeneralDistribution::ExpectedValue() const { return myLetter->ExpectedValue(); }
double GeneralDistribution::Variance() const { return myLetter->Variance(); }
double GeneralDistribution::Asymmetry() const { return myLetter->Asymmetry(); }
double GeneralDistribution::Kurtosis() const { return myLetter->Kurtosis(); }
double GeneralDistribution::RandNum() { return myLetter->RandNum(); }

void GeneralDistribution::Save(std::ostream& theOut) const
{
    theOut << myLetter->Name() << "\n";
    if (auto* aPersist = dynamic_cast<IPersistent*>(myLetter)) {
        aPersist->Save(theOut);
    }
}

void GeneralDistribution::Load(std::istream& theIn)
{
    delete myLetter;
    std::string aName;
    theIn >> aName;
    
    myLetter = DistributionFactory::Instance().CreateDistribution(aName);
    
    if (auto* aPersist = dynamic_cast<IPersistent*>(myLetter)) {
        aPersist->Load(theIn);
    }
}

IDistribution& GeneralDistribution::Component()
{
    return *myLetter;
}