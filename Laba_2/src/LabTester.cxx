#include <LabTester.hxx>
#include <Normal.hxx>
#include <Uniform.hxx>
#include <IGLDistribution.hxx>
#include <DistributionFactory.hxx>
#include <GeneralDistribution.hxx>
#include <iostream>
#include <fstream>
#include <vector>

void LabTester::RunAllTests()
{
    std::setlocale(LC_ALL, "");
    TestVirtualConstructors();
    TestFactoryPattern();
    TestEnvelopeIdiom();
    TestPolymorphicPersistence();
}

void LabTester::TestVirtualConstructors()
{
    std::cout << "\n=== Тест 1: Виртуальные конструкторы (Clone и Name) ===\n";
    Normal aNorm(5.0, 2.0);
    IDistribution* aClonedNorm = aNorm.Clone();

    std::cout << "Оригинал Normal: name = " << aNorm.Name() << ", E(X)=" << aNorm.ExpectedValue() << "\n";
    std::cout << "Клон Normal:     name = " << aClonedNorm->Name() << ", E(X)=" << aClonedNorm->ExpectedValue() << "\n";

    delete aClonedNorm;
}

void LabTester::TestFactoryPattern()
{
    std::cout << "\n=== Тест 2: Паттерн Фабрика (Factory Method) ===\n";
    auto& aFactory = DistributionFactory::Instance();

    IDistribution* aD1 = aFactory.CreateDistribution("Normal");
    IDistribution* aD2 = aFactory.CreateDistribution("Uniform");
    IDistribution* aD3 = aFactory.CreateDistribution("IG_L");

    std::cout << "Создано через фабрику: " << aD1->Name() << "\n";
    std::cout << "Создано через фабрику: " << aD2->Name() << "\n";
    std::cout << "Создано через фабрику: " << aD3->Name() << "\n";

    delete aD1; delete aD2; delete aD3;
}

void LabTester::TestEnvelopeIdiom()
{
    std::cout << "\n=== Тест 3: Идиома Конверт/Письмо (GeneralDistribution) ===\n";
    Normal aNorm(10.0, 3.0);
    GeneralDistribution aGen1(aNorm);

    std::cout << "GeneralDistribution оборачивает " << aGen1.Component().Name()
        << ", E(X)=" << aGen1.ExpectedValue() << "\n";

    // Проверка эффекта матрешки
    GeneralDistribution aGen2(aGen1);
    std::cout << "GeneralDistribution оборачивает GeneralDistribution. Внутренний тип: "
        << aGen2.Component().Name() << " (если не GeneralDistribution, то матрешки нет)\n";
}

void LabTester::TestPolymorphicPersistence()
{
    std::cout << "\n=== Тест 4: Полиморфная персистентность ===\n";

    std::vector<GeneralDistribution> aSaveDists;
    aSaveDists.push_back(GeneralDistribution(Normal(10.0, 3.0)));
    aSaveDists.push_back(GeneralDistribution(Uniform(5.0, 2.0)));

    std::ofstream anOut("test_distributions.txt");
    if (anOut.is_open()) {
        for (const auto& aDist : aSaveDists) {
            aDist.Save(anOut);
        }
        anOut.close();
        std::cout << "Сохранено 2 объекта в файл.\n";
    }

    std::vector<GeneralDistribution> aLoadDists;
    std::ifstream anIn("test_distributions.txt");
    if (anIn.is_open()) {
        while (anIn.peek() != EOF) {
            std::string aCheck;
            anIn >> aCheck;
            if (aCheck.empty()) break;

            anIn.seekg(-static_cast<int>(aCheck.length()), std::ios_base::cur);
            aLoadDists.push_back(GeneralDistribution(anIn));
        }
        anIn.close();
    }

    for (size_t i = 0; i < aLoadDists.size(); ++i) {
        std::cout << "Загружен объект " << i + 1 << ": name = " << aLoadDists[i].Component().Name()
            << ", E(X)=" << aLoadDists[i].ExpectedValue() << "\n";
    }
}