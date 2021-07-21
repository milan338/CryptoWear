#ifndef UTIL_H
#define UTIL_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define __RGB(R, G, B) R, G, B
#define __RGBA(R, G, B, A) R, G, B, A

#define RGB_NORM(R, G, B) __RGB((double)R / 255.0, (double)G / 255.0, (double)B / 255.0)
#define RGBA_NORM(R, G, B, A) __RGBA((double)R / 255.0, (double)G / 255.0, (double)B / 255.0, (double)A / 255.0)

#define COLOR_DECREASE_RGB RGB_NORM(178, 34, 34)
#define COLOR_DECREASE_RGBA RGBA_NORM(178, 34, 34, 255)

#define COLOR_INCREASE_RGB RGB_NORM(0, 128, 0)
#define COLOR_INCREASE_RGBA RGBA_NORM(0, 128, 0, 255)

#define COLOR_SAME_RGB RGB_NORM(255, 255, 255)
#define COLOR_SAME_RGBA RGBA_NORM(255, 255, 255, 255)

#define COLOR_SECONDARY_RGB RGB_NORM(65, 65, 65, 255)
#define COLOR_SECONDARY_RGBA RGBA_NORM(65, 65, 65, 255)

#define TYPE_FIAT 0
#define TYPE_COIN 1

typedef struct CoinData
{
    char *symbol;
    double c_balance;
    double d_balance;
} CoinData;

typedef struct ExchangeData
{
    int exchange;
    bundle *coin_data_dict;
    Eina_Array *coin_data_sorted;
} ExchangeData;

typedef struct FiatData
{
    char *name;
    char *symbol;
} FiatData;

typedef struct CoinPriceData
{
    double price;
    double high;
    double low;
    double open;
    double volume;
    double percent_change;
    double absolute_change;
} CoinPriceData;

typedef struct CoinNewsData
{
    int published_on;
    char *source;
    char *url;
    char *title;
    char *body;
} CoinNewsData;

typedef struct CoinNews
{
    CoinNewsData **news_arr;
    size_t size;
} CoinNews;

void image_content_set(Evas_Object *parent, char *file_name);

bool set_preference_char(const char *key, char *value);
bool get_preference_char(const char *key, char **output);
void free_preference(void *data);

char *new_string_printf(const char *fmt, ...);

void eina_array_cleanup(Eina_Array *array);

char *d_balance_str_fmt(char *symbol, double bal, double max_with_decimal, double max_non_scientific);
char *c_balance_str_fmt(char *symbol, double bal, double max_with_decimal, double max_non_scientific);

char *c_balance_str_fmt_suffix(char *symbol, double bal);

char *fiat_symbol();

unsigned long long get_unix_time_ms();
char *unix_time_fmt(time_t time);
char *unix_hours_ago(time_t then);

#endif // UTIL_H
