#include <GeneralDistribution.hxx>
#include <DistributionFactory.hxx>

#include <typeinfo>


GeneralDistribution::GeneralDistribution(std::istream & theIn) : myLetter(nullptr)
{
    Load(theIn);
}

GeneralDistribution::GeneralDistribution(const GeneralDistribution& theOther)
{
    myLetter = theOther.myLetter->Clone();
}

GeneralDistribution::GeneralDistribution(const IDistribution& theOther)
{
    // Защита от эффекта матрешки
    if (typeid(theOther) == typeid(const GeneralDistribution)) {
        myLetter = dynamic_cast<const GeneralDistribution&>(theOther).myLetter->Clone();
    }
    else {
        myLetter = theOther.Clone();
    }
}

GeneralDistribution::~GeneralDistribution()
{
    delete myLetter;
}

GeneralDistribution& GeneralDistribution::operator=(const GeneralDistribution& theOther)
{
    if (this != &theOther) {
        delete myLetter;
        myLetter = theOther.myLetter->Clone();
    }
    return *this;
}

double GeneralDistribution::Density(double theX) const { return myLetter->Density(theX); }
double GeneralDistribution::ExpectedValue() const { return myLetter->ExpectedValue(); }
double GeneralDistribution::Variance() const { return myLetter->Variance(); }
double GeneralDistribution::Asymmetry() const { return myLetter->Asymmetry(); }
double GeneralDistribution::Kurtosis() const { return myLetter->Kurtosis(); }
double GeneralDistribution::RandNum() { return myLetter->RandNum(); }

IDistribution* GeneralDistribution::Clone() const
{
    return new GeneralDistribution(*this);
}

std::string GeneralDistribution::Name() const
{
    return myLetter->Name();
}

void GeneralDistribution::Save(std::ostream& theOut) const
{
    theOut << myLetter->Name() << "\n";
    dynamic_cast<IPersistent*>(myLetter)->Save(theOut);
}

void GeneralDistribution::Load(std::istream& theIn)
{
    delete myLetter;
    myLetter = nullptr;
    std::string aName;
    theIn >> aName;
    myLetter = DistributionFactory::Instance().CreateDistribution(aName);
    dynamic_cast<IPersistent*>(myLetter)->Load(theIn);
}

IDistribution& GeneralDistribution::Component() { return *myLetter; }
const IDistribution& GeneralDistribution::Component() const { return *myLetter; }
