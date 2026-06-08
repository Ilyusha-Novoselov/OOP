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
void GenerateDataCSV(const string& theFilePath, const string& theTitle, Empiric& theData, Histogram& theHist,
    IDistribution& theTrue, MultiLayerMixture& theNonRob, MultiLayerMixture& theRob) {

    ofstream anOut(theFilePath);
    anOut << "# title: " << theTitle << "\n";
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
    cout << "   -> " << left << setw(22) << theName
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
        string aNodeType = (typeid(item.first.Component()) == typeid(MultiLayerMixture))
            ? "[Узел Смеси]" : "[Базовый Лист]";
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

// Главная функция проведения эксперимента
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
    cout << "   -> " << left << setw(22) << "Эмпирические данные"
        << "| M=" << fixed << setprecision(3) << setw(7) << aData.Mean()
        << "| D=" << setw(7) << aData.Variance()
        << "| As=" << setw(7) << aData.Asymmetry()
        << "| Ex=" << setw(7) << aData.Kurtosis() << "\n";
    PrintMetrics("Истинное распр.", theTrueDist);
    PrintMetrics("Неробастная аппр.", aModelNR);
    PrintMetrics("Робастная аппр.", aModelR);

    // Пункт 3.2: Сравнение по критериям качества (BIC, AIC, ICL согласно п. 2.2.1)
    cout << "\n[ КРИТЕРИИ КАЧЕСТВА (BIC / AIC / ICL) ]\n";
    cout << "   Неробастная (k=" << aNonRobust.GetComponentCount() << "):"
        << "  BIC=" << fixed << setprecision(2) << aNonRobust.GetBIC()
        << "  AIC=" << aNonRobust.GetAIC()
        << "  ICL=" << aNonRobust.GetICL()
        << "  LL=" << aNonRobust.GetLogLikelihood() << "\n";
    cout << "   Робастная   (k=" << aRobust.GetComponentCount() << "):"
        << "  BIC=" << aRobust.GetBIC()
        << "  AIC=" << aRobust.GetAIC()
        << "  ICL=" << aRobust.GetICL()
        << "  LL=" << aRobust.GetLogLikelihood() << "\n";

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
    GenerateDataCSV(aFilePath, theTitle, aData, aHist, theTrueDist, aModelNR, aModelR);
    cout << "\n-> Плотности компонент выгружены в " << aFilePath << "\n";
}

void LabTester::RunAllTests() {
    setlocale(0, "");

    string aPythonScript = "../../Laba_3/src/plotter.py";
    string anOutputDir = string(SOURCE_DIR) + "/results";

    if (!fs::exists(anOutputDir)) {
        fs::create_directories(anOutputDir);
    }

    // =========================================================================
    // ПУНКТ 3.1.1: 2-уровневая смесь с небольшим числом нормальных компонент
    // =========================================================================
    MultiLayerMixture aSubMix311;
    aSubMix311.Add(GeneralDistribution(Normal(-5.0, 1.0)), 0.5);
    aSubMix311.Add(GeneralDistribution(Normal(5.0, 1.0)), 0.5);

    MultiLayerMixture aMix311;
    aMix311.Add(GeneralDistribution(Normal(0.0, 0.8)), 0.4);
    aMix311.Add(GeneralDistribution(aSubMix311), 0.6);

    RunExperimentLaba3(
        "ЭКСПЕРИМЕНТ 3.1.1: Двухуровневая смесь (3 компоненты)",
        "Двухуровневая структура, 3 нормальные компоненты (п. 3.1.1). N=3000.",
        aMix311, anOutputDir, "exp_3_1_1.csv", 3000);

    // =========================================================================
    // ПУНКТ 3.1.2: Смесь из п. 3.1.1 с примесью равномерного распределения
    // =========================================================================
    MultiLayerMixture aMix312;
    aMix312.Add(GeneralDistribution(aMix311), 0.85);
    aMix312.Add(GeneralDistribution(Uniform(-15.0, 15.0)), 0.15);

    RunExperimentLaba3(
        "ЭКСПЕРИМЕНТ 3.1.2: Смесь 3.1.1 + Равномерный шум",
        "Смесь из п. 3.1.1 + 15% равномерного шума. Проверка робастности. N=3000.",
        aMix312, anOutputDir, "exp_3_1_2.csv", 3000);

    // =========================================================================
    // ПУНКТ 3.1.3: Распределение из варианта (IG_L)
    // =========================================================================
    IGLDistribution anIGL(0.0, 1.5, 3.0);
    RunExperimentLaba3(
        "ЭКСПЕРИМЕНТ 3.1.3: Распределение по варианту (IG_L)",
        "Вариант №3: IG_L(сдвиг=0, масштаб=1.5, форма=3). Тяжёлые хвосты. N=4000.",
        anIGL, anOutputDir, "exp_3_1_3.csv", 4000);

    // =========================================================================
    // ДОПОЛНИТЕЛЬНЫЕ ТЕСТЫ: разные N
    // =========================================================================
    IGLDistribution anIGL_small(0.0, 1.5, 3.0);
    RunExperimentLaba3(
        "ДОП. 4.1: IG_L — малая выборка N=500",
        "IG_L(0, 1.5, 3). Малый объём: аппроксиматор может недооценить число компонент.",
        anIGL_small, anOutputDir, "exp_igl_n500.csv", 500);

    // =========================================================================
    // ДОПОЛНИТЕЛЬНЫЕ ТЕСТЫ: разные сдвиги IG_L
    // =========================================================================
    IGLDistribution anIGL_shift_neg(-10.0, 1.5, 3.0);
    RunExperimentLaba3(
        "ДОП. 5.1: IG_L — сдвиг -10",
        "IG_L(сдвиг=-10, масштаб=1.5, форма=3). Проверка работы при отрицательном сдвиге.",
        anIGL_shift_neg, anOutputDir, "exp_igl_shift_neg10.csv", 1000);

    IGLDistribution anIGL_shift_pos(15.0, 1.5, 3.0);
    RunExperimentLaba3(
        "ДОП. 5.2: IG_L — сдвиг +15",
        "IG_L(сдвиг=+15, масштаб=1.5, форма=3). Проверка работы при большом положительном сдвиге.",
        anIGL_shift_pos, anOutputDir, "exp_igl_shift_pos15.csv", 1000);

    // =========================================================================
    // ДОПОЛНИТЕЛЬНЫЕ ТЕСТЫ: разные масштабы IG_L
    // =========================================================================
    IGLDistribution anIGL_narrow(0.0, 0.3, 3.0);
    RunExperimentLaba3(
        "ДОП. 6.1: IG_L — узкий масштаб 0.3",
        "IG_L(0, масштаб=0.3, форма=3). Узкое распределение — пик в нуле.",
        anIGL_narrow, anOutputDir, "exp_igl_scale_narrow.csv", 1000);

    IGLDistribution anIGL_wide(0.0, 5.0, 3.0);
    RunExperimentLaba3(
        "ДОП. 6.2: IG_L — широкий масштаб 5.0",
        "IG_L(0, масштаб=5.0, форма=3). Широкое распределение — размытые хвосты.",
        anIGL_wide, anOutputDir, "exp_igl_scale_wide.csv", 1000);

    // =========================================================================
    // ДОПОЛНИТЕЛЬНЫЕ ТЕСТЫ: разные формы (параметр shape) IG_L
    // =========================================================================
    IGLDistribution anIGL_laplace(0.0, 1.5, 0.0);  // shape=0 => Лаплас
    RunExperimentLaba3(
        "ДОП. 7.1: IG_L — форма=0 (Лаплас)",
        "IG_L(0, 1.5, форма=0): вырождается в распределение Лапласа.",
        anIGL_laplace, anOutputDir, "exp_igl_shape0_laplace.csv", 1000);

    IGLDistribution anIGL_heavytail(0.0, 1.5, 10.0);
    RunExperimentLaba3(
        "ДОП. 7.2: IG_L — форма=10 (очень тяжёлые хвосты)",
        "IG_L(0, 1.5, форма=10): экстремальные хвосты. Сложный случай для EM.",
        anIGL_heavytail, anOutputDir, "exp_igl_shape10.csv", 1000);

    // =========================================================================
    // ДОПОЛНИТЕЛЬНЫЕ ТЕСТЫ: разные N для смеси 3.1.1
    // =========================================================================
    {
        MultiLayerMixture aSubMixSmall;
        aSubMixSmall.Add(GeneralDistribution(Normal(-5.0, 1.0)), 0.5);
        aSubMixSmall.Add(GeneralDistribution(Normal(5.0, 1.0)), 0.5);
        MultiLayerMixture aMixSmall;
        aMixSmall.Add(GeneralDistribution(Normal(0.0, 0.8)), 0.4);
        aMixSmall.Add(GeneralDistribution(aSubMixSmall), 0.6);
        RunExperimentLaba3(
            "ДОП. 8.1: Смесь 3.1.1 — N=1000",
            "Та же двухуровневая смесь, но N=1000. Хватит ли данных для выявления 3 компонент?",
            aMixSmall, anOutputDir, "exp_311_n1000.csv", 1000);
    }

    // =========================================================================
    // ДОПОЛНИТЕЛЬНО: Экстремальное слияние плотностей (для защиты)
    // =========================================================================
    MultiLayerMixture aMixBonus;
    aMixBonus.Add(GeneralDistribution(Normal(-2.0, 1.5)), 0.4);
    aMixBonus.Add(GeneralDistribution(Normal(2.0, 1.5)), 0.6);
    RunExperimentLaba3(
        "ДОП. 9: Сильное перекрытие плотностей",
        "Две нормальные компоненты с широкой дисперсией — сливаются в единый колокол.",
        aMixBonus, anOutputDir, "exp_bonus_overlapping.csv", 4000);

    cout << "\n========================================================================\n";
    cout << "Запуск визуализатора (Python)...\n";
    string aCmd = "python \"" + aPythonScript + "\" \"" + anOutputDir + "\"";
    std::system(aCmd.c_str());
    cout << "Готово! Все результаты в папке: " << anOutputDir << "\n";
}