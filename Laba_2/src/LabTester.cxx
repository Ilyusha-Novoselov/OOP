#include <LabTester.hxx>
#include <Normal.hxx>
#include <Uniform.hxx>
#include <IGLDistribution.hxx>
#include <DistributionFactory.hxx>
#include <GeneralDistribution.hxx>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>

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

    std::vector<IDistribution*> anOrigs;
    anOrigs.push_back(new Normal(5.0, 2.0));
    anOrigs.push_back(new Uniform(1.0, 10.0));
    anOrigs.push_back(new IGLDistribution(0.0, 1.0, 2.0));

    for (auto* anOrig : anOrigs) {
        IDistribution* aClone = anOrig->Clone();
        std::cout << "Оригинал: name = " << anOrig->Name() << "\t| E(X) = " << anOrig->ExpectedValue() << "\n";
        std::cout << "Клон:     name = " << aClone->Name() << "\t| E(X) = " << aClone->ExpectedValue() << "\n";
        std::cout << "---------------------------------------------------\n";

        delete aClone;
        delete anOrig;
    }
}

void LabTester::TestFactoryPattern()
{
    std::cout << "\n=== Тест 2: Паттерн Фабрика (Factory Method) ===\n";
    auto& aFactory = DistributionFactory::Instance();

    std::vector<std::string> aNames = { "Normal", "Uniform", "IG_L" };
    for (const auto& aName : aNames) {
        IDistribution* aDist = aFactory.CreateDistribution(aName);
        std::cout << "Успешно создано через фабрику: " << aDist->Name() << "\n";
        delete aDist;
    }

    // Проверка на несуществующий класс (защита от "дурака")
    std::cout << "Проверка обработки ошибок:\n";
    try {
        IDistribution* aFail = aFactory.CreateDistribution("UnknownDistribution");
    }
    catch (const std::exception& e) {
        std::cout << " -> Ожидаемое исключение поймано: " << e.what() << "\n";
    }
}

void LabTester::TestEnvelopeIdiom()
{
    std::cout << "\n=== Тест 3: Идиома Конверт/Письмо (GeneralDistribution) ===\n";

    Normal aNorm(10.0, 3.0);
    GeneralDistribution aGen1(aNorm);
    std::cout << "1. Базовая обертка создана. Внутренний класс: " << aGen1.Component().Name() << "\n";

    // Проверка конструктора копирования
    GeneralDistribution aGen2(aGen1);
    std::cout << "2. Копия обертки создана. Внутренний класс: " << aGen2.Component().Name() << "\n";

    // Проверка оператора присваивания
    GeneralDistribution aGen3(Uniform(0.0, 1.0));
    aGen3 = aGen1;
    std::cout << "3. После присваивания обертка содержит: " << aGen3.Component().Name() << "\n";

    // Проверка защиты от эффекта матрешки
    GeneralDistribution aMatryoshka(aGen1);
    std::cout << "4. Проверка 'матрешки' (передача конверта в конверт).\n"
        << "   Внутренний тип: " << aMatryoshka.Component().Name()
        << " (Если Normal, то защиты сработала верно, матрешки нет!)\n";
}

void LabTester::TestPolymorphicPersistence()
{
    std::cout << "\n=== Тест 4: Полиморфная персистентность ===\n";

    std::vector<GeneralDistribution> aSaveDists;
    aSaveDists.push_back(GeneralDistribution(Normal(10.0, 3.0)));
    aSaveDists.push_back(GeneralDistribution(Uniform(5.0, 2.0)));
    aSaveDists.push_back(GeneralDistribution(IGLDistribution(0.0, 1.0, 2.0)));

    std::cout << "--- Сохранение объектов ---\n";
    std::ofstream anOut("test_distributions.txt");
    if (anOut.is_open()) {
        for (const auto& aDist : aSaveDists) {
            std::cout << "Сохранение: " << aDist.Component().Name()
                << "\t| E(X) = " << aDist.ExpectedValue()
                << "\t| Var = " << aDist.Variance() << "\n";
            aDist.Save(anOut);
        }
        anOut.close();
        std::cout << "Данные успешно записаны в test_distributions.txt.\n";
    }

    std::cout << "\n--- Восстановление объектов через фабрику ---\n";
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
        std::cout << "Загрузка:   " << aLoadDists[i].Component().Name()
            << "\t| E(X) = " << aLoadDists[i].ExpectedValue()
            << "\t| Var = " << aLoadDists[i].Variance() << "\n";
    }
}