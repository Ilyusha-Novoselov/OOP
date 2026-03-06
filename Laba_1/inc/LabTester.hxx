#ifndef _Laba1_LabTester_Header
#define _Laba1_LabTester_Header

#include <IDistribution.hxx>
#include <Histogram.hxx>

#include <vector>
#include <string>

class LabTester
{
public:
    void RunAllTests();

private:
    // ПУНКТЫ 6.1 и 6.2: Генерация и проверка статистических характеристик
    void TestStats(IDistribution& theDist, const std::string& theName, const std::vector<int>& theSizes);

    // ПУНКТ 6.3: Выгрузка данных для графиков в формате x y
    void TestOutput(IDistribution& theDist, const std::string& theName, const std::vector<int>& theSizes);

    // ПУНКТ 6.4: Демонстрация паттерна Наблюдатель
    void TestObserver();

    // Проверка табличных значения IGL распределения
    void TestIGLTable();

    // Вспомогательные методы для вывода таблицы Наблюдателя
    std::string FormatBin(double theLower, double theUpper, double theDensity, bool theIsLast, bool theChanged);
    void PrintState(const std::string& theTitle, const Histogram& theH1, const Histogram& theH2,
        const std::vector<double>& thePrevDensities1 = {}, const std::vector<double>& thePrevDensities2 = {});
};

#endif
