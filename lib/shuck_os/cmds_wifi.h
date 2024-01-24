/*
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
ShuckCMD wifi("wifi", "Controll WiFi station and access point", [](ShuckArgs& args, ShuckPrompt& cl) {

    if (args.arg_is(1, "off")) {
        WiFi.enableAP(false);
        WiFi.enableSTA(false);
        return;
    }

    if (args.arg_is(1, "mode")) {
        switch (WiFi.getMode()) {
            case WIFI_OFF:
                stream.println("WiFi is off");
                break;
            case WIFI_AP:
                stream.println("WiFi is in acces point mode");
                break;
            case WIFI_STA:
                stream.println("WiFi is in station mode");
                break;
            case WIFI_AP_STA:
                stream.println("WiFi is in acces point and station mode");
                break;
        }
        return;
    }

    if (args.arg_is(1, "ap")) {

        if (args.arg_is(2, "start")) {
            WiFi.enableAP(true);
            if (args.argc() == 4)
                WiFi.softAP(args.arg(3));
            if (args.argc() == 5)
                WiFi.softAP(args.arg(3), args.arg(4));
        }
        else if (args.arg_is(2, "stop")) {
            WiFi.enableAP(false);
        }
        else if (args.arg_is(2, "num")) {
            Serial.println(WiFi.softAPgetStationNum());
        }
        else if (args.arg_is(2, "ip")) {
            Serial.println(WiFi.softAPIP());
        }
        else {
            Serial.println();
            Serial.println("WiFi acces point controll");
            Serial.println("Usage:");
            Serial.println("wifi ap [cmd]");
            Serial.println("cmd:");
            Serial.println("  start [name] [password] - start ap");
            Serial.println("  stop - turn off ap");
            Serial.println("  num - num of connected devices");
            Serial.println("  ip - ap ip address");
        }
        return;
    }

    if (args.arg_is(1, "sta")) {

        if (args.arg_is(2, "connect") || args.arg_is(2, "conn")) {
            if (args.has(3)) {
                if (args.has(4)) {
                    Serial.printf("Connecting to \"%s\" with password \"%s\"\n", args.arg(3), args.arg(4));
                    WiFi.begin(args.arg(3), args.arg(4));
                }
                else {
                    Serial.printf("Connecting to \"%s\" without password\n", args.arg(3));
                    WiFi.begin(args.arg(3));
                }
            }
        }
        else if (args.arg_is(2, "scan")) {
            Serial.println("Scanning...");
            int n = WiFi.scanNetworks();
            if (n == 0) {
                Serial.println("No networks found");
            }
            else {
                Serial.println("Available networks");
                for (int i = 0; i < n; ++i) {
                    Serial.printf(
                        "%-15s (%d) %s\n",
                        WiFi.SSID(i),
                        WiFi.RSSI(i),
                        (WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"SECURE"
                    );
                }
            }
        }
        else if (args.arg_is(2, "status")) {
            if (WiFi.isConnected())
                Serial.printf("Connected to '%s'\n", WiFi.SSID());
            else
                Serial.println("Not connected");
        }
        else if (args.arg_is(2, "disconn")) {
            WiFi.disconnect();
        }
        else if (args.arg_is(2, "off")) {
            WiFi.enableSTA(false);
        }
        else {
            Serial.println();
            Serial.println("WiFi acces point controll");
            Serial.println("Usage:");
            Serial.println("wifi sta [cmd]");
            Serial.println("cmd:");
            Serial.println("  conn [name] [password] - connect to network");
            Serial.println("  disconn - disconnect from wifi");
            Serial.println("  status - get connection info");
        }
        return;
    }

    Serial.println();
    Serial.println("Usage:");
    Serial.println("wifi [ap/sta/off]");
    Serial.println("  ap   - acces point commands (see: wifi ap help)");
    Serial.println("  sta  - station commands (see: wifi sta help)");
    Serial.println("  mode - see which mode ");
    Serial.println("  off  - turn off both ap and sta");
});
*/