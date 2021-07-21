#include <app_resource_manager.h>
#include <app_preference.h>
#include <stdarg.h>
#include "main.h"
#include "util.h"

// Gets an image from the application resource path and sets it as content for an Evas_Object
void image_content_set(Evas_Object *parent, char *file_name)
{
    Evas_Object *icon;
    icon = elm_image_add(parent);
    char *icon_path = NULL;
    app_resource_manager_get(APP_RESOURCE_TYPE_IMAGE, file_name, &icon_path);
    elm_image_file_set(icon, icon_path, NULL);
    elm_object_part_content_set(parent, "elm.swallow.content", icon);
    evas_object_show(icon);
    free(icon_path);
}

// Update value of preference with a char-array
bool set_preference_char(const char *key, char *value)
{
    int ret;
    // Set the value of preference 'key' to a string 'value'
    ret = preference_set_string(key, value);
    if (ret < 0)
        return false;
    return true;
}

// Get the value of a preference with a char-array
bool get_preference_char(const char *key, char **output)
{
    // Ensure variable exists
    bool exists;
    preference_is_existing(key, &exists);
    if (!exists)
        return false;
    // Get value of variable
    int ret;
    ret = preference_get_string(key, output);
    if (ret < 0)
        return false;
    return true;
}

// Free resources after getting preference data
void free_preference(void *data)
{
    free(data);
}

// Create a new string using printf formatting
// Return value must be freed
char *new_string_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Eina_Strbuf *strbuf = eina_strbuf_new();
    char *new_char = NULL;
    eina_strbuf_append_vprintf(strbuf, fmt, args);
    new_char = eina_strbuf_string_steal(strbuf);
    eina_strbuf_free(strbuf);
    va_end(args);
    return new_char;
}

// Free resources from Eina_Array
void eina_array_cleanup(Eina_Array *array)
{
    // Free elements inside array
    while (eina_array_count(array))
        free(eina_array_pop(array));
    // Free array
    eina_array_free(array);
}

// Create new formatted string based on balance
// Returned value must be freed
static char *balance_str_fmt(char *symbol, double bal, double max_with_decimal, double max_non_scientific, int type)
{
    Eina_Strbuf *strbuf = eina_strbuf_new();
    char *str = NULL;
    char *fmt_str = NULL;
    // Draw decimals
    if (bal < max_with_decimal)
    {
        switch (type)
        {
        case (TYPE_FIAT):
            fmt_str = "%.2f";
            break;
        case (TYPE_COIN):
            fmt_str = "%.6f";
            break;
        }
    }
    // Don't draw decimals if too many non-decimal digits present
    else if (bal < max_non_scientific)
        fmt_str = "%.0f";
    // Draw in scientific notation
    else
        fmt_str = "%.2e";
    // Generate output string
    if (type == TYPE_FIAT)
        eina_strbuf_append(strbuf, symbol);
    eina_strbuf_append_printf(strbuf, fmt_str, bal);
    if (type == TYPE_COIN)
        eina_strbuf_append_printf(strbuf, " %s", symbol);
    str = eina_strbuf_string_steal(strbuf);
    // Cleanup
    eina_strbuf_free(strbuf);
    return str;
}

// Create new formatted string based on fiat balance
// Returned value must be freed
char *d_balance_str_fmt(char *symbol, double bal, double max_with_decimal, double max_non_scientific)
{
    char *str = balance_str_fmt(symbol, bal, max_with_decimal, max_non_scientific, TYPE_FIAT);
    return str;
}

// Create new formatted string based on coin balance
// Returned value must be freed
char *c_balance_str_fmt(char *symbol, double bal, double max_with_decimal, double max_non_scientific)
{
    char *str = balance_str_fmt(symbol, bal, max_with_decimal, max_non_scientific, TYPE_COIN);
    return str;
}

// Create new formatted string based on balance, replacing larger numbers with suffixes
// Returned value must be freed
static char *balance_str_fmt_suffix(char *symbol, double bal, int type)
{
    Eina_Strbuf *strbuf = eina_strbuf_new();
    char *str = NULL;
    // Suffixes to add after balance
    static char *suffixes = "kmbtqQsSond";
    // Add fiat symbol
    if (type == TYPE_FIAT)
        eina_strbuf_append(strbuf, symbol);
    // Don't modify if less than 1 thousand
    if (bal < 1000.0)
        eina_strbuf_append_printf(strbuf, "%f", bal);
    else
    {
        // Base 10 exponent e.g 6 for millions, 7 for tens of millions etc.
        int exp = (int)trunc(log10(bal));
        // Index exponent in range, e.g same for 1, 10, and 100 million
        int newexp = (int)floor(exp / 3.0);
        // Get suffix corresponding to thousands, millions, billions, etc.
        char suffix = suffixes[newexp - 1];
        // Balance rounded to range
        double newbal = bal / (pow(1000.0, newexp));
        // Add formatted balance with suffix to buffer
        eina_strbuf_append_printf(strbuf, "%.2f%c", newbal, suffix);
    }
    if (type == TYPE_COIN)
        eina_strbuf_append_printf(strbuf, " %s", symbol);
    str = eina_strbuf_string_steal(strbuf);
    // Cleanup
    eina_strbuf_free(strbuf);
    return str;
}

// Create new formatted string based on coin balance, replacing larger numbers with suffixes
// Returned value must be freed
char *c_balance_str_fmt_suffix(char *symbol, double bal)
{
    char *str = balance_str_fmt_suffix(symbol, bal, TYPE_COIN);
    return str;
}

// Get current fiat symbol
char *fiat_symbol()
{
    appdata_s *ad = get_appdata(NULL);
    // Get currency symbol
    FiatData *fiat_data = NULL;
    size_t size;
    eina_lock_take(&ad->fiat_data_mutex);
    bundle_get_byte(ad->fiat_data, ad->currency, (void **)&fiat_data, &size);
    eina_lock_release(&ad->fiat_data_mutex);
    // Return pointer to symbol - must not be freed
    return fiat_data->symbol;
}

// Get unix timestamp in milliseconds
unsigned long long get_unix_time_ms()
{
    return ecore_time_unix_get() * 1000000.0;
}

// Convert unix timestamp to formatted string
// Returned value must be freed
char *unix_time_fmt(time_t time)
{
    char buf[20];
    strftime(buf, sizeof(buf), "%d %a %Y %H:%M", localtime(&time));
    return strdup(buf);
}

// Get how long ago (in hours) 'then' happened from 'now' (both must be in seconds)
// Returned value must be freed
char *unix_hours_ago(time_t then)
{
    time_t now = (time_t)ecore_time_unix_get();
    double diff = difftime(now, then);
    double hh = diff / 3600.0;
    char *str;
    if (hh < 1.0)
        str = strdup("Just now");
    else
        str = new_string_printf("%.1f hours ago", hh);
    return str;
}
