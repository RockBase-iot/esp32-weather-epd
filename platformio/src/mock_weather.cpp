/* Local OpenWeatherMap sample data for esp32-weather-epd.
 * Copyright (C) 2026  Chengdu RockBase Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "mock_weather.h"

#include "config.h"

#ifdef USE_OPENWEATHER_MOCK_DATA

#include <Arduino.h>
#include <algorithm>
#include <cmath>
#include <ctime>
namespace
{
constexpr float DEG_C_TO_K = 273.15f;
constexpr int64_t ONE_HOUR = 3600;
constexpr int64_t ONE_DAY = 86400;

struct WeatherSample
{
  int id;
  const char *main;
  const char *description;
  const char *icon;
};

const WeatherSample DAILY_WEATHER[] =
{
  {800, "Clear",        "clear sky",        "01d"},
  {801, "Clouds",       "few clouds",       "02d"},
  {500, "Rain",         "light rain",       "10d"},
  {803, "Clouds",       "broken clouds",    "04d"},
  {802, "Clouds",       "scattered clouds", "03d"},
  {501, "Rain",         "moderate rain",    "10d"},
  {800, "Clear",        "clear sky",        "01d"},
  {804, "Clouds",       "overcast clouds",  "04d"},
};

const WeatherSample NIGHT_WEATHER =
{
  800, "Clear", "clear sky", "01n"
};

void setWeather(owm_weather_t &weather, const WeatherSample &sample)
{
  weather.id = sample.id;
  weather.main = sample.main;
  weather.description = sample.description;
  weather.icon = sample.icon;
}

int64_t dayStart(int64_t now)
{
  time_t ts = static_cast<time_t>(now);
  tm local = {};
  localtime_r(&ts, &local);
  local.tm_hour = 0;
  local.tm_min = 0;
  local.tm_sec = 0;
  return static_cast<int64_t>(mktime(&local));
}

void loadMockOneCall(owm_resp_onecall_t &r, int64_t now)
{
  const int64_t today = dayStart(now);

  r.lat = LAT.toFloat();
  r.lon = LON.toFloat();
  r.timezone = TIMEZONE;
  r.timezone_offset = 0;
  r.alerts.clear();

  r.current.dt = now;
  r.current.sunrise = today + 6 * ONE_HOUR + 10 * 60;
  r.current.sunset = today + 20 * ONE_HOUR + 18 * 60;
  r.current.temp = DEG_C_TO_K + 24.5f;
  r.current.feels_like = DEG_C_TO_K + 25.1f;
  r.current.pressure = 1014;
  r.current.humidity = 56;
  r.current.dew_point = DEG_C_TO_K + 15.2f;
  r.current.clouds = 35;
  r.current.uvi = 5.4f;
  r.current.visibility = 10000;
  r.current.wind_speed = 4.8f;
  r.current.wind_gust = 8.2f;
  r.current.wind_deg = 225;
  r.current.rain_1h = 0.0f;
  r.current.snow_1h = 0.0f;
  setWeather(r.current.weather, DAILY_WEATHER[1]);

  for (int i = 0; i < OWM_NUM_DAILY; ++i)
  {
    const int64_t day = today + i * ONE_DAY;
    const float dayTempC = 24.0f + std::sin(i * 0.9f) * 3.0f;
    const float nightTempC = 16.0f + std::cos(i * 0.8f) * 2.0f;
    const float pop = (i == 2 || i == 5) ? 0.65f : (i == 1 ? 0.25f : 0.05f);
    const WeatherSample &sample = DAILY_WEATHER[i % OWM_NUM_DAILY];

    r.daily[i].dt = day + 12 * ONE_HOUR;
    r.daily[i].sunrise = day + 6 * ONE_HOUR + (10 + i) * 60;
    r.daily[i].sunset = day + 20 * ONE_HOUR + (18 - i) * 60;
    r.daily[i].moonrise = day + (21 + (i % 3)) * ONE_HOUR;
    r.daily[i].moonset = day + (7 + (i % 2)) * ONE_HOUR;
    r.daily[i].moon_phase = std::min(0.95f, 0.12f + i * 0.1f);
    r.daily[i].temp.morn = DEG_C_TO_K + nightTempC + 2.0f;
    r.daily[i].temp.day = DEG_C_TO_K + dayTempC;
    r.daily[i].temp.eve = DEG_C_TO_K + dayTempC - 2.5f;
    r.daily[i].temp.night = DEG_C_TO_K + nightTempC;
    r.daily[i].temp.min = DEG_C_TO_K + nightTempC - 1.0f;
    r.daily[i].temp.max = DEG_C_TO_K + dayTempC + 2.0f;
    r.daily[i].feels_like.morn = r.daily[i].temp.morn + 0.4f;
    r.daily[i].feels_like.day = r.daily[i].temp.day + 0.7f;
    r.daily[i].feels_like.eve = r.daily[i].temp.eve + 0.2f;
    r.daily[i].feels_like.night = r.daily[i].temp.night;
    r.daily[i].pressure = 1012 + (i % 4);
    r.daily[i].humidity = 52 + i * 3;
    r.daily[i].dew_point = DEG_C_TO_K + 14.0f + i * 0.4f;
    r.daily[i].clouds = (i * 17 + 25) % 100;
    r.daily[i].uvi = std::max(1.0f, 6.0f - i * 0.45f);
    r.daily[i].visibility = 10000;
    r.daily[i].wind_speed = 3.5f + i * 0.35f;
    r.daily[i].wind_gust = 6.0f + i * 0.55f;
    r.daily[i].wind_deg = (210 + i * 22) % 360;
    r.daily[i].pop = pop;
    r.daily[i].rain = pop > 0.3f ? 2.5f + i : 0.0f;
    r.daily[i].snow = 0.0f;
    setWeather(r.daily[i].weather, sample);
  }

  for (int i = 0; i < OWM_NUM_HOURLY; ++i)
  {
    const int64_t hour = now + i * ONE_HOUR;
    const int hourOfDay = (static_cast<int>((hour - today) / ONE_HOUR) % 24);
    const float cycle = std::sin((hourOfDay - 7) * 3.14159f / 12.0f);
    const float tempC = 21.0f + cycle * 4.5f + std::sin(i * 0.35f);
    const bool daytime = hourOfDay >= 6 && hourOfDay < 20;
    const bool rainy = (i >= 14 && i <= 18) || (i >= 35 && i <= 38);
    const WeatherSample sample = rainy
                               ? WeatherSample{500, "Rain", "light rain", "10d"}
                               : (daytime ? DAILY_WEATHER[(i / 8) % 4]
                                          : NIGHT_WEATHER);

    r.hourly[i].dt = hour;
    r.hourly[i].temp = DEG_C_TO_K + tempC;
    r.hourly[i].feels_like = DEG_C_TO_K + tempC + 0.5f;
    r.hourly[i].pressure = 1011 + (i % 5);
    r.hourly[i].humidity = 50 + (i % 18);
    r.hourly[i].dew_point = DEG_C_TO_K + 14.0f + std::sin(i * 0.4f) * 2.0f;
    r.hourly[i].clouds = rainy ? 88 : ((i * 9) % 80);
    r.hourly[i].uvi = daytime ? std::max(0.0f, 6.2f * cycle) : 0.0f;
    r.hourly[i].visibility = rainy ? 8500 : 10000;
    r.hourly[i].wind_speed = 3.0f + std::sin(i * 0.3f) * 1.6f;
    r.hourly[i].wind_gust = r.hourly[i].wind_speed + 2.5f;
    r.hourly[i].wind_deg = (200 + i * 7) % 360;
    r.hourly[i].pop = rainy ? 0.75f : (i % 9 == 0 ? 0.2f : 0.0f);
    r.hourly[i].rain_1h = rainy ? 0.8f : 0.0f;
    r.hourly[i].snow_1h = 0.0f;
    setWeather(r.hourly[i].weather, sample);
  }

}

void loadMockAirPollution(owm_resp_air_pollution_t &r, int64_t now)
{
  r.coord.lat = LAT.toFloat();
  r.coord.lon = LON.toFloat();

  for (int i = 0; i < OWM_NUM_AIR_POLLUTION; ++i)
  {
    r.main_aqi[i] = 2;
    r.dt[i] = now - (OWM_NUM_AIR_POLLUTION - 1 - i) * ONE_HOUR;
    r.components.co[i] = 210.0f + i * 1.5f;
    r.components.no[i] = 0.4f + i * 0.01f;
    r.components.no2[i] = 8.0f + std::sin(i * 0.4f) * 2.0f;
    r.components.o3[i] = 55.0f + std::cos(i * 0.25f) * 8.0f;
    r.components.so2[i] = 3.0f + std::sin(i * 0.2f);
    r.components.pm2_5[i] = 9.0f + std::sin(i * 0.5f) * 3.0f;
    r.components.pm10[i] = 18.0f + std::cos(i * 0.3f) * 4.0f;
    r.components.nh3[i] = 1.2f + i * 0.02f;
  }
}
} // namespace

void loadMockOpenWeatherData(owm_resp_onecall_t &onecall,
                             owm_resp_air_pollution_t &air_pollution)
{
  time_t currentTime = time(nullptr);
  int64_t now = static_cast<int64_t>(currentTime);
  if (now < 1700000000)
  {
    now = 1719835200; // 2024-07-01 12:00:00 UTC
  }

  loadMockOneCall(onecall, now);
  loadMockAirPollution(air_pollution, now);
}

#endif
