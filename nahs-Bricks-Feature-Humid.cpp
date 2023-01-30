#include <nahs-Bricks-Feature-Humid.h>
#include <nahs-Bricks-Lib-SerHelp.h>

NahsBricksFeatureHumid::NahsBricksFeatureHumid() {
}

/*
Returns name of feature
*/
String NahsBricksFeatureHumid::getName() {
    return "humid";
}

/*
Returns version of feature
*/
uint16_t NahsBricksFeatureHumid::getVersion() {
    return version;
}

/*
Configures FSmem und RTCmem variables (prepares feature to be fully operational)
*/
void NahsBricksFeatureHumid::begin() {
    _HDC1080_connected = HDC1080.begin();
    _SHT4x_connected = SHT4x.begin();

    if (!FSdata.containsKey("sCorr")) FSdata.createNestedObject("sCorr");  // dict with sensorSN as key and sensorCorr as value

    if (!RTCmem.isValid()) {
        if (!_HDC1080_connected || !_SHT4x_connected) {
            delay(15);
            _HDC1080_connected = HDC1080.isConnected();
            _SHT4x_connected = SHT4x.isConnected();
        }
        RTCdata->sensorCorrRequested = false;

        if (_HDC1080_connected) {
            HDC1080.getSN(RTCdata->HDC1080SN);
            String sn = HDC1080.snToString(RTCdata->HDC1080SN);
            if (FSdata["sCorr"].as<JsonObject>().containsKey(sn)) {
                RTCdata->HDC1080Corr = FSdata["sCorr"].as<JsonObject>()[sn].as<float>();
            }
            else RTCdata->HDC1080Corr = 0;
        }
        else {
            memset(RTCdata->HDC1080SN, 0, sizeof(RTCdata->HDC1080SN));
            RTCdata->HDC1080Corr = 0;
        }

        if (_SHT4x_connected) {
            SHT4x.getSN(SHTdata->SHT4xSN);
            String sn = SHT4x.snToString(SHTdata->SHT4xSN);
            if (FSdata["sCorr"].as<JsonObject>().containsKey(sn)) {
                SHTdata->SHT4xCorr = FSdata["sCorr"].as<JsonObject>()[sn].as<float>();
            }
            else SHTdata->SHT4xCorr = 0;
        }
        else {
            memset(SHTdata->SHT4xSN, 0, sizeof(SHTdata->SHT4xSN));
            SHTdata->SHT4xCorr = 0;
        }
    }
}

/*
Starts background processes like fetching data from other components
*/
void NahsBricksFeatureHumid::start() {
    // Trigger Conversion of HDC1080 in Background if connected
    if (_HDC1080_connected) {
        HDC1080.triggerRead();
    }

    // Trigger Conversion of SHT4x in Background if connected
    if (_SHT4x_connected) {
        SHT4x.triggerRead();
    }
}

/*
Adds data to outgoing json, that is send to BrickServer
*/
void NahsBricksFeatureHumid::deliver(JsonDocument* out_json) {
    // deliver sensors correction values if requested
    if (RTCdata->sensorCorrRequested) {
        RTCdata->sensorCorrRequested = false;
        JsonArray c_array = out_json->createNestedArray("k");
        if (_HDC1080_connected) {
            JsonArray s_array = c_array.createNestedArray();
            s_array.add(HDC1080.snToString(RTCdata->HDC1080SN));
            s_array.add(RTCdata->HDC1080Corr);
        }
        if (_SHT4x_connected) {
            JsonArray s_array = c_array.createNestedArray();
            s_array.add(SHT4x.snToString(SHTdata->SHT4xSN));
            s_array.add(SHTdata->SHT4xCorr);
        }
    }

    // deliver the humidity readings
    JsonArray t_array = out_json->createNestedArray("h");
    if (_HDC1080_connected) {
        JsonArray s_array = t_array.createNestedArray();
        s_array.add(HDC1080.snToString(RTCdata->HDC1080SN));
        s_array.add(HDC1080.getH() + RTCdata->HDC1080Corr);
    }
    if (_SHT4x_connected) {
        JsonArray s_array = t_array.createNestedArray();
        s_array.add(SHT4x.snToString(SHTdata->SHT4xSN));
        s_array.add(SHT4x.getH() + SHTdata->SHT4xCorr);
    }
}

/*
Processes feedback coming from BrickServer
*/
void NahsBricksFeatureHumid::feedback(JsonDocument* in_json) {
    // evaluate requests
    if (in_json->containsKey("r")) {
        for (JsonVariant value : in_json->operator[]("r").as<JsonArray>()) {
            switch(value.as<uint8_t>()) {
                case 9:
                    RTCdata->sensorCorrRequested = true;
                    break;
            }
        }
    }
}

/*
Finalizes feature (closes stuff)
*/
void NahsBricksFeatureHumid::end() {
}

/*
Prints the features RTCdata in a formatted way to Serial (used for brickSetup)
*/
void NahsBricksFeatureHumid::printRTCdata() {
    Serial.print("  sensorCorrRequested: ");
    SerHelp.printlnBool(RTCdata->sensorCorrRequested);
    Serial.println("  sensor (correction): ");
    if (_HDC1080_connected) {
        Serial.print("    ");
        Serial.print(HDC1080.snToString(RTCdata->HDC1080SN));
        Serial.print(" (");
        Serial.print(RTCdata->HDC1080Corr);
        Serial.println(")");
    }
    if (_SHT4x_connected) {
        Serial.print("    ");
        Serial.print(SHT4x.snToString(SHTdata->SHT4xSN));
        Serial.print(" (");
        Serial.print(SHTdata->SHT4xCorr);
        Serial.println(")");
    }
}

/*
Prints the features FSdata in a formatted way to Serial (used for brickSetup)
*/
void NahsBricksFeatureHumid::printFSdata() {
    Serial.println("  default sensor corrections:");
    for (JsonPair kv : FSdata["sCorr"].as<JsonObject>()) {
        Serial.print("    ");
        Serial.print(kv.key().c_str());
        Serial.print(": ");
        Serial.println(kv.value().as<float>());
    }
}

/*
BrickSetup hands over to this function, when features-submenu is selected
*/
void NahsBricksFeatureHumid::brickSetupHandover() {
    _printMenu();
    while (true) {
        Serial.println();
        Serial.print("Select: ");
        uint8_t input = SerHelp.readLine().toInt();
        switch(input) {
            case 1:
                _readSensorsRaw();
                break;
            case 2:
                _readSensorsCorr();
                break;
            case 3:
                _setDefaultCorr();
                break;
            case 4:
                _deleteDefaultCorr();
                break;
            case 9:
                Serial.println("Returning to MainMenu!");
                return;
                break;
            default:
                Serial.println("Invalid input!");
                _printMenu();
                break;
        }
    }
}

/*
Helper to print Feature submenu during BrickSetup
*/
void NahsBricksFeatureHumid::_printMenu() {
    Serial.println("1) Read sensors (raw)");
    Serial.println("2) Read sensors (with corr)");
    Serial.println("3) Set sensor corr");
    Serial.println("4) Delete sensor corr");
    Serial.println("9) Return to MainMenu");
}

/*
BrickSetup function to do a humidity-reading on all sensors (and put it out WITHOUT correction value)
*/
void NahsBricksFeatureHumid::_readSensorsRaw() {
    Serial.println("Requesting Huminity...");
    if (_HDC1080_connected) {
        HDC1080.getH();  // dummy read to be able to trigger an new conversion
        HDC1080.triggerRead();
        Serial.print(HDC1080.snToString(RTCdata->HDC1080SN));
        Serial.print(": ");
        Serial.println(HDC1080.getH());
    }
    if (_SHT4x_connected) {
        SHT4x.getH();  // dummy read to be able to trigger an new conversion
        SHT4x.triggerRead();
        Serial.print(SHT4x.snToString(SHTdata->SHT4xSN));
        Serial.print(": ");
        Serial.println(SHT4x.getH());
    }
}

/*
BrickSetup function to do a humidity-reading on all sensors (and put it out WITH correction value)
*/
void NahsBricksFeatureHumid::_readSensorsCorr() {
    Serial.println("Requesting Humidity...");
    if (_HDC1080_connected) {
        HDC1080.getH();  // dummy read to be able to trigger an new conversion
        HDC1080.triggerRead();
        Serial.print(HDC1080.snToString(RTCdata->HDC1080SN));
        Serial.print(": ");
        Serial.println(HDC1080.getH() + RTCdata->HDC1080Corr);
    }
    if (_SHT4x_connected) {
        SHT4x.getH();  // dummy read to be able to trigger an new conversion
        SHT4x.triggerRead();
        Serial.print(SHT4x.snToString(SHTdata->SHT4xSN));
        Serial.print(": ");
        Serial.println(SHT4x.getH() + SHTdata->SHT4xCorr);
    }
}

/*
BrickSetup function to set a default correction value for a sensor
*/
void NahsBricksFeatureHumid::_setDefaultCorr() {
    Serial.print("Enter sensor ID: ");
    String addr = SerHelp.readLine();
    Serial.print("Enter correction value: ");
    float corr = SerHelp.readLine().toFloat();

    if (HDC1080.snToString(RTCdata->HDC1080SN) == addr) {
        RTCdata->HDC1080Corr = corr;
    } else if (SHT4x.snToString(SHTdata->SHT4xSN) == addr) {
        SHTdata->SHT4xCorr = corr;
    }
    FSdata["sCorr"].as<JsonObject>()[addr] = corr;
    Serial.print("Set correction value of ");
    Serial.print(addr);
    Serial.print(" to ");
    Serial.println(corr);
}

/*
BrickSetup function to delete a default correction value for a sensor
*/
void NahsBricksFeatureHumid::_deleteDefaultCorr() {
    Serial.print("Enter sensor ID: ");
    String addr = SerHelp.readLine();

    if (HDC1080.snToString(RTCdata->HDC1080SN) == addr) {
        RTCdata->HDC1080Corr = 0;
    } else if (SHT4x.snToString(SHTdata->SHT4xSN) == addr) {
        SHTdata->SHT4xCorr = 0;
    }

    if (FSdata["sCorr"].as<JsonObject>().containsKey(addr)) FSdata["sCorr"].as<JsonObject>().remove(addr);
    Serial.print("Deleted correction value of ");
    Serial.println(addr);
}


//------------------------------------------
// globally predefined variable
#if !defined(NO_GLOBAL_INSTANCES)
NahsBricksFeatureHumid FeatureHumid;
#endif
