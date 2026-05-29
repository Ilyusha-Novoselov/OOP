#ifndef _Laba3_MixtureEM_Header
#define _Laba3_MixtureEM_Header

#ifdef __cplusplus
extern "C" {
#endif

struct Empiric;

// Интерфейсная функция для доступа к данным (будет определена в C++)
extern double empiricData(struct Empiric* e, int i);

typedef struct {
    int components_count;
    double weights[10];
    double means[10];
    double sigmas[10];
    double bic;
    double aic;
    double log_likelihood;
} EMResult;

// Главная функция неробастного построителя
int mixtureBuilder(struct Empiric* e, int n, int min_k, int max_k, int max_iter, EMResult* result);

// Главная функция робастного построителя
int robustMixtureBuilder(struct Empiric* e, int n, int min_k, int max_k, int max_iter, EMResult* result, double* unif_w, double* unif_min, double* unif_max);

#ifdef __cplusplus
}
#endif

#endif