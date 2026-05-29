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

using namespace std;
namespace fs = std::filesystem;

#ifndef SOURCE_DIR
#define SOURCE_DIR "."
#endif

void GetDeepComponents(MultiLayerMixture& theMix, double theX, vector<double>& theOutComps) {
    DeepMixtureTraverser aTraverser(theMix);
    aTraverser.Traverse([&theOutComps, theX](GeneralDistribution& d, double w) {
        theOutComps.push_back(w * d.Density(theX));
        return true;
        });
}

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

void PrintMetrics(const string& theName, IDistribution& theDist) {
    cout << "   -> " << left << setw(20) << theName
        << "| M=" << fixed << setprecision(3) << setw(7) << theDist.ExpectedValue()
        << "| D=" << setw(7) << theDist.Variance()
        << "| As=" << setw(7) << theDist.Asymmetry()
        << "| Ex=" << setw(7) << theDist.Kurtosis() << "\n";
}

void RunExperimentLaba3(const string& theTitle, IDistribution& theTrueDist,
    const string& theOutputDir, const string& theFileName, int theSampleSize = 3000) {
    cout << "\n========================================================================\n";
    cout << "  " << theTitle << " (N = " << theSampleSize << ")\n";
    cout << "========================================================================\n";

    Empiric aData;
    for (int i = 0; i < theSampleSize; i++) aData.AddData(theTrueDist.RandNum());

    UniversalApproximator aNonRobust(aData, false);
    UniversalApproximator aRobust(aData, true);

    // Пакетное обновление Observer'а
    aData.Notify(1);

    MultiLayerMixture& aModelNR = aNonRobust.GetModel();
    MultiLayerMixture& aModelR = aRobust.GetModel();

    // Статистика
    cout << "[ ХАРАКТЕРИСТИКИ ]\n";
    cout << "   -> " << left << setw(20) << "Эмпирические"
        << "| M=" << fixed << setprecision(3) << setw(7) << aData.Mean()
        << "| D=" << setw(7) << aData.Variance()
        << "| As=" << setw(7) << aData.Asymmetry()
        << "| Ex=" << setw(7) << aData.Kurtosis() << "\n";
    PrintMetrics("Истинное распр.", theTrueDist);
    PrintMetrics("Неробастная аппр.", aModelNR);
    PrintMetrics("Робастная аппр.", aModelR);

    // Критерии
    cout << "\n[ КРИТЕРИИ КАЧЕСТВА ]\n";
    cout << "   Неробастная (k=" << aNonRobust.GetComponentCount() << "): LL = "
        << aNonRobust.GetLogLikelihood() << ", AIC = " << aNonRobust.GetAIC() << ", BIC = " << aNonRobust.GetBIC() << "\n";
    cout << "   Робастная   (k=" << aRobust.GetComponentCount() << "): LL = "
        << aRobust.GetLogLikelihood() << ", AIC = " << aRobust.GetAIC() << ", BIC = " << aRobust.GetBIC() << "\n";

    cout << "\n[ ИСТИННАЯ СМЕСЬ (Глубокий обход) ]\n";
    if (typeid(theTrueDist) == typeid(MultiLayerMixture)) {
        DeepMixtureTraverser aDeepTrue(dynamic_cast<MultiLayerMixture&>(theTrueDist));
        aDeepTrue.Traverse([](GeneralDistribution& d, double w) {
            cout << " -> " << w << " * " << d.Component().Name() << " (M=" << d.ExpectedValue() << ", D=" << d.Variance() << ")\n";
            return true;
            });
    }
    else {
        cout << " -> " << theTrueDist.Name() << " (Not a mixture)\n";
    }

    cout << "\n[ РОБАСТНАЯ МОДЕЛЬ (Глубокий обход) ]\n";
    DeepMixtureTraverser aDeepR(aModelR);
    aDeepR.Traverse([](GeneralDistribution& d, double w) {
        cout << " -> " << w << " * " << d.Component().Name() << " (M=" << d.ExpectedValue() << ", D=" << d.Variance() << ")\n";
        return true;
        });

    // Экспорт
    Histogram aHist(aData, 60);
    string aFilePath = theOutputDir + "/" + theFileName;
    GenerateDataCSV(aFilePath, aData, aHist, theTrueDist, aModelNR, aModelR);
    cout << "\n-> Данные выгружены в " << aFilePath << "\n";
}

void LabTester::RunAllTests() {
    setlocale(0, "");

    // Настройка путей (Относительные пути от исходников)
    string aPythonScript = "../../Laba_3/src/plotter.py";
    string anOutputDir = string(SOURCE_DIR) + "/results";

    // Создаем папку results, если её нет
    if (!fs::exists(anOutputDir)) {
        fs::create_directories(anOutputDir);
    }

    // ТЕСТ 1: 3 хорошо разделимые нормальные компоненты
    MultiLayerMixture aMix1;
    aMix1.Add(GeneralDistribution(Normal(-5.0, 0.8)), 0.3);
    aMix1.Add(GeneralDistribution(Normal(0.0, 1.2)), 0.4);
    aMix1.Add(GeneralDistribution(Normal(5.0, 0.9)), 0.3);
    RunExperimentLaba3("ТЕСТ 1: Хорошо разделимая смесь N(x)", aMix1, anOutputDir, "test1_separated.csv");

    // ТЕСТ 2: 3 сильно перекрывающиеся компоненты (Сложно для EM-алгоритма)
    MultiLayerMixture aMix2;
    aMix2.Add(GeneralDistribution(Normal(-1.0, 1.5)), 0.3);
    aMix2.Add(GeneralDistribution(Normal(0.0, 0.5)), 0.4);
    aMix2.Add(GeneralDistribution(Normal(1.0, 2.0)), 0.3);
    RunExperimentLaba3("ТЕСТ 2: Перекрывающаяся смесь N(x)", aMix2, anOutputDir, "test2_overlapping.csv", 5000);

    // ТЕСТ 3: Смесь из Теста 1 + 5% шума
    MultiLayerMixture aMix3;
    aMix3.Add(GeneralDistribution(aMix1), 0.95);
    aMix3.Add(GeneralDistribution(Uniform(-15.0, 30.0)), 0.05); // Размазанный шум
    RunExperimentLaba3("ТЕСТ 3: Смесь + 5% равномерного шума", aMix3, anOutputDir, "test3_light_noise.csv");

    // ТЕСТ 4: Смесь из Теста 1 + 20% жесткого шума
    MultiLayerMixture aMix4;
    aMix4.Add(GeneralDistribution(aMix1), 0.80);
    aMix4.Add(GeneralDistribution(Uniform(-15.0, 30.0)), 0.20);
    RunExperimentLaba3("ТЕСТ 4: Смесь + 20% сильного шума", aMix4, anOutputDir, "test4_heavy_noise.csv");

    // ТЕСТ 5: Вариант 3 (IG_L)
    IGLDistribution anIGL(0.0, 1.5, 3.0);
    RunExperimentLaba3("ТЕСТ 5: Распределение Варианта 3 (IG_L)", anIGL, anOutputDir, "test5_igl.csv", 4000);

    // ТЕСТ 6: Эффект матрешки (Смесь внутри смеси внутри смеси)
    MultiLayerMixture aDeepMixL1, aDeepMixL2, aDeepMixL3;
    aDeepMixL1.Add(GeneralDistribution(Normal(-10.0, 1.0)), 1.0);
    aDeepMixL2.Add(GeneralDistribution(aDeepMixL1), 0.5);
    aDeepMixL2.Add(GeneralDistribution(Normal(0.0, 1.0)), 0.5);
    aDeepMixL3.Add(GeneralDistribution(aDeepMixL2), 0.5);
    aDeepMixL3.Add(GeneralDistribution(Normal(10.0, 1.0)), 0.5);
    RunExperimentLaba3("ТЕСТ 6: Глубокая Матрешка (3 уровня вложенности)", aDeepMixL3, anOutputDir, "test6_matryoshka.csv");

    // Запуск Python-скрипта с передачей пути к выходной папке
    string aCmd = "python \"" + aPythonScript + "\" \"" + anOutputDir + "\"";
    cout << "\n========================================================================\n";
    cout << "Запуск визуализатора: " << aCmd << "\n";
    std::system(aCmd.c_str());
    cout << "Готово! Все результаты в папке: " << anOutputDir << "\n";
}