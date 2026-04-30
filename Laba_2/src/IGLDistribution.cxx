#include <IGLDistribution.hxx>

#include <cmath>
#include <string>
#include <random>
#include <limits>


// Вспомогательная функция для расчета omega из методички
static double getOmega(double v) {
    return std::sqrt(3.0 * v + 1.0);
}

double IGLDistribution::Density(double theX) const
{
    // Обработка краевого случая
    if (std::abs(myShape) <= std::numeric_limits<double>::epsilon()) {
        double aX = std::abs((theX - myShift) / myScale);
        return (0.5 * std::exp(-aX)) / myScale;
    }

    double aX = std::abs((theX - myShift) / myScale);
    double v = myShape;
    double w = getOmega(v);

    double aRoot = std::sqrt(1.0 + 2.0 * v * aX);
    double aF = (w / (2.0 * aRoot)) * std::exp((w / v) * (1.0 - aRoot));

    return aF / myScale;
}

double IGLDistribution::ExpectedValue() const { return myShift; }

double IGLDistribution::Variance() const
{
    if (std::abs(myShape) <= std::numeric_limits<double>::epsilon()) {
        return 2.0 * myScale * myScale;
    }

    double v = myShape;
    double w = getOmega(v);

    // Формула из image_cf0e5c.png: 2 * [3v(v + w + 1) + 1] / w^4
    double aNum = 2.0 * (3.0 * v * (v + w + 1.0) + 1.0);

    // Оптимизация w^4 = (3v + 1)^2 вместо std::pow(w, 4.0)
    double w2 = 3.0 * v + 1.0;
    double aDen = w2 * w2;

    return (aNum / aDen) * myScale * myScale;
}

double IGLDistribution::Asymmetry() const { return 0.0; }

double IGLDistribution::Kurtosis() const
{
    if (std::abs(myShape) <= std::numeric_limits<double>::epsilon()) {
        return 3.0;
    }

    double v = myShape;
    double w = getOmega(v);

    // k(v) из методички
    double kv = 10.0 * v * (21.0 * v * v * (v + w + 1.0) + 2.0 * w * w * w)
        + 12.0 * v * (5.0 * v * v + 9.0 * v + 1.0)
        + std::pow(3.0 * v * (v + w + 1.0) + 1.0, 2.0);

    // Знаменатель основной формулы
    double aDenBase = w + 3.0 * v * (w + v * (w + 3.0) + 1.0);
    double aKurt = (3.0 * w * w * (kv + 2.0)) / (aDenBase * aDenBase) - 6.0;

    return aKurt;
}

double IGLDistribution::RandNum()
{
    // Обработка краевого случая: если shape == 0, генерируем величину Лапласа
    if (std::abs(myShape) <= std::numeric_limits<double>::epsilon()) {
        double e = myExp(myEngine);
        double b = myBern(myEngine) ? 1.0 : 0.0;
        double z = e * (2.0 * b - 1.0);
        return myShift + myScale * z;
    }

    double v = myShape;
    double w = getOmega(v);

    // Шаг 1: Получить u
    double a = myNorm(myEngine);
    double y = a * a;

    double aTerm = std::sqrt((4.0 * w * w * w * y) / v + w * w * y * y);
    double u = w + (v * y) / 2.0 - (v / (2.0 * w)) * aTerm;

    // Шаг 2: Получить r и z (лапласовская)
    double r = myUnif(myEngine);
    double e = myExp(myEngine);
    double b = myBern(myEngine) ? 1.0 : 0.0;
    double z = e * (2.0 * b - 1.0);

    // Шаг 3: Выбор реализации по методу суперпозиции
    double aTarget;
    if (r <= (w / (w + u))) {
        aTarget = z / u;
    } else {
        aTarget = (z * u) / (w * w);
    }

    return myShift + myScale * aTarget;
}

void IGLDistribution::Save(std::ostream& theOut) const
{
    theOut << "IG_L " << myShift << " " << myScale << " " << myShape << "\n";
}

void IGLDistribution::Load(std::istream& theIn)
{
    std::string aName;
    theIn >> aName >> myShift >> myScale >> myShape;
}

IDistribution* IGLDistribution::Clone() const { return new IGLDistribution(*this); }

std::string IGLDistribution::Name() const { return "IG_L"; }
