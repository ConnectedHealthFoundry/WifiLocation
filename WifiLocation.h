#ifndef WIFILOCATION_H
#define WIFILOCATION_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "Client.h"

static const char* DEFAULT_HOST = "us2.unwiredlabs.com";
static const int DEFAULT_PORT = 80;
static const char* ENDPOINT = "/v2/process.php";

typedef struct Location
{
  String address;
  double longitude;
  double latitude;
  int accuracy;
} Location;

typedef struct WifiAP
{
  String bssid;
  int rssi;
  int channel;
} WifiAP;

class WifiLocation
{
  public:
    WifiLocation(Client* client,
                const char* token,
                const char* host = DEFAULT_HOST,
                int port = DEFAULT_PORT);

    Location updateLocation(WifiAP *wifis, int count);
    Location lastKnownLocation(void);

  private:
    Client* _client;
    const char* _token;
    const char* _host;
    int _port;
    Location _location;

    int connect(void);
    String getHttpCode(String resp);
    void close(void);
    void quickSort(WifiAP *arr, int left, int right);
};

#endif
