/**
 * The CC0 1.0 Universal is applied to this work.
 *
 * To the extent possible under law, Matt Hayes and Jake LeMaster have
 * waived all copyright and related or neighboring rights to donky.
 * This work is published from the United States.
 *
 * Please see the copy of the CC0 included with this program for complete
 * information including limitations and disclaimers. If no such copy
 * exists, see <http://creativecommons.org/publicdomain/zero/1.0/legalcode>.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../cfg.h"
#include "../mem.h"
#include "../module.h"
#include "../util.h"

#define ma(a, b, c, d, e) module_var_add(a, b, c, d, e)
#define vs VARIABLE_STR
#define as ARGSTR
#define p "weather_"

char module_name[] = "weather";

/* Globals. */
static pthread_t weather_thread_id;

struct forecast_information {
        char city[64];
        char postal_code[16];
        char forecast_date[16];
        char current_date_time[32];
        char unit_system[8];
};

struct current_conditions {
        char condition[32];
        char temp_f[8];
        char temp_c[8];
        char humidity[32];
        char wind_condition[32];
};

struct forecast_conditions {
        char low[8];
        char high[8];
        char condition[32];
};

struct weather_data {
        struct forecast_information fi;
        struct current_conditions cc;
        struct forecast_conditions sun;
        struct forecast_conditions mon;
        struct forecast_conditions tue;
        struct forecast_conditions wed;
        struct forecast_conditions thu;
        struct forecast_conditions fri;
        struct forecast_conditions sat;
} wd;

/**
 * @brief This is run on module initialization.
 */
void module_init(const struct module *mod)
{
        /* forecast information */
        ma(mod, p "city", "get_city", 120.0, vs | as);
        ma(mod, p "postal_code", "get_postal_code", 120.0, vs | as);
        ma(mod, p "forecast_date", "get_forecast_date", 120.0, vs | as);
        ma(mod, p "current_date_time", "get_current_date_time", 120.0, vs | as);
        ma(mod, p "unit_system", "get_unit_system", 120.0, vs | as);

        /* current conditions */
        ma(mod, p "cur_condition", "get_cur_condition", 120.0, vs | as);
        ma(mod, p "cur_temp_f", "get_cur_temp_f", 120.0, vs | as);
        ma(mod, p "cur_temp_c", "get_cur_temp_c", 120.0, vs | as);
        ma(mod, p "cur_humidity", "get_cur_humidity", 120.0, vs | as);
        ma(mod, p "cur_wind_condition", "get_cur_wind_condition", 120.0, vs | as);

        /* forecast conditions */
        ma(mod, p "sun_low", "get_sun_low", 120.0, vs | as);
        ma(mod, p "sun_high", "get_sun_high", 120.0, vs | as);
        ma(mod, p "sun_condition", "get_sun_condition", 120.0, vs | as);

        ma(mod, p "mon_low", "get_mon_low", 120.0, vs | as);
        ma(mod, p "mon_high", "get_mon_high", 120.0, vs | as);
        ma(mod, p "mon_condition", "get_mon_condition", 120.0, vs | as);

        ma(mod, p "tue_low", "get_tue_low", 120.0, vs | as);
        ma(mod, p "tue_high", "get_tue_high", 120.0, vs | as);
        ma(mod, p "tue_condition", "get_tue_condition", 120.0, vs | as);

        ma(mod, p "wed_low", "get_wed_low", 120.0, vs | as);
        ma(mod, p "wed_high", "get_wed_high", 120.0, vs | as);
        ma(mod, p "wed_condition", "get_wed_condition", 120.0, vs | as);

        ma(mod, p "thu_low", "get_thu_low", 120.0, vs | as);
        ma(mod, p "thu_high", "get_thu_high", 120.0, vs | as);
        ma(mod, p "thu_condition", "get_thu_condition", 120.0, vs | as);

        ma(mod, p "fri_low", "get_fri_low", 120.0, vs | as);
        ma(mod, p "fri_high", "get_fri_high", 120.0, vs | as);
        ma(mod, p "fri_condition", "get_fri_condition", 120.0, vs | as);

        ma(mod, p "sat_low", "get_sat_low", 120.0, vs | as);
        ma(mod, p "sat_high", "get_sat_high", 120.0, vs | as);
        ma(mod, p "sat_condition", "get_sat_condition", 120.0, vs | as);

        /* Set all this memory to zero!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
        memset(&wd, 0, sizeof(wd));

        /* Launch thread. */
}

/**
 * @brief This is run when the module is about to be unloaded.
 */
void module_destroy(void)
{
}
