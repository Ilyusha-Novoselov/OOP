#include <LabTester.hxx>
#include <DistributionFactory.hxx>
#include <GeneralDistribution.hxx>
#include <IDistribution.hxx>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <iomanip>


void LabTester::RunAllTests()
{
    setlocale(0, "");
    std::cout << "=================================================\n";
    std::cout << "  СТАРТ ПОЛНОГО ТЕСТИРОВАНИЯ ЛАБОРАТОРНОЙ 2\n";
    std::cout << "=================================================\n\n";

    TestSingletonAndFactory();
    TestVirtualConstructors();
    TestMathAndDelegation();
    TestPolymorphicPersistence();
    TestMatryoshkaEffect();

    std::cout << "\n=================================================\n";
    std::cout << "  ВСЕ ТЕСТЫ УСПЕШНО ПРОЙДЕНЫ!\n";
    std::cout << "=================================================\n";
}

void LabTester::TestSingletonAndFactory()
{
    std::cout << "[ТЕСТ 1] Проверка паттернов Singleton и Factory (Саморегистрация)\n";

    // Проверка Синглтона
    DistributionFactory& aFactory1 = DistributionFactory::Instance();
    DistributionFactory& aFactory2 = DistributionFactory::Instance();

    std::cout << "  -> Адрес фабрики 1: " << &aFactory1 << "\n";
    std::cout << "  -> Адрес фабрики 2: " << &aFactory2 << "\n";

    if (&aFactory1 != &aFactory2) {
        throw std::runtime_error("ОШИБКА: Singleton нарушен, адреса фабрик не совпадают!");
    }
    std::cout << "  -> [УСПЕХ] Singleton работает корректно (адреса совпадают).\n\n";

    // Проверка создания
    IDistribution* aNormal = aFactory1.CreateDistribution("Normal");
    std::cout << "  -> Создан объект 'Normal'. Адрес: " << aNormal << "\n";

    if (!aNormal || aNormal->Name() != "Normal") {
        throw std::runtime_error("ОШИБКА: Фабрика не смогла создать Normal!");
    }

    delete aNormal;
    std::cout << "  -> [УСПЕХ] Саморегистрация и генерация объектов фабрикой работают.\n\n";
}

void LabTester::TestVirtualConstructors()
{
    std::cout << "[ТЕСТ 2] Проверка виртуальных конструкторов (Clone и Name)\n";

    IDistribution* anOriginal = DistributionFactory::Instance().CreateDistribution("Uniform");
    IDistribution* aClone = anOriginal->Clone();

    std::cout << "  -> Имя оригинала: " << anOriginal->Name() << ", Адрес: " << anOriginal << "\n";
    std::cout << "  -> Имя клона:     " << aClone->Name() << ", Адрес: " << aClone << "\n";

    if (anOriginal == aClone) {
        throw std::runtime_error("ОШИБКА: Clone вернул указатель на тот же объект, а не создал новый!");
    }
    if (anOriginal->Name() != aClone->Name()) {
        throw std::runtime_error("ОШИБКА: Имена оригинала и клона не совпадают!");
    }

    delete anOriginal;
    delete aClone;
    std::cout << "  -> [УСПЕХ] Клонирование создает независимый объект правильного типа.\n\n";
}

void LabTester::TestMathAndDelegation()
{
    std::cout << "[ТЕСТ 3] Математика ЛР 1 и делегирование в Конверте (GeneralDistribution)\n";

    // Создаем сырое распределение и загружаем в него конкретные параметры
    IDistribution* aRawDist = DistributionFactory::Instance().CreateDistribution("Normal");

    // Имитируем чтение из файла для задания параметров (Например: Name=Normal, Shift=5.0, Scale=2.0)
    std::stringstream aDataStream("Normal 5.0 2.0");
    dynamic_cast<IPersistent*>(aRawDist)->Load(aDataStream);

    // Упаковываем в конверт
    GeneralDistribution anEnvelope(*aRawDist);

    std::cout << "  -> Сравниваем вычисления для Normal(5.0, 2.0). Точка x = 6.0\n";
    std::cout << std::left << std::setw(25) << "  Метод" << std::setw(15) << "Сырой класс" << "Конверт (General)\n";
    std::cout << "  --------------------------------------------------------\n";

    double aRawExp = aRawDist->ExpectedValue();
    double aEnvExp = anEnvelope.ExpectedValue();
    std::cout << std::left << std::setw(25) << "  ExpectedValue():" << std::setw(15) << aRawExp << aEnvExp << "\n";

    double aRawVar = aRawDist->Variance();
    double aEnvVar = anEnvelope.Variance();
    std::cout << std::left << std::setw(25) << "  Variance():" << std::setw(15) << aRawVar << aEnvVar << "\n";

    double aRawDens = aRawDist->Density(6.0);
    double aEnvDens = anEnvelope.Density(6.0);
    std::cout << std::left << std::setw(25) << "  Density(6.0):" << std::setw(15) << aRawDens << aEnvDens << "\n";

    // Из-за ГПСЧ RandNum будет выдавать разные значения при двух подряд вызовах, 
    // поэтому проверяем просто что метод не падает и возвращает число.
    std::cout << std::left << std::setw(25) << "  RandNum() [test]:" << std::setw(15) << aRawDist->RandNum() << anEnvelope.RandNum() << "\n";

    // Жесткая проверка
    if (std::abs(aRawExp - aEnvExp) > 1e-9 || std::abs(aRawDens - aEnvDens) > 1e-9) {
        throw std::runtime_error("ОШИБКА: Расчеты Конверта отличаются от расчетов Письма!");
    }

    delete aRawDist;
    std::cout << "  -> [УСПЕХ] Конверт идеально делегирует математические функции Письму.\n\n";
}

void LabTester::TestPolymorphicPersistence()
{
    std::cout << "[ТЕСТ 4] Полиморфная персистентность (Save / Load)\n";

    // 1. Создаем объект через фабрику
    IDistribution* aRawUniform = DistributionFactory::Instance().CreateDistribution("Uniform");
    std::stringstream aInitStream("Uniform 10.0 5.0"); // Uniform со сдвигом 10 и масштабом 5
    dynamic_cast<IPersistent*>(aRawUniform)->Load(aInitStream);

    GeneralDistribution aSavedEnvelope(*aRawUniform);
    delete aRawUniform;

    // 2. Сохраняем в поток
    std::stringstream aStorage;
    aSavedEnvelope.Save(aStorage);

    std::string aSavedStr = aStorage.str();
    std::cout << "  -> Данные, записанные объектом в поток:\n";
    std::cout << "     \"" << aSavedStr.substr(0, aSavedStr.size() - 1) << "\" (формат: Имя [параметры])\n";

    // 3. Загружаем из потока в АБСОЛЮТНО ДРУГОЙ конверт, который изначально был Normal
    IDistribution* aDummyNormal = DistributionFactory::Instance().CreateDistribution("Normal");
    GeneralDistribution aLoadedEnvelope(*aDummyNormal);
    delete aDummyNormal;

    std::cout << "  -> До Load() тип пустого конверта: " << aLoadedEnvelope.Name() << "\n";

    aLoadedEnvelope.Load(aStorage); // Читаем данные

    std::cout << "  -> После Load() тип конверта:      " << aLoadedEnvelope.Name() << "\n";
    std::cout << "  -> Ожидаемое значение (ExpectedValue) после загрузки: " << aLoadedEnvelope.ExpectedValue() << " (Ожидается 10)\n";

    if (aLoadedEnvelope.Name() != "Uniform" || std::abs(aLoadedEnvelope.ExpectedValue() - 10.0) > 1e-9) {
        throw std::runtime_error("ОШИБКА: Полиморфная загрузка не восстановила нужный тип или параметры!");
    }

    std::cout << "  -> [УСПЕХ] Объект успешно меняет тип во время выполнения на основе данных потока.\n\n";
}

void LabTester::TestMatryoshkaEffect()
{
    std::cout << "[ТЕСТ 5] Защита от эффекта 'Матрешки' (Конверт в Конверте)\n";

    IDistribution* aRawIGL = DistributionFactory::Instance().CreateDistribution("IG_L");

    GeneralDistribution aLevel1(*aRawIGL);
    std::cout << "  -> Создан первый Конверт(Уровень 1) из сырого IG_L.\n";

    // Пытаемся засунуть конверт в конверт
    GeneralDistribution aLevel2(aLevel1);
    std::cout << "  -> Создан второй Конверт(Уровень 2) из Конверта(Уровня 1).\n";

    // Извлекаем "Письмо" из второго конверта.
    const IDistribution& aCoreComponent = aLevel2.Component();

    std::cout << "  -> Внутренний компонент Уровня 2 имеет имя: " << aCoreComponent.Name() << "\n";

    // Если бы сработал эффект матрешки, то aCoreComponent был бы типом GeneralDistribution.
    // Удостоверимся, что внутри именно базовое распределение, а не другой конверт.
    // Для этого проверим, можем ли мы сделать dynamic_cast к GeneralDistribution.
    const GeneralDistribution* aCheckPtr = dynamic_cast<const GeneralDistribution*>(&aCoreComponent);

    if (aCheckPtr != nullptr) {
        throw std::runtime_error("ОШИБКА: Эффект матрешки ДОПУЩЕН! Внутри конверта лежит другой конверт.");
    }

    std::cout << "  -> Dynamic cast проверки показал, что внутри лежит " << aCoreComponent.Name() << ", а не GeneralDistribution.\n";
    std::cout << "  -> [УСПЕХ] Защита от эффекта 'Матрешки' сработала штатно.\n";

    delete aRawIGL;
}
