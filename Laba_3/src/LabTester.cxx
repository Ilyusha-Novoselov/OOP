#include <LabTester.hxx>
#include <Empiric.hxx>
#include <Histogram.hxx>
#include <Normal.hxx>
#include <Uniform.hxx>
#include <IGLDistribution.hxx>
#include <MultiLayerMixture.hxx>
#include <Iterators.hxx>
#include <UniversalApproximator.hxx>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <filesystem>
#include <typeinfo>

using namespace std;
namespace fs = std::filesystem;

#ifndef SOURCE_DIR
#define SOURCE_DIR "."
#endif

// Пункт 3.5: Получение теоретических плотностей всех компонент на каждом шаге
void GetDeepComponents(MultiLayerMixture& theMix, double theX, vector<double>& theOutComps) {
    DeepMixtureTraverser aTraverser(theMix);
    aTraverser.Traverse([&theOutComps, theX](GeneralDistribution& d, double w) {
        theOutComps.push_back(w * d.Density(theX));
        return true;
        });
}

// Выгрузка в CSV для питона
void GenerateDataCSV(const string& theFilePath, Empiric& theData, Histogram& theHist,
    IDistribution& theTrue, MultiLayerMixture& theNonRob, MultiLayerMixture& theRob) {

    ofstream anOut(theFilePath);
    vector<double> aRaw = theData.GetRawData();
    sort(aRaw.begin(), aRaw.end());

    vector<double> aNRComps, aRComps;
    GetDeepComponents(theNonRob, 0.0, aNRComps);
    GetDeepComponents(theRob, 0.0, aRComps);

    anOut << "x,emp,true_pdf,nonrob_pdf,rob_pdf";
    for (size_t i = 0; i < aNRComps.size(); ++i) anOut << ",nr_c" << i;
    for (size_t i = 0; i < aRComps.size(); ++i) anOut << ",r_c" << i;
    anOut << "\n";

    for (double x : aRaw) {
        aNRComps.clear();
        aRComps.clear();
        GetDeepComponents(theNonRob, x, aNRComps);
        GetDeepComponents(theRob, x, aRComps);

        anOut << x << ","
            << theHist.GetDensityByValue(x) << ","
            << theTrue.Density(x) << ","
            << theNonRob.Density(x) << ","
            << theRob.Density(x);

        for (double c : aNRComps) anOut << "," << c;
        for (double c : aRComps) anOut << "," << c;
        anOut << "\n";
    }
    anOut.close();
}

// Пункт 3.4: Сравнение эмпирических и теоретических характеристик
void PrintMetrics(const string& theName, IDistribution& theDist) {
    cout << "   -> " << left << setw(20) << theName
        << "| M=" << fixed << setprecision(3) << setw(7) << theDist.ExpectedValue()
        << "| D=" << setw(7) << theDist.Variance()
        << "| As=" << setw(7) << theDist.Asymmetry()
        << "| Ex=" << setw(7) << theDist.Kurtosis() << "\n";
}

// Пункт 3.3: Распечатка смесей (Внешний/Поверхностный итератор)
void PrintShallow(MultiLayerMixture& theMix) {
    ShallowMixtureIterator aShallow(theMix);
    for (aShallow.First(); !aShallow.IsDone(); aShallow.Next()) {
        auto item = aShallow.CurrentItem();
        string aNodeType = (typeid(item.first.Component()) == typeid(MultiLayerMixture)) ? "[Узел Смеси]" : "[Базовый Лист]";
        cout << "   " << aNodeType << " " << fixed << setprecision(3) << item.second
            << " * " << item.first.Component().Name()
            << " (M=" << item.first.ExpectedValue() << ", D=" << item.first.Variance() << ")\n";
    }
}

// Пункт 3.3: Распечатка смесей (Внутренний/Глубокий итератор)
void PrintDeep(MultiLayerMixture& theMix) {
    DeepMixtureTraverser aDeep(theMix);
    aDeep.Traverse([](GeneralDistribution& d, double w) {
        cout << "   [Лист] " << fixed << setprecision(3) << w
            << " * " << d.Component().Name()
            << " (M=" << d.ExpectedValue() << ", D=" << d.Variance() << ")\n";
        return true;
        });
}

// Главная функция проведения эксперимента (Объединяет пункты 3.2, 3.3, 3.4, 3.5)
void RunExperimentLaba3(const string& theTitle, const string& theDesc, IDistribution& theTrueDist,
    const string& theOutputDir, const string& theFileName, int theSampleSize = 3000) {
    cout << "\n========================================================================\n";
    cout << "  " << theTitle << " (N = " << theSampleSize << ")\n";
    cout << "========================================================================\n";
    cout << "ОПИСАНИЕ ВХОДНЫХ ДАННЫХ: " << theDesc << "\n\n";

    // Пункт 3.1: Генерация выборки
    Empiric aData;
    for (int i = 0; i < theSampleSize; i++) aData.AddData(theTrueDist.RandNum());

    // Пункт 3.2: Универсальный аппроксиматор (неробастный и робастный)
    UniversalApproximator aNonRobust(aData, false);
    UniversalApproximator aRobust(aData, true);

    aData.Notify(1);

    MultiLayerMixture& aModelNR = aNonRobust.GetModel();
    MultiLayerMixture& aModelR = aRobust.GetModel();

    // Пункт 3.4: Вывод характеристик
    cout << "[ СРАВНЕНИЕ ХАРАКТЕРИСТИК ]\n";
    cout << "   -> " << left << setw(20) << "Эмпирические данные"
        << "| M=" << fixed << setprecision(3) << setw(7) << aData.Mean()
        << "| D=" << setw(7) << aData.Variance()
        << "| As=" << setw(7) << aData.Asymmetry()
        << "| Ex=" << setw(7) << aData.Kurtosis() << "\n";
    PrintMetrics("Истинное распр.", theTrueDist);
    PrintMetrics("Неробастная аппр.", aModelNR);
    PrintMetrics("Робастная аппр.", aModelR);

    // Пункт 3.2: Сравнение по критериям качества
    cout << "\n[ КРИТЕРИИ КАЧЕСТВА (Выбор числа компонент) ]\n";
    cout << "   Неробастная (k=" << aNonRobust.GetComponentCount() << "): LL = "
        << aNonRobust.GetLogLikelihood() << ", AIC = " << aNonRobust.GetAIC() << ", BIC = " << aNonRobust.GetBIC() << "\n";
    cout << "   Робастная   (k=" << aRobust.GetComponentCount() << "): LL = "
        << aRobust.GetLogLikelihood() << ", AIC = " << aRobust.GetAIC() << ", BIC = " << aRobust.GetBIC() << "\n";

    // Пункт 3.3: Распечатка смесей через итераторы
    cout << "\n[ СТРУКТУРА - ИСТИННОЕ РАСПРЕДЕЛЕНИЕ ]\n";
    if (typeid(theTrueDist) == typeid(MultiLayerMixture)) {
        MultiLayerMixture& aTrueMix = dynamic_cast<MultiLayerMixture&>(theTrueDist);
        cout << "--- Внешний итератор (1 уровень) ---\n";
        PrintShallow(aTrueMix);
        cout << "--- Внутренний итератор (Все листья) ---\n";
        PrintDeep(aTrueMix);
    }
    else {
        cout << "   -> " << theTrueDist.Name() << " (Не является смесью. Итераторы не применимы.)\n";
    }

    cout << "\n[ СТРУКТУРА - НЕРОБАСТНАЯ МОДЕЛЬ ]\n";
    cout << "--- Внешний итератор (1 уровень) ---\n";
    PrintShallow(aModelNR);
    cout << "--- Внутренний итератор (Все листья) ---\n";
    PrintDeep(aModelNR);

    cout << "\n[ СТРУКТУРА - РОБАСТНАЯ МОДЕЛЬ ]\n";
    cout << "--- Внешний итератор (1 уровень) ---\n";
    PrintShallow(aModelR);
    cout << "--- Внутренний итератор (Все листья) ---\n";
    PrintDeep(aModelR);

    // Пункт 3.5: Сохранение плотностей для графика
    Histogram aHist(aData, 60);
    string aFilePath = theOutputDir + "/" + theFileName;
    GenerateDataCSV(aFilePath, aData, aHist, theTrueDist, aModelNR, aModelR);
    cout << "\n-> Плотности компонент выгружены в " << aFilePath << "\n";
}

void LabTester::RunAllTests() {
    setlocale(0, "");

    string aPythonScript = "../../Laba_3/src/plotter.py";
    string anOutputDir = string(SOURCE_DIR) + "/results";

    if (!fs::exists(anOutputDir)) {
        fs::create_directories(anOutputDir);
    }

    // ==============================================================================
    // ПУНКТ 3.1.1: 2- или 3-уровневая смесь с небольшим числом нормальных компонент
    // ==============================================================================
    // Создаем 2-уровневую матрешку. На нижнем уровне 2 Гауссианы, на верхнем - ещё 1. Всего 3.
    MultiLayerMixture aSubMix311;
    aSubMix311.Add(GeneralDistribution(Normal(-5.0, 1.0)), 0.5);
    aSubMix311.Add(GeneralDistribution(Normal(5.0, 1.0)), 0.5);

    MultiLayerMixture aMix311;
    aMix311.Add(GeneralDistribution(Normal(0.0, 0.8)), 0.4);  // Нормальная компонента (уровень 1)
    aMix311.Add(GeneralDistribution(aSubMix311), 0.6);        // Под-смесь (уровень 1 -> уровень 2)

    RunExperimentLaba3(
        "ЭКСПЕРИМЕНТ 3.1.1: Двухуровневая смесь (3 компоненты)",
        "Двухуровневая древовидная структура, содержащая в сумме 3 нормальные компоненты (п. 3.1.1).",
        aMix311, anOutputDir, "exp_3_1_1.csv");

    // ==============================================================================
    // ПУНКТ 3.1.2: Смесь из п. 3.1.1 с примесью равномерного распределения
    // ==============================================================================
    // Берем матрешку из прошлого пункта целиком и добавляем равномерный шум (уровень 0)
    MultiLayerMixture aMix312;
    aMix312.Add(GeneralDistribution(aMix311), 0.85);
    aMix312.Add(GeneralDistribution(Uniform(-15.0, 15.0)), 0.15); // Шум ломает классический алгоритм

    RunExperimentLaba3(
        "ЭКСПЕРИМЕНТ 3.1.2: Смесь 3.1.1 + Равномерный шум",
        "Смесь из п. 3.1.1, в которую добавлено 15% шумовых данных из равномерного распределения.",
        aMix312, anOutputDir, "exp_3_1_2.csv");

    // ==============================================================================
    // ПУНКТ 3.1.3: Распределение из варианта
    // ==============================================================================
    // Твой вариант 3 (IG_L)
    IGLDistribution anIGL(0.0, 1.5, 3.0);
    RunExperimentLaba3(
        "ЭКСПЕРИМЕНТ 3.1.3: Распределение по варианту (IG_L)",
        "Индивидуальный вариант №3. Закон распределения с экстремальным эксцессом и тяжелыми хвостами.",
        anIGL, anOutputDir, "exp_3_1_3.csv", 4000);

    // ==============================================================================
    // ДОПОЛНИТЕЛЬНО: Экстремальное слияние плотностей (Для защиты)
    // ==============================================================================
    MultiLayerMixture aMixBonus;
    aMixBonus.Add(GeneralDistribution(Normal(-2.0, 1.5)), 0.4);
    aMixBonus.Add(GeneralDistribution(Normal(2.0, 1.5)), 0.6);
    RunExperimentLaba3(
        "ДОП. ЭКСПЕРИМЕНТ: Сильное перекрытие плотностей",
        "Две нормальные компоненты с широкой дисперсией, которые сливаются в единый колокол.",
        aMixBonus, anOutputDir, "exp_bonus_overlapping.csv", 4000);


    cout << "\n========================================================================\n";
    cout << "Запуск визуализатора (Python)...\n";
    string aCmd = "python \"" + aPythonScript + "\" \"" + anOutputDir + "\"";
    std::system(aCmd.c_str());
    cout << "Готово! Все результаты в папке: " << anOutputDir << "\n";
}