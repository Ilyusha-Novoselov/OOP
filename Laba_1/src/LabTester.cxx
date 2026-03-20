#include <LabTester.hxx>

#include <Empiric.hxx>
#include <Normal.hxx>
#include <Uniform.hxx>
#include <IGLDistribution.hxx>

#include <iostream>
#include <iomanip>
#include <cmath>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <vector>
#include <utility>


using namespace std;

void LabTester::RunAllTests()
{
    std::setlocale(LC_ALL, "");

    // Размеры выборок для тестов
    vector<int> aStatSizes = { 100, 10000, 1000000 };
    vector<int> anOutputSizes = { 100, 1000 };

    // --- ПЕРЕБОР ПАРАМЕТРОВ ДЛЯ NORMAL ---
    cout << "\n==================================================";
    cout << "\n   ТЕСТИРОВАНИЕ НОРМАЛЬНОГО РАСПРЕДЕЛЕНИЯ (Normal)";
    cout << "\n==================================================";
    // Параметры: {Shift, Scale}
    vector<pair<double, double>> aNormParams = {
        {0.0, 1.0},    // Стандартное нормальное распределение N(0, 1). Классический тест.
        {5.0, 0.1},    // Узкое распределение (острый пик). Краевой случай на малый разброс.
        {-20.0, 10.0}, // Широкое распределение (колокол сильно размазан), уходящее в отрицательную область.
        {100.0, 2.0}   // Сильное смещение далеко вправо при небольшом отклонении.
    };

    for (const auto& p : aNormParams) {
        Normal aNormDist(p.first, p.second);

        ostringstream oss;
        oss << "Normal_sh" << p.first << "_sc" << p.second;
        string aName = oss.str();

        cout << "\n\n>>> Значения параметров: Shift = " << p.first << ", Scale = " << p.second << " <<<";
        TestStats(aNormDist, aName, aStatSizes);
        TestOutput(aNormDist, aName, anOutputSizes);
    }

    // --- ПЕРЕБОР ПАРАМЕТРОВ ДЛЯ UNIFORM ---
    cout << "\n\n==================================================";
    cout << "\n ТЕСТИРОВАНИЕ РАВНОМЕРНОГО РАСПРЕДЕЛЕНИЯ (Uniform)";
    cout << "\n==================================================";
    // Параметры: {Shift, Scale}
    vector<pair<double, double>> anUnifParams = {
        {0.0, 1.0},    // Отрезок [-1, 1]. Базовый случай.
        {0.5, 0.5},    // Отрезок [0, 1]. Классический стандартный ГСЧ.
        {10.0, 0.05},  // Отрезок [9.95, 10.05]. Очень узкий интервал, проверка на чувствительность шага гистограммы.
        {-50.0, 25.0}  // Отрезок [-75, -25]. Широкий разброс строго в отрицательных числах.
    };

    for (const auto& p : anUnifParams) {
        Uniform anUnifDist(p.first, p.second);

        ostringstream oss;
        oss << "Uniform_sh" << p.first << "_sc" << p.second;
        string aName = oss.str();

        cout << "\n\n>>> Значения параметров: Shift = " << p.first << ", Scale = " << p.second << " <<<";
        TestStats(anUnifDist, aName, aStatSizes);
        TestOutput(anUnifDist, aName, anOutputSizes);
    }

    // --- ПЕРЕБОР ПАРАМЕТРОВ ДЛЯ IGLDistribution ---
    cout << "\n\n==================================================";
    cout << "\n       ТЕСТИРОВАНИЕ РАСПРЕДЕЛЕНИЯ IG_L (Вариант 3)";
    cout << "\n==================================================";
    struct IGLParams { double shift, scale, shape; };
    // Параметры: {Shift, Scale, Shape}
    vector<IGLParams> anIGLParams = {
        {0.0, 1.0, 2.0},   // Базовый случай по варианту.
        {0.0, 1.0, 0.0},   // КРАЕВОЙ СЛУЧАЙ: Shape = 0 (вырождается в распределение Лапласа, отрабатывает отдельный if в коде).
        {-5.0, 2.0, 0.1},  // Малое ненулевое значение формы (Shape = 0.1), проверка перехода от Лапласа к IG_L.
        {10.0, 0.5, 5.0}   // Большое значение параметра формы, узкий масштаб. Специфический вид пика.
    };

    for (const auto& p : anIGLParams) {
        IGLDistribution anIGLDist(p.shift, p.scale, p.shape);

        ostringstream oss;
        oss << "IG_L_sh" << p.shift << "_sc" << p.scale << "_shape" << p.shape;
        string aName = oss.str();

        cout << "\n\n>>> Значения параметров: Shift = " << p.shift << ", Scale = " << p.scale << ", Shape = " << p.shape << " <<<";
        TestStats(anIGLDist, aName, aStatSizes);
        TestOutput(anIGLDist, aName, anOutputSizes);
    }

    // --- ПУНКТ 6.4 ---
    TestObserver();

    TestIGLTable();
}

// ПУНКТ 6.1: сгенерировать несколько выборок разного объема и поместить в объекты
// ПУНКТ 6.2: вычислить выборочные характеристики и проконтролировать их близость с теоретическими
void LabTester::TestStats(IDistribution& theDist, const string& theName, const vector<int>& theSizes)
{
    cout << "\n=== Проверка характеристик: " << theName << " ===\n";
    cout << setw(10) << "Объем" << " | "
        << setw(15) << "Мат. ожидание" << " | "
        << setw(15) << "Дисперсия" << " | "
        << setw(15) << "Асимметрия" << " | "
        << setw(15) << "Эксцесс" << "\n";
    cout << string(80, '-') << "\n";

    // Теоретические значения
    cout << setw(10) << "ТЕОРИЯ" << " | "
        << setw(15) << theDist.ExpectedValue() << " | "
        << setw(15) << theDist.Variance() << " | "
        << setw(15) << theDist.Asymmetry() << " | "
        << setw(15) << theDist.Kurtosis() << "\n";

    // Эмпирические значения (ПУНКТ 6.1 - помещение в набор данных)
    for (int aSize : theSizes) {
        Empiric anEmpiric;
        for (int i = 0; i < aSize; ++i) {
            double aVal = theDist.RandNum();
            anEmpiric.AddData(aVal);
        }

        // ПУНКТ 6.2 - вычисление эмпирических характеристик
        cout << setw(10) << aSize << " | "
            << setw(15) << anEmpiric.Mean() << " | "
            << setw(15) << anEmpiric.Variance() << " | "
            << setw(15) << anEmpiric.Asymmetry() << " | "
            << setw(15) << anEmpiric.Kurtosis() << "\n";
    }
}

// ПУНКТ 6.3: Вывод в текстовый файл с двумя наборами точек: 
// 1) Точки выборки + теоретическая плотность (гладкая кривая)
// 2) Точки выборки + эмпирическая плотность ("облако из ступенек")
void LabTester::TestOutput(IDistribution& theDist, const string& theName, const vector<int>& theSizes)
{
    cout << "\n=== п. 6.3: Генерация файлов точек для " << theName << " ===\n";

    for (int aSize : theSizes) {
        Empiric anEmpiric;
        for (int i = 0; i < aSize; ++i) {
            double aVal = theDist.RandNum();
            anEmpiric.AddData(aVal);
        }

        // Формула Стерджеса для количества столбцов гистограммы
        int aBins = static_cast<int>(std::log2(anEmpiric.Size())) + 1;
        Histogram aHist(anEmpiric, aBins);

        string aFileName = theName + "_" + std::to_string(aSize) + ".txt";
        ofstream anOut(aFileName);

        if (anOut.is_open()) {
            // Обязательно сортируем точки по оси X
            vector<double> aRawData = anEmpiric.GetRawData();
            std::sort(aRawData.begin(), aRawData.end());

            // НАБОР 1: ТЕОРЕТИЧЕСКАЯ ПЛОТНОСТЬ
            // x = сгенерированная точка, y = точное значение по формуле распределения
            for (double aVal : aRawData) {
                double aTheorDens = theDist.Density(aVal);
                anOut << aVal << " " << aTheorDens << "\n";
            }

            // Пустая строка для разделения наборов точек для построителя графиков
            anOut << "\n";

            // НАБОР 2: ЭМПИРИЧЕСКАЯ ПЛОТНОСТЬ
            // x = сгенерированная точка, y = плотность корзины, в которую она попала
            for (double aVal : aRawData) {
                double anEmpDens = aHist.GetDensityByValue(aVal);

                anOut << aVal << " " << anEmpDens << "\n";
            }

            anOut.close();
            cout << "Сгенерирован файл: " << aFileName << "\n";
        }
    }
}

// ПУНКТ 6.4: продемонстрировать работу паттерна наблюдатель на малых выборках
void LabTester::TestObserver()
{
    cout << "\n=== ПУНКТ 6.4: ДЕМОНСТРАЦИЯ ПАТТЕРНА НАБЛЮДАТЕЛЬ (Сгенерированные данные) ===\n";

    Empiric anEmpiric;
    Normal aNormalGen(0.0, 1.0);

    // Генерируем начальную выборку (10 точек) своими классами
    for (int i = 0; i < 10; ++i) {
        anEmpiric.AddData(aNormalGen.RandNum());
    }

    // Создаем наблюдателей
    Histogram aHist1(anEmpiric, 3, 0, true); // Автоматический (интерес 0)
    Histogram aHist2(anEmpiric, 3, 1, true); // Ручной (интерес 1)

    vector<double> aState1 = aHist1.GetDensities();
    vector<double> aState2 = aHist2.GetDensities();

    cout << "| " << setw(22) << left << "Гистограмма h1 (Авто)" << " | " << setw(22) << left << "Гистограмма h2 (Ручн)" << " |\n";
    PrintState("Изначальное состояние", aHist1, aHist2);

    // Добавляем новый элемент (сгенерированный)
    double aNewVal = aNormalGen.RandNum();
    cout << "--> Добавляем в выборку значение: " << aNewVal << endl;
    anEmpiric.AddData(aNewVal);

    PrintState("После AddData (h1 обновилась*)", aHist1, aHist2, aState1, aState2);

    aState1 = aHist1.GetDensities();
    aState2 = aHist2.GetDensities();

    // Принудительное уведомление для интереса 1
    cout << "--> Вызываем Notify(1) для ручного обновления h2..." << endl;
    anEmpiric.Notify(1);

    PrintState("После Notify(1) (h2 обновилась*)", aHist1, aHist2, aState1, aState2);
}

void LabTester::TestIGLTable()
{
    cout << "\n=== Сверка характеристик IGL с теоретической таблицей ===\n";

    vector<double> aShapes = { 0.0, 0.01, 0.02, 0.03, 0.04, 0.05, 0.1, 0.15 };

    cout << fixed << setprecision(3);

    cout << setw(5) << left << "v" << " |";
    for (double v : aShapes) {
        cout << setw(7) << right << v << " |";
    }
    cout << "\n" << string(7 + 10 * 10, '-') << "\n";

    cout << setw(5) << left << "s^2" << " |";
    for (double v : aShapes) {
        IGLDistribution aDist(0.0, 1.0, v);
        cout << setw(7) << right << aDist.Variance() << " |";
    }
    cout << "\n";

    cout << setw(5) << left << "g_2" << " |";
    for (double v : aShapes) {
        IGLDistribution aDist(0.0, 1.0, v);
        cout << setw(7) << right << aDist.Kurtosis() << " |";
    }
    cout << "\n";

    cout << setw(5) << left << "f_0" << " |";
    for (double v : aShapes) {
        IGLDistribution aDist(0.0, 1.0, v);
        cout << setw(7) << right << aDist.Density(0.0) << " |";
    }
    cout << "\n";

    cout << defaultfloat;
}

string LabTester::FormatBin(double theLower, double theUpper, double theDensity, bool theIsLast, bool theChanged)
{
    std::ostringstream oss;
    char aBracket = theIsLast ? ']' : ')';

    oss << "[" << std::fixed << std::setprecision(2) << theLower << ", "
        << theUpper << aBracket << ": " << theDensity;

    string aRes = oss.str();
    if (theChanged) {
        aRes = "*" + aRes + "*";
    }
    return aRes;
}

void LabTester::PrintState(const string& theTitle, const Histogram& theH1, const Histogram& theH2,
    const vector<double>& thePrevDensities1, const vector<double>& thePrevDensities2)
{
    cout << string(52, '-') << "\n";
    cout << setw(38) << left << ("  " + theTitle) << "\n";
    cout << string(52, '-') << "\n";

    for (int i = 0; i < theH1.GetBinsCount(); ++i) {
        bool anIsLast = (i == theH1.GetBinsCount() - 1);

        bool aChanged1 = !thePrevDensities1.empty() && (abs(theH1.GetDensity(i) - thePrevDensities1[i]) > 1e-5);
        bool aChanged2 = !thePrevDensities2.empty() && (abs(theH2.GetDensity(i) - thePrevDensities2[i]) > 1e-5);

        string aStr1 = FormatBin(theH1.GetBound(i), theH1.GetBound(i + 1), theH1.GetDensity(i), anIsLast, aChanged1);
        string aStr2 = FormatBin(theH2.GetBound(i), theH2.GetBound(i + 1), theH2.GetDensity(i), anIsLast, aChanged2);

        cout << "| " << setw(22) << left << aStr1 << " | " << setw(22) << left << aStr2 << " |\n";
    }
}