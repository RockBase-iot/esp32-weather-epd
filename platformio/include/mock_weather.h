/* Local OpenWeatherMap sample data declarations for esp32-weather-epd.
 * Copyright (C) 2026  Chengdu RockBase Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef __MOCK_WEATHER_H__
#define __MOCK_WEATHER_H__

#include "api_response.h"

void loadMockOpenWeatherData(owm_resp_onecall_t &onecall,
                             owm_resp_air_pollution_t &air_pollution);

#endif
