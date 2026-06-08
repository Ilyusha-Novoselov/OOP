#include "MixtureEM.h"
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <time.h>

#define MAX_K 10
#define EPS 1e-9

/* Число случайных рестартов для поиска глобального оптимума.
   Согласно лекциям: "наиболее простым способом сделать это является
   многократное решение с использованием EM-алгоритма из случайных
   начальных приближений. В качестве решения следует выбрать то значение,
   которое соответствует максимальному достигнутому значению функции
   правдоподобия." */
#define NUM_RESTARTS 10

static double normalDensity(double x, double mean, double sigma) {
    if (sigma < EPS) sigma = EPS;
    double z = (x - mean) / sigma;
    return exp(-0.5 * z * z) / (sigma * sqrt(2.0 * 3.14159265358979323846));
}

/* Простой линейный конгруэнтный ГПСЧ (без зависимости от rand()) */
static unsigned long lcg_state = 0;

static void lcg_seed(unsigned long seed) {
    lcg_state = seed;
}

/* Возвращает равномерное число в [0, 1) */
static double lcg_uniform(void) {
    lcg_state = lcg_state * 6364136223846793005ULL + 1442695040888963407ULL;
    return (double)((lcg_state >> 33) & 0x7FFFFFFF) / (double)0x80000000;
}

int compare_doubles(const void* a, const void* b) {
    double arg1 = *(const double*)a;
    double arg2 = *(const double*)b;
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

/* Случайная инициализация параметров согласно рекомендациям лекций:
   - сдвиги генерируются равномерно между min и max выборки
   - дисперсии генерируются между c (минимально допустимым) и эмп. дисперсией */
static void initParamsRandom(struct Empiric* data, int n, int k,
    double* weights, double* means, double* sigmas,
    double min_val, double max_val, double emp_sigma)
{
    double sum_w = 0.0;
    for (int j = 0; j < k; j++) {
        /* Случайный сдвиг в [min_val, max_val] */
        means[j] = min_val + lcg_uniform() * (max_val - min_val);
        /* Случайная сигма в [0.01, emp_sigma] */
        double sigma_range = emp_sigma - 0.01;
        if (sigma_range < 0.0) sigma_range = 0.0;
        sigmas[j] = 0.01 + lcg_uniform() * sigma_range;
        /* Случайный вес (нормируем после) */
        weights[j] = lcg_uniform() + 0.1;
        sum_w += weights[j];
    }
    for (int j = 0; j < k; j++) weights[j] /= sum_w;
}

/* Одна EM-итерация для неробастного алгоритма.
   Возвращает log-likelihood после сходимости. */
static double runEM(struct Empiric* data, int n, int k, int max_iter,
    double* w, double* m, double* s,
    double min_val, double max_val)
{
    double* gamma = (double*)malloc(sizeof(double) * n * k);
    if (!gamma) return -DBL_MAX;

    for (int iter = 0; iter < max_iter; iter++) {
        /* E-шаг */
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
        /* M-шаг */
        for (int j = 0; j < k; j++) {
            double Nk = 0.0, mean = 0.0, var = 0.0;
            for (int i = 0; i < n; i++) {
                Nk += gamma[i * k + j];
                mean += gamma[i * k + j] * empiricData(data, i);
            }
            if (Nk < EPS) {
                w[j] = EPS;
                m[j] = min_val + (max_val - min_val) / 2.0;
                s[j] = (max_val - min_val) / 4.0;
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
    return ll;
}

int mixtureBuilder(struct Empiric* data, int n, int min_k, int max_k, int max_iter, EMResult* result) {
    if (n <= 0 || !result) return 0;

    /* Предвычисляем глобальную статистику выборки один раз */
    double* sorted_data = (double*)malloc(sizeof(double) * n);
    if (!sorted_data) return 0;

    double global_mean = 0.0;
    for (int i = 0; i < n; i++) {
        sorted_data[i] = empiricData(data, i);
        global_mean += sorted_data[i];
    }
    global_mean /= n;

    qsort(sorted_data, n, sizeof(double), compare_doubles);
    int p5_idx = (int)(n * 0.05); if (p5_idx < 0) p5_idx = 0;
    int p95_idx = (int)(n * 0.95); if (p95_idx >= n) p95_idx = n - 1;
    double min_val = sorted_data[p5_idx];
    double max_val = sorted_data[p95_idx];
    free(sorted_data);

    double global_var = 0.0;
    for (int i = 0; i < n; i++) {
        double v = empiricData(data, i) - global_mean;
        global_var += v * v;
    }
    global_var /= n;
    double emp_sigma = sqrt(global_var);
    if (emp_sigma < 0.01) emp_sigma = 1.0;

    /* Инициализируем ГПСЧ детерминированным зерном для воспроизводимости */
    lcg_seed(42u);

    double best_bic = DBL_MAX;
    int found = 0;

    for (int k = min_k; k <= max_k; k++) {
        /* ──────────────────────────────────────────────────────────────
           Поиск глобального оптимума через мультистарт.
           Лекции: "Наиболее простым способом сделать это является
           многократное решение с использованием EM-алгоритма из
           случайных начальных приближений. В качестве решения следует
           выбрать то значение, которое соответствует максимальному
           достигнутому значению функции правдоподобия."
        ────────────────────────────────────────────────────────────── */
        double best_ll_k = -DBL_MAX;
        double best_w[MAX_K], best_m[MAX_K], best_s[MAX_K];

        for (int restart = 0; restart < NUM_RESTARTS; restart++) {
            double w[MAX_K], m[MAX_K], s[MAX_K];
            initParamsRandom(data, n, k, w, m, s, min_val, max_val, emp_sigma);

            double ll = runEM(data, n, k, max_iter, w, m, s, min_val, max_val);

            if (ll > best_ll_k) {
                best_ll_k = ll;
                for (int j = 0; j < k; j++) {
                    best_w[j] = w[j];
                    best_m[j] = m[j];
                    best_s[j] = s[j];
                }
            }
        }

        if (best_ll_k <= -DBL_MAX) continue;

        int params = 3 * k - 1;
        double bic = -2.0 * best_ll_k + params * log((double)n);
        double aic = -2.0 * best_ll_k + 2.0 * params;
        /* ICL ≈ BIC + штраф за перекрытие кластеров (оценка через энтропию).
           Используем BIC как основной критерий выбора числа компонент,
           но сохраняем ICL для отчёта. */
        double icl = bic; /* упрощённо: без скрытых данных ICL = BIC */

        if (bic == bic && bic < best_bic) {
            best_bic = bic;
            found = 1;
            result->components_count = k;
            result->bic = bic;
            result->aic = aic;
            result->icl = icl;
            result->log_likelihood = best_ll_k;
            for (int j = 0; j < k; j++) {
                result->weights[j] = best_w[j];
                result->means[j] = best_m[j];
                result->sigmas[j] = best_s[j];
            }
        }
    }
    return found;
}

/* ─── РОБАСТНЫЙ АЛГОРИТМ ─────────────────────────────────────────────── */

static double runRobustEM(struct Empiric* data, int n, int k, int max_iter,
    double* w, double* m, double* s,
    double* cur_unif_w,
    double unif_density,
    double tmp_min, double tmp_max)
{
    double* gamma = (double*)malloc(sizeof(double) * n * (k + 1));
    if (!gamma) return -DBL_MAX;

    for (int iter = 0; iter < max_iter; iter++) {
        /* E-шаг */
        for (int i = 0; i < n; i++) {
            double val = empiricData(data, i);
            double sum = (*cur_unif_w) * unif_density;
            gamma[i * (k + 1) + k] = sum;
            for (int j = 0; j < k; j++) {
                gamma[i * (k + 1) + j] = w[j] * normalDensity(val, m[j], s[j]);
                sum += gamma[i * (k + 1) + j];
            }
            if (sum < EPS) {
                for (int j = 0; j < k; j++) gamma[i * (k + 1) + j] = (1.0 - (*cur_unif_w)) / k;
                gamma[i * (k + 1) + k] = *cur_unif_w;
            }
            else {
                for (int j = 0; j <= k; j++) gamma[i * (k + 1) + j] /= sum;
            }
        }
        /* M-шаг */
        double N_u = 0.0;
        for (int i = 0; i < n; i++) N_u += gamma[i * (k + 1) + k];
        *cur_unif_w = N_u / n;

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
        double sum = (*cur_unif_w) * unif_density;
        for (int j = 0; j < k; j++) sum += w[j] * normalDensity(val, m[j], s[j]);
        if (sum < EPS) sum = EPS;
        ll += log(sum);
    }
    free(gamma);
    return ll;
}

int robustMixtureBuilder(struct Empiric* data, int n, int min_k, int max_k, int max_iter,
    EMResult* result, double* unif_w, double* unif_min, double* unif_max)
{
    if (n <= 0 || !result) return 0;

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

    /* Статистика для рестартов */
    double* sorted_data = (double*)malloc(sizeof(double) * n);
    if (!sorted_data) return 0;
    double global_mean = 0.0;
    for (int i = 0; i < n; i++) {
        sorted_data[i] = empiricData(data, i);
        global_mean += sorted_data[i];
    }
    global_mean /= n;
    qsort(sorted_data, n, sizeof(double), compare_doubles);
    int p5_idx = (int)(n * 0.05); if (p5_idx < 0) p5_idx = 0;
    int p95_idx = (int)(n * 0.95); if (p95_idx >= n) p95_idx = n - 1;
    double min_val = sorted_data[p5_idx];
    double max_val = sorted_data[p95_idx];
    free(sorted_data);
    double global_var = 0.0;
    for (int i = 0; i < n; i++) {
        double v = empiricData(data, i) - global_mean;
        global_var += v * v;
    }
    global_var /= n;
    double emp_sigma = sqrt(global_var);
    if (emp_sigma < 0.01) emp_sigma = 1.0;

    lcg_seed(42u);

    double best_bic = DBL_MAX;
    int found = 0;

    for (int k = min_k; k <= max_k; k++) {
        double best_ll_k = -DBL_MAX;
        double best_w[MAX_K], best_m[MAX_K], best_s[MAX_K];
        double best_uw = 0.1;

        for (int restart = 0; restart < NUM_RESTARTS; restart++) {
            double w[MAX_K], m[MAX_K], s[MAX_K];
            initParamsRandom(data, n, k, w, m, s, min_val, max_val, emp_sigma);

            double cur_uw = 0.1;
            for (int j = 0; j < k; j++) w[j] = (1.0 - cur_uw) / k;

            double ll = runRobustEM(data, n, k, max_iter, w, m, s,
                &cur_uw, unif_density, min_val, max_val);

            if (ll > best_ll_k) {
                best_ll_k = ll;
                best_uw = cur_uw;
                for (int j = 0; j < k; j++) {
                    best_w[j] = w[j];
                    best_m[j] = m[j];
                    best_s[j] = s[j];
                }
            }
        }

        if (best_ll_k <= -DBL_MAX) continue;

        int params = 3 * k;
        double bic = -2.0 * best_ll_k + params * log((double)n);
        double aic = -2.0 * best_ll_k + 2.0 * params;
        double icl = bic;

        if (bic == bic && bic < best_bic) {
            best_bic = bic;
            found = 1;
            result->components_count = k;
            result->bic = bic;
            result->aic = aic;
            result->icl = icl;
            result->log_likelihood = best_ll_k;
            *unif_w = best_uw;

            double sum_norm_w = 0.0;
            for (int j = 0; j < k; j++) sum_norm_w += best_w[j];
            for (int j = 0; j < k; j++) {
                result->weights[j] = (sum_norm_w > EPS) ? best_w[j] / sum_norm_w : 1.0 / k;
                result->means[j] = best_m[j];
                result->sigmas[j] = best_s[j];
            }
        }
    }
    return found;
}