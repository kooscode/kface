// Companion JS: there's no public API for a watchapp to fetch weather
// itself, so this does the geolocation + Open-Meteo HTTP fetch on the
// phone and pushes the result to the watch over AppMessage. Modeled on
// https://github.com/jhatax/at-a-glance's src/pkjs/weather_location.js -
// including its city reverse-geocoding via OpenStreetMap's Nominatim.

var messageKeys = require('message_keys');

var UPDATE_INTERVAL_MS = 15 * 60 * 1000;
var GEO_TIMEOUT_MS = 2 * 60 * 1000;
var GEO_MAX_AGE_MS = 30 * 60 * 1000;
var MAX_CITY_STRING_LENGTH = 18;

function sendWeather(tempCelsius, weatherCode, isDay) {
  var payload = {};
  // Sent as tenths of a degree so the watch side can stay integer-only.
  payload[messageKeys.TEMPERATURE] = Math.round(tempCelsius * 10);
  payload[messageKeys.WEATHER_CODE] = weatherCode;
  payload[messageKeys.IS_DAY] = isDay ? 1 : 0;

  Pebble.sendAppMessage(payload, function () {
    console.log('kface: weather sent');
  }, function (e) {
    console.log('kface: weather send failed: ' + JSON.stringify(e));
  });
}

function sendCity(city) {
  var payload = {};
  payload[messageKeys.CITY] = city;

  Pebble.sendAppMessage(payload, function () {
    console.log('kface: city sent');
  }, function (e) {
    console.log('kface: city send failed: ' + JSON.stringify(e));
  });
}

function fetchWeather(latitude, longitude) {
  var url = 'https://api.open-meteo.com/v1/forecast?latitude=' + latitude +
    '&longitude=' + longitude +
    '&current=temperature_2m,weather_code,is_day&temperature_unit=celsius';

  var xhr = new XMLHttpRequest();
  xhr.open('GET', url, true);
  xhr.timeout = GEO_TIMEOUT_MS;
  xhr.onload = function () {
    if (xhr.status !== 200) {
      console.log('kface: weather fetch failed, status=' + xhr.status);
      return;
    }

    try {
      var data = JSON.parse(xhr.responseText);
      if (data.current &&
          typeof data.current.temperature_2m === 'number' &&
          typeof data.current.weather_code === 'number' &&
          typeof data.current.is_day === 'number') {
        sendWeather(data.current.temperature_2m, data.current.weather_code, data.current.is_day === 1);
      }
    } catch (e) {
      console.log('kface: weather parse error: ' + JSON.stringify(e));
    }
  };
  xhr.onerror = function () {
    console.log('kface: weather fetch network error');
  };
  xhr.ontimeout = function () {
    console.log('kface: weather fetch timed out');
  };
  xhr.send();
}

function fetchCity(latitude, longitude) {
  var url = 'https://nominatim.openstreetmap.org/reverse?format=json&lat=' +
    latitude + '&lon=' + longitude;

  var xhr = new XMLHttpRequest();
  xhr.open('GET', url, true);
  try {
    // 'User-Agent' is a forbidden header in some XHR implementations and
    // can throw synchronously - don't let that take the whole fetch down.
    xhr.setRequestHeader('User-Agent', 'PebbleWatchFace-kface');
  } catch (e) {
    console.log('kface: city setRequestHeader failed: ' + JSON.stringify(e));
  }
  xhr.timeout = GEO_TIMEOUT_MS;
  xhr.onload = function () {
    if (xhr.status !== 200) {
      console.log('kface: city fetch failed, status=' + xhr.status);
      return;
    }

    try {
      var data = JSON.parse(xhr.responseText);
      // Rural coordinates often have no city/town/village in the address -
      // fall back to progressively broader regions rather than showing
      // nothing.
      var city = (
        data.address.city ||
        data.address.town ||
        data.address.village ||
        data.address.county ||
        data.address.state ||
        ''
      ).slice(0, MAX_CITY_STRING_LENGTH);
      if (city) {
        sendCity(city);
      } else {
        console.log('kface: city fetch had no city/town/village: ' + xhr.responseText);
      }
    } catch (e) {
      console.log('kface: city parse error: ' + JSON.stringify(e));
    }
  };
  xhr.onerror = function () {
    console.log('kface: city fetch network error');
  };
  xhr.ontimeout = function () {
    console.log('kface: city fetch timed out');
  };
  xhr.send();
}

function refreshWeather() {
  if (typeof navigator === 'undefined' || !navigator.geolocation) {
    console.log('kface: geolocation unavailable');
    return;
  }

  navigator.geolocation.getCurrentPosition(
    function (pos) {
      fetchWeather(pos.coords.latitude, pos.coords.longitude);
      fetchCity(pos.coords.latitude, pos.coords.longitude);
    },
    function (err) {
      console.log('kface: geolocation error: ' + JSON.stringify(err));
    },
    { timeout: GEO_TIMEOUT_MS, maximumAge: GEO_MAX_AGE_MS }
  );
}

Pebble.addEventListener('ready', function () {
  refreshWeather();
  setInterval(refreshWeather, UPDATE_INTERVAL_MS);
});
