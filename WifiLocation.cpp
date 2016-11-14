#include "WifiLocation.h"

static const int E_OK = 0;
static const int E_NOCONNECTION = -1;
static const int WIFI_MAX = 15;

WifiLocation::WifiLocation(Client* client,
                          const char* token,
                          const char* host,
                          int port)
{
  _client = client;
  _token = token;
  _host = host;
  _port = port;
  _location.address = "N/A";
  _location.longitude = 0.0;
  _location.latitude = 0.0;
  _location.accuracy = 0;
}

int WifiLocation::connect(void)
{
  if(!_client->connect(_host, _port))
  {
    return E_NOCONNECTION;
  }

  return E_OK;
}

Location WifiLocation::lastKnownLocation(void)
{
  return _location;
}

Location WifiLocation::updateLocation(WifiAP *wifiAPs, int count)
{
  Location newLocation;
  newLocation.address = "N/A";
  newLocation.longitude = 0.0;
  newLocation.latitude = 0.0;
  newLocation.accuracy = 0;

  if(!_client->connected())
  {
    if(connect() != E_OK)
    {
      _location = newLocation;
      return _location;
    }
  }

  if(count > WIFI_MAX)
  {
    quickSort(wifiAPs, 0, count-1);
  }

  StaticJsonBuffer<2000> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["token"] = _token;
  root["address"] = 1;

  JsonArray& wifis = root.createNestedArray("wifi");
  int numWifi = (count > WIFI_MAX) ? WIFI_MAX : count;
  for(int i=0; i<numWifi; i++)
  {
    JsonObject& wifi = wifis.createNestedObject();
    wifi["bssid"] = wifiAPs[i].bssid;
    wifi["rssi"] = wifiAPs[i].rssi;
    wifi["channel"] = wifiAPs[i].channel;
  }

  char buff[root.measureLength() + 1];
  root.printTo(buff, sizeof(buff));
  String strBuff(buff);
  strBuff += "\n";

  String request = "POST " + (String)ENDPOINT + " HTTP/1.1\r\n" +
    "Host: " + _host + "\r\n" +
    "Content-Type: application/json\r\n" +
    "Content-Length: " + strBuff.length() + "\r\n\r\n" + strBuff;

  String response = "";
  String chunk = "";
  int limit = 0;
  int sendCount = 0;

  _client->print(request);
  do
  {
    if (_client->connected())
    {
      chunk = _client->readStringUntil('\n');
      response += chunk;
    }
    limit++;
  } while (chunk.length() > 0 && limit < 100);
  close();

  if (response.length() > 12)
  {
    String httpCode = getHttpCode(response);
    if(httpCode == "200")
    {
      String respBody = response.substring(response.indexOf('{'), response.indexOf('}') + 1);
      JsonObject& root2 = jsonBuffer.parseObject(respBody);

      if (!root.success())
      {
        return newLocation;
      }

      newLocation.address = root2["address"].asString();
      newLocation.longitude = root2["lon"];
      newLocation.latitude = root2["lat"];
      newLocation.accuracy = root2["accuracy"];
      _location = newLocation;

      return _location;
    }
  }
}

String WifiLocation::getHttpCode(String resp)
{
  int periodIndex = resp.indexOf('.');
  return resp.substring(periodIndex + 3, periodIndex + 6);
}

void WifiLocation::quickSort(WifiAP *arr, int left, int right)
{
  int i = left, j = right;
  WifiAP tmp;
  int pivot = arr[((left + right) / 2)].rssi;

  /* partition */
  while (i <= j) {
        while (arr[i].rssi > pivot)
              i++;
        while (arr[j].rssi < pivot)
              j--;
        if (i <= j) {
              tmp = arr[i];
              arr[i] = arr[j];
              arr[j] = tmp;
              i++;
              j--;
        }
  };

  /* recursion */
  if (left < j)
        quickSort(arr, left, j);
  if (i < right)
        quickSort(arr, i, right);
}

void WifiLocation::close(void)
{
  _client->flush();
  _client->stop();
}
