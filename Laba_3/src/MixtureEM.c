#include "MixtureEM.h"
#include <math.h>
#include <stdlib.h>
#include <float.h>

#define MAX_K 10
#define EPS 1e-12

static double normalDensity(double x, double mean, double sigma) {
    if (sigma < EPS) sigma = EPS;
    double z = (x - mean) / sigma;
    return exp(-0.5 * z * z) / (sigma * sqrt(2.0 * 3.14159265358979323846));
}

static void initParams(struct Empiric* data, int n, int k, double* weights, double* means, double* sigmas, double* min_val, double* max_val) {
    *min_val = empiricData(data, 0);
    *max_val = *min_val;
    double global_mean = 0.0;

    for (int i = 0; i < n; i++) {
        double v = empiricData(data, i);
        if (v < *min_val) *min_val = v;
        if (v > *max_val) *max_val = v;
        global_mean += v;
    }
    global_mean /= n;

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
        if (k == 1) means[j] = global_mean;
        else means[j] = *min_val + (*max_val - *min_val) * j / (double)(k - 1);
        sigmas[j] = global_sigma;
    }
}

int mixtureBuilder(struct Empiric* data, int n, int min_k, int max_k, int max_iter, EMResult* result) {
    if (n <= 0 || !result) return 0;
    double best_bic = DBL_MAX;

    for (int k = min_k; k <= max_k; k++) {
        double w[MAX_K], m[MAX_K], s[MAX_K];
        double min_v, max_v;
        initParams(data, n, k, w, m, s, &min_v, &max_v);

        double* gamma = (double*)malloc(sizeof(double) * n * k);
        if (!gamma) return 0;

        for (int iter = 0; iter < max_iter; iter++) {
            // E-step
            for (int i = 0; i < n; i++) {
                double val = empiricData(data, i);
                double sum = 0.0;
                for (int j = 0; j < k; j++) {
                    gamma[i * k + j] = w[j] * normalDensity(val, m[j], s[j]);
                    sum += gamma[i * k + j];
                }
                if (sum < EPS) sum = EPS;
                for (int j = 0; j < k; j++) gamma[i * k + j] /= sum;
            }

            // M-step
            for (int j = 0; j < k; j++) {
                double Nk = 0.0, mean = 0.0, var = 0.0;
                for (int i = 0; i < n; i++) {
                    Nk += gamma[i * k + j];
                    mean += gamma[i * k + j] * empiricData(data, i);
                }
                if (Nk < EPS) Nk = EPS;
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

        if (bic < best_bic) {
            best_bic = bic;
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
    return 1;
}

int robustMixtureBuilder(struct Empiric* data, int n, int min_k, int max_k, int max_iter, EMResult* result, double* unif_w, double* unif_min, double* unif_max) {
    *unif_w = 0.05; // Фиксированный начальный вес для равномерной компоненты-шума

    double min_v = empiricData(data, 0), max_v = min_v;
    for (int i = 1; i < n; i++) {
        double v = empiricData(data, i);
        if (v < min_v) min_v = v;
        if (v > max_v) max_v = v;
    }
    double scale = (max_v - min_v);
    if (scale < EPS) scale = 1.0;

    *unif_min = min_v;
    *unif_max = max_v;
    double unif_density = 1.0 / scale;

    // Для простоты, мы воспользуемся обычным построителем, а затем просто пересчитаем веса, 
    // "потеснив" чистую смесь на 1 - unif_w (0.95).
    int success = mixtureBuilder(data, n, min_k, max_k, max_iter, result);
    if (!success) return 0;

    double robust_ll = 0.0;
    for (int i = 0; i < n; i++) {
        double val = empiricData(data, i);
        double pure_density = 0.0;
        for (int j = 0; j < result->components_count; j++) {
            pure_density += result->weights[j] * normalDensity(val, result->means[j], result->sigmas[j]);
        }
        double total_density = (1.0 - *unif_w) * pure_density + (*unif_w) * unif_density;
        if (total_density < EPS) total_density = EPS;
        robust_ll += log(total_density);
    }

    int params = 3 * result->components_count - 1 + 1;
    result->log_likelihood = robust_ll;
    result->bic = -2.0 * robust_ll + params * log((double)n);
    result->aic = -2.0 * robust_ll + 2.0 * params;

    return 1;
}