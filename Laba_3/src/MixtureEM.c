#include "MixtureEM.h"
#include <math.h>
#include <float.h>
#include <stdlib.h> 

#define MAX_K 10
#define EPS 1e-9

static double normalDensity(double x, double mean, double sigma) {
    if (sigma < EPS) sigma = EPS;
    double z = (x - mean) / sigma;
    return exp(-0.5 * z * z) / (sigma * sqrt(2.0 * 3.14159265358979323846));
}

// Вспомогательная функция для сортировки (нужна для перцентилей)
int compare_doubles(const void* a, const void* b) {
    double arg1 = *(const double*)a;
    double arg2 = *(const double*)b;
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

static void initParams(struct Empiric* data, int n, int k, double* weights, double* means, double* sigmas, double* min_val, double* max_val) {
    double global_mean = 0.0;

    // Выделяем память для временной сортировки выборки
    double* sorted_data = (double*)malloc(sizeof(double) * n);
    for (int i = 0; i < n; i++) {
        sorted_data[i] = empiricData(data, i);
        global_mean += sorted_data[i];
    }
    global_mean /= n;

    // Сортируем данные для поиска 5-го и 95-го перцентилей
    qsort(sorted_data, n, sizeof(double), compare_doubles);

    int p5_idx = (int)(n * 0.05);
    int p95_idx = (int)(n * 0.95);
    if (p5_idx < 0) p5_idx = 0;
    if (p95_idx >= n) p95_idx = n - 1;

    // Игнорируем экстремальные выбросы при расстановке стартовых центров
    *min_val = sorted_data[p5_idx];
    *max_val = sorted_data[p95_idx];
    free(sorted_data);

    double global_var = 0.0;
    for (int i = 0; i < n; i++) {
        double v = empiricData(data, i);
        global_var += (v - global_mean) * (v - global_mean);
    }
    global_var /= n;
    double global_sigma = sqrt(global_var);
    if (global_sigma < EPS) global_sigma = 1.0;

    for (int j = 0; j < k; j++) {
        weights[j] = 1.0 / k;
        if (k == 1) {
            means[j] = global_mean;
        }
        else {
            means[j] = *min_val + (*max_val - *min_val) * j / (double)(k - 1);
        }
        sigmas[j] = global_sigma;
    }
}

int mixtureBuilder(struct Empiric* data, int n, int min_k, int max_k, int max_iter, EMResult* result) {
    if (n <= 0 || !result) return 0;
    double best_bic = DBL_MAX;
    int found = 0;

    for (int k = min_k; k <= max_k; k++) {
        double w[MAX_K], m[MAX_K], s[MAX_K];
        double min_v, max_v;
        initParams(data, n, k, w, m, s, &min_v, &max_v);

        double* gamma = (double*)malloc(sizeof(double) * n * k);
        if (!gamma) return 0;

        for (int iter = 0; iter < max_iter; iter++) {
            // E-ШАГ
            for (int i = 0; i < n; i++) {
                double val = empiricData(data, i);
                double sum = 0.0;
                for (int j = 0; j < k; j++) {
                    gamma[i * k + j] = w[j] * normalDensity(val, m[j], s[j]);
                    sum += gamma[i * k + j];
                }

                if (sum < EPS) {
                    for (int j = 0; j < k; j++) gamma[i * k + j] = 1.0 / k;
                }
                else {
                    for (int j = 0; j < k; j++) gamma[i * k + j] /= sum;
                }
            }

            // M-ШАГ
            for (int j = 0; j < k; j++) {
                double Nk = 0.0, mean = 0.0, var = 0.0;
                for (int i = 0; i < n; i++) {
                    Nk += gamma[i * k + j];
                    mean += gamma[i * k + j] * empiricData(data, i);
                }

                if (Nk < EPS) {
                    w[j] = EPS;
                    m[j] = min_v + (max_v - min_v) / 2.0;
                    s[j] = (max_v - min_v) / 4.0;
                    if (s[j] < 0.01) s[j] = 0.01;
                    continue;
                }

                mean /= Nk;
                for (int i = 0; i < n; i++) {
                    double val = empiricData(data, i);
                    var += gamma[i * k + j] * (val - mean) * (val - mean);
                }
                var /= Nk;

                w[j] = Nk / n;
                m[j] = mean;
                s[j] = sqrt(var);
                if (s[j] < 0.01) s[j] = 0.01;
            }
        }

        double ll = 0.0;
        for (int i = 0; i < n; i++) {
            double val = empiricData(data, i);
            double sum = 0.0;
            for (int j = 0; j < k; j++) sum += w[j] * normalDensity(val, m[j], s[j]);
            if (sum < EPS) sum = EPS;
            ll += log(sum);
        }
        free(gamma);

        int params = 3 * k - 1;
        double bic = -2.0 * ll + params * log((double)n);
        double aic = -2.0 * ll + 2.0 * params;

        if (bic == bic && bic < best_bic) {
            best_bic = bic;
            found = 1;
            result->components_count = k;
            result->bic = bic;
            result->aic = aic;
            result->log_likelihood = ll;
            for (int j = 0; j < k; j++) {
                result->weights[j] = w[j];
                result->means[j] = m[j];
                result->sigmas[j] = s[j];
            }
        }
    }
    return found;
}

// ИСТИННЫЙ РОБАСТНЫЙ АЛГОРИТМ (Шум глубоко интегрирован в E-M шаги)
int robustMixtureBuilder(struct Empiric* data, int n, int min_k, int max_k, int max_iter, EMResult* result, double* unif_w, double* unif_min, double* unif_max) {
    if (n <= 0 || !result) return 0;

    // Рамки равномерного шума всегда остаются по абсолютному минимуму и максимуму
    double abs_min = empiricData(data, 0), abs_max = abs_min;
    for (int i = 1; i < n; i++) {
        double v = empiricData(data, i);
        if (v < abs_min) abs_min = v;
        if (v > abs_max) abs_max = v;
    }
    double scale = (abs_max - abs_min);
    if (scale < EPS) scale = 1.0;

    *unif_min = abs_min;
    *unif_max = abs_max;
    double unif_density = 1.0 / scale;

    double best_bic = DBL_MAX;
    int found = 0;

    for (int k = min_k; k <= max_k; k++) {
        double w[MAX_K], m[MAX_K], s[MAX_K];
        double tmp_min, tmp_max;

        // Гауссианы инициализируются в центре, игнорируя шум
        initParams(data, n, k, w, m, s, &tmp_min, &tmp_max);

        double current_unif_w = 0.1; // Начинаем с гипотезы: 10% шума
        for (int j = 0; j < k; j++) w[j] = (1.0 - current_unif_w) / k;

        // Матрица gamma расширяется на 1 столбец для шумовой компоненты
        double* gamma = (double*)malloc(sizeof(double) * n * (k + 1));
        if (!gamma) return 0;

        for (int iter = 0; iter < max_iter; iter++) {

            // E-ШАГ: Вычисление конкуренции Гауссиан и Шума
            for (int i = 0; i < n; i++) {
                double val = empiricData(data, i);
                double sum = current_unif_w * unif_density; // Доля шума
                gamma[i * (k + 1) + k] = sum;

                for (int j = 0; j < k; j++) {
                    gamma[i * (k + 1) + j] = w[j] * normalDensity(val, m[j], s[j]);
                    sum += gamma[i * (k + 1) + j];
                }

                if (sum < EPS) {
                    for (int j = 0; j < k; j++) gamma[i * (k + 1) + j] = (1.0 - current_unif_w) / k;
                    gamma[i * (k + 1) + k] = current_unif_w;
                }
                else {
                    for (int j = 0; j <= k; j++) gamma[i * (k + 1) + j] /= sum;
                }
            }

            // M-ШАГ: Пересчет весов и параметров
            double N_u = 0.0;
            for (int i = 0; i < n; i++) N_u += gamma[i * (k + 1) + k];
            current_unif_w = N_u / n; // Обновляем глобальный вес шума

            for (int j = 0; j < k; j++) {
                double Nk = 0.0, mean = 0.0, var = 0.0;
                for (int i = 0; i < n; i++) {
                    Nk += gamma[i * (k + 1) + j];
                    mean += gamma[i * (k + 1) + j] * empiricData(data, i);
                }

                if (Nk < EPS) {
                    w[j] = EPS;
                    m[j] = tmp_min + (tmp_max - tmp_min) / 2.0;
                    s[j] = (tmp_max - tmp_min) / 4.0;
                    if (s[j] < 0.01) s[j] = 0.01;
                    continue;
                }

                mean /= Nk;
                for (int i = 0; i < n; i++) {
                    double val = empiricData(data, i);
                    var += gamma[i * (k + 1) + j] * (val - mean) * (val - mean);
                }
                var /= Nk;

                w[j] = Nk / n;
                m[j] = mean;
                s[j] = sqrt(var);
                if (s[j] < 0.01) s[j] = 0.01;
            }
        }

        double ll = 0.0;
        for (int i = 0; i < n; i++) {
            double val = empiricData(data, i);
            double sum = current_unif_w * unif_density;
            for (int j = 0; j < k; j++) sum += w[j] * normalDensity(val, m[j], s[j]);
            if (sum < EPS) sum = EPS;
            ll += log(sum);
        }
        free(gamma);

        // Расчет BIC с учетом дополнительного параметра (веса шума)
        int params = 3 * k;
        double bic = -2.0 * ll + params * log((double)n);
        double aic = -2.0 * ll + 2.0 * params;

        if (bic == bic && bic < best_bic) {
            best_bic = bic;
            found = 1;
            result->components_count = k;
            result->bic = bic;
            result->aic = aic;
            result->log_likelihood = ll;
            *unif_w = current_unif_w;

            // Нормируем веса Гауссиан для передачи в C++ Конверт
            double sum_norm_w = 0.0;
            for (int j = 0; j < k; j++) sum_norm_w += w[j];

            for (int j = 0; j < k; j++) {
                result->weights[j] = (sum_norm_w > EPS) ? w[j] / sum_norm_w : 1.0 / k;
                result->means[j] = m[j];
                result->sigmas[j] = s[j];
            }
        }
    }
    return found;
}