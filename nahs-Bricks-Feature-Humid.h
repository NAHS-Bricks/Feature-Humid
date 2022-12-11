#ifndef NAHS_BRICKS_FEATURE_HUMID_H
#define NAHS_BRICKS_FEATURE_HUMID_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <nahs-Bricks-Lib-HDC1080.h>
#include <nahs-Bricks-Feature-BaseClass.h>
#include <nahs-Bricks-Lib-RTCmem.h>
#include <nahs-Bricks-Lib-FSmem.h>

class NahsBricksFeatureHumid : public NahsBricksFeatureBaseClass {
    private:  // Variables
        static const uint8_t version = 1;
        bool _HDC1080_connected = false;
        typedef struct {
            bool sensorCorrRequested;
            HDC1080_SerialNumber HDC1080SN;  // holds SN of HDC1080 Sensor if connected
            float HDC1080Corr;  // holds correction value for HDC1080 if connected
        } _RTCdata;
        _RTCdata* RTCdata = RTCmem.registerData<_RTCdata>();
        JsonObject FSdata = FSmem.registerData("h");

    public: // BaseClass implementations
        NahsBricksFeatureHumid();
        String getName();
        uint16_t getVersion();
        void begin();
        void start();
        void deliver(JsonDocument* out_json);
        void feedback(JsonDocument* in_json);
        void end();
        void printRTCdata();
        void printFSdata();
        void brickSetupHandover();

    public:  // Brick-Specific setter

    private:  // internal Helpers

    private:  // BrickSetup Helpers
        void _printMenu();
        void _readSensorsRaw();
        void _readSensorsCorr();
        void _setDefaultCorr();
        void _deleteDefaultCorr();
};

#if !defined(NO_GLOBAL_INSTANCES)
extern NahsBricksFeatureHumid FeatureHumid;
#endif

#endif // NAHS_BRICKS_FEATURE_HUMID_H
