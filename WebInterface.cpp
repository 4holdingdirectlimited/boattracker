#include "WebInterface.h"
#include "GlobalConfig.h"

const char HTML_HEADER[] PROGMEM = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>RaceBoatDataLogger</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 15px;
            background: #f0f0f0;
        }
        .header {
            background: #003366;
            color: white;
            padding: 20px;
            text-align: center;
            margin: -15px -15px 20px -15px;
            font-size: 24px;
            font-weight: bold;
        }
        .category {
            background: white;
            border-radius: 10px;
            padding: 15px;
            margin-bottom: 20px;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
        }
        .category h2 {
            color: #003366;
            margin-top: 0;
            border-bottom: 2px solid #003366;
            padding-bottom: 10px;
        }
        .field {
            margin-bottom: 15px;
        }
        label {
            display: block;
            margin-bottom: 5px;
            color: #444;
            font-weight: bold;
        }
        input, select, textarea {
            width: 100%;
            padding: 8px;
            border: 1px solid #ddd;
            border-radius: 4px;
            box-sizing: border-box;
            font-size: 16px;
        }
        .submit-btn {
            background: #003366;
            color: white;
            padding: 15px 30px;
            border: none;
            border-radius: 5px;
            font-size: 18px;
            cursor: pointer;
            width: 100%;
            margin-top: 20px;
        }
        .success-message {
            background: #4CAF50;
            color: white;
            padding: 15px;
            border-radius: 5px;
            margin-bottom: 20px;
            text-align: center;
        }
        textarea {
            resize: vertical;
            min-height: 60px;
        }
    </style>
</head>
<body>
)";

const char HTML_FOOTER[] PROGMEM = R"(
    <script>
        function submitForm() {
            const form = document.getElementById('configForm');
            const formData = new FormData(form);
            
            fetch('/update', {
                method: 'POST',
                body: formData
            })
            .then(response => response.text())
            .then(html => {
                document.body.innerHTML = html;
                setTimeout(() => {
                    window.location.href = '/';
                }, 10000);
            });
            return false;
        }
    </script>
</body>
</html>
)";

WebInterface::WebInterface() : server(80), clientConnected(false), lastClientActivity(0), onConfigUpdate(nullptr) {}

void WebInterface::begin() {
    server.on("/", HTTP_GET, [this]() { this->handleRoot(); });
    server.on("/update", HTTP_POST, [this]() { this->handleUpdate(); });
    server.on("/getconfig", HTTP_GET, [this]() { this->handleGetConfig(); });
    server.begin();
}

void WebInterface::handleClient() {
    server.handleClient();
    if (clientConnected && (millis() - lastClientActivity > 30000)) {
        clientConnected = false;
    }
}

bool WebInterface::isClientConnected() {
    return clientConnected;
}

void WebInterface::setConfigUpdateCallback(void (*callback)()) {
    onConfigUpdate = callback;
}

String WebInterface::generateHTML() {
    String html = FPSTR(HTML_HEADER);
    
    html += F("<div class='header'>RaceBoatDataLogger</div>");
    html += F("<form id='configForm' onsubmit='return submitForm()'>");

    // Boat Category
    html += F("<div class='category'>");
    html += F("<h2>Boat Configuration</h2>");
    html += F("<div class='field'><label>Boat Name:</label><input type='text' name='boat_name' maxlength='20' value='");
    html += userConfig.boat.name;
    html += F("'></div>");
    html += F("<div class='field'><label>Boat Length:</label><input type='text' name='boat_length' maxlength='20' value='");
    html += userConfig.boat.length;
    html += F("'></div>");
    html += F("<div class='field'><label>Boat Weight:</label><input type='text' name='boat_weight' maxlength='20' value='");
    html += userConfig.boat.weight;
    html += F("'></div>");
    html += F("<div class='field'><label>Boat Type:</label><input type='text' name='boat_type' maxlength='20' value='");
    html += userConfig.boat.type;
    html += F("'></div>");
    html += F("<div class='field'><label>Boat Number:</label><input type='text' name='boat_number' maxlength='20' value='");
    html += userConfig.boat.number;
    html += F("'></div>");
    html += F("<div class='field'><label>Notes:</label><textarea name='boat_notes' maxlength='100'>");
    html += userConfig.boat.notes;
    html += F("</textarea></div></div>");

    // Driver Category
    html += F("<div class='category'>");
    html += F("<h2>Driver Configuration</h2>");
    html += F("<div class='field'><label>Driver Name:</label><input type='text' name='driver_name' maxlength='20' value='");
    html += userConfig.driver.name;
    html += F("'></div>");
    html += F("<div class='field'><label>Driver Height:</label><input type='text' name='driver_height' maxlength='20' value='");
    html += userConfig.driver.height;
    html += F("'></div>");
    html += F("<div class='field'><label>Driver Weight:</label><input type='text' name='driver_weight' maxlength='20' value='");
    html += userConfig.driver.weight;
    html += F("'></div>");
    html += F("<div class='field'><label>Notes:</label><textarea name='driver_notes' maxlength='100'>");
    html += userConfig.driver.notes;
    html += F("</textarea></div></div>");

    // Engine Category
    html += F("<div class='category'>");
    html += F("<h2>Engine Configuration</h2>");
    html += F("<div class='field'><label>Engine Brand:</label><input type='text' name='engine_brand' maxlength='20' value='");
    html += userConfig.engine.brand;
    html += F("'></div>");
    html += F("<div class='field'><label>Engine HP:</label><input type='text' name='engine_hp' maxlength='20' value='");
    html += userConfig.engine.horsepower;
    html += F("'></div>");
    html += F("<div class='field'><label>Engine Type:</label><select name='engine_type'>");
    html += F("<option value='inboard'");
    if (String(userConfig.engine.type) == "inboard") html += F(" selected");
    html += F(">Inboard</option>");
    html += F("<option value='outboard'");
    if (String(userConfig.engine.type) == "outboard") html += F(" selected");
    html += F(">Outboard</option></select></div>");
    html += F("<div class='field'><label>Engine Timing:</label><input type='text' name='engine_timing' maxlength='20' value='");
    html += userConfig.engine.timing;
    html += F("'></div>");
    html += F("<div class='field'><label>Ignition Type:</label><input type='text' name='engine_ignition' maxlength='20' value='");
    html += userConfig.engine.ignitionType;
    html += F("'></div>");
    html += F("<div class='field'><label>Fuel Pressure:</label><input type='text' name='engine_fuel_pressure' maxlength='20' value='");
    html += userConfig.engine.fuelPressure;
    html += F("'></div>");
    html += F("<div class='field'><label>Carb Jetting:</label><input type='text' name='engine_carb_jetting' maxlength='20' value='");
    html += userConfig.engine.carbJetting;
    html += F("'></div>");
    html += F("<div class='field'><label>RPM Limit:</label><input type='text' name='engine_rpm_limit' maxlength='20' value='");
    html += userConfig.engine.rpmLimit;
    html += F("'></div>");
    html += F("<div class='field'><label>Fuel Type:</label><input type='text' name='engine_fuel_type' maxlength='20' value='");
    html += userConfig.engine.fuelType;
    html += F("'></div>");
    html += F("<div class='field'><label>Fuel System:</label><select name='engine_fuel_system'>");
    html += F("<option value='carb'");
    if (String(userConfig.engine.fuelSystem) == "carb") html += F(" selected");
    html += F(">Carburetor</option>");
    html += F("<option value='injection'");
    if (String(userConfig.engine.fuelSystem) == "injection") html += F(" selected");
    html += F(">Injection</option></select></div>");
    html += F("<div class='field'><label>Stroke Number:</label><input type='text' name='engine_stroke' maxlength='20' value='");
    html += userConfig.engine.strokeNumber;
    html += F("'></div>");
    html += F("<div class='field'><label>Cylinder Number:</label><input type='text' name='engine_cylinders' maxlength='20' value='");
    html += userConfig.engine.cylinderNumber;
    html += F("'></div>");
    html += F("<div class='field'><label>Cooling System:</label><input type='text' name='engine_cooling' maxlength='20' value='");
    html += userConfig.engine.coolingSystem;
    html += F("'></div>");
    html += F("<div class='field'><label>Notes:</label><textarea name='engine_notes' maxlength='100'>");
    html += userConfig.engine.notes;
    html += F("</textarea></div></div>");

    // Driveline Category
    html += F("<div class='category'>");
    html += F("<h2>Driveline Configuration</h2>");
    html += F("<div class='field'><label>Gear Ratio:</label><input type='text' name='driveline_gear_ratio' maxlength='20' value='");
    html += userConfig.driveline.gearRatio;
    html += F("'></div>");
    html += F("<div class='field'><label>Gearbox Style:</label><input type='text' name='driveline_gearbox' maxlength='20' value='");
    html += userConfig.driveline.gearboxStyle;
    html += F("'></div>");
    html += F("<div class='field'><label>Prop Diameter:</label><input type='text' name='driveline_prop_diameter' maxlength='20' value='");
    html += userConfig.driveline.propDiameter;
    html += F("'></div>");
    html += F("<div class='field'><label>Prop Pitch:</label><input type='text' name='driveline_prop_pitch' maxlength='20' value='");
    html += userConfig.driveline.propPitch;
    html += F("'></div>");
    html += F("<div class='field'><label>Prop Rake:</label><input type='text' name='driveline_prop_rake' maxlength='20' value='");
    html += userConfig.driveline.propRake;
    html += F("'></div>");
    html += F("<div class='field'><label>Prop Style:</label><input type='text' name='driveline_prop_style' maxlength='20' value='");
    html += userConfig.driveline.propStyle;
    html += F("'></div>");
    html += F("<div class='field'><label>Prop Number:</label><input type='text' name='driveline_prop_number' maxlength='20' value='");
    html += userConfig.driveline.propNumber;
    html += F("'></div>");
    html += F("<div class='field'><label>Notes:</label><textarea name='driveline_notes' maxlength='100'>");
    html += userConfig.driveline.notes;
    html += F("</textarea></div></div>");

    // Water Category
    html += F("<div class='category'>");
    html += F("<h2>Water Configuration</h2>");
    html += F("<div class='field'><label>Water Type:</label><select name='water_type'>");
    html += F("<option value='fresh'");
    if (String(userConfig.water.type) == "fresh") html += F(" selected");
    html += F(">Fresh</option>");
    html += F("<option value='salt'");
    if (String(userConfig.water.type) == "salt") html += F(" selected");
    html += F(">Salt</option></select></div>");
    html += F("<div class='field'><label>Water Temperature:</label><input type='text' name='water_temp' maxlength='20' value='");
    html += userConfig.water.temperature;
    html += F("'></div>");
    html += F("<div class='field'><label>Notes:</label><textarea name='water_notes' maxlength='100'>");
    html += userConfig.water.notes;
    html += F("</textarea></div></div>");

    // Logger Settings Category
    html += F("<div class='category'>");
    html += F("<h2>Logger Settings</h2>");
    html += F("<div class='field'><label>RPM1 Magnets per Revolution:</label><input type='text' name='logger_rpm1_magnets' maxlength='20' value='");
    html += userConfig.logger.rpm1MagnetsPerRev;
    html += F("'></div>");
    html += F("<div class='field'><label>RPM2 Magnets per Revolution:</label><input type='text' name='logger_rpm2_magnets' maxlength='20' value='");
    html += userConfig.logger.rpm2MagnetsPerRev;
    html += F("'></div>");
    html += F("<div class='field'><label>RPM1 Cable Length:</label><input type='text' name='logger_rpm1_cable' maxlength='20' value='");
    html += userConfig.logger.rpm1CableLength;
    html += F("'></div>");
    html += F("<div class='field'><label>RPM2 Cable Length:</label><input type='text' name='logger_rpm2_cable' maxlength='20' value='");
    html += userConfig.logger.rpm2CableLength;
    html += F("'></div>");
    html += F("<div class='field'><label>Notes:</label><textarea name='logger_notes' maxlength='100'>");
    html += userConfig.logger.notes;
    html += F("</textarea></div></div>");

    html += F("<input type='submit' value='Confirm Changes' class='submit-btn'>");
    html += F("</form>");
    html += FPSTR(HTML_FOOTER);
    
    return html;
}

void WebInterface::handleUpdate() {
    updateConfigFromPost();
    userConfig.saveToSD();
    userConfig.logCurrentConfig();
    
    if (onConfigUpdate) {
        onConfigUpdate();
    }
    
    server.send(200, "text/html", generateSuccessHTML());
}

String WebInterface::generateSuccessHTML() {
    String html = FPSTR(HTML_HEADER);
    html += F("<div class='header'>RaceBoatDataLogger</div>");
    html += F("<div class='success-message'>Configuration updated successfully! Redirecting in 10 seconds...</div>");
    html += FPSTR(HTML_FOOTER);
    return html;
}

void WebInterface::updateConfigFromPost() {
    // Boat Configuration
    strlcpy(userConfig.boat.name, server.arg("boat_name").c_str(), sizeof(userConfig.boat.name));
    strlcpy(userConfig.boat.length, server.arg("boat_length").c_str(), sizeof(userConfig.boat.length));
    strlcpy(userConfig.boat.weight, server.arg("boat_weight").c_str(), sizeof(userConfig.boat.weight));
    strlcpy(userConfig.boat.type, server.arg("boat_type").c_str(), sizeof(userConfig.boat.type));
    strlcpy(userConfig.boat.number, server.arg("boat_number").c_str(), sizeof(userConfig.boat.number));
    strlcpy(userConfig.boat.notes, server.arg("boat_notes").c_str(), sizeof(userConfig.boat.notes));

    // Driver Configuration
    strlcpy(userConfig.driver.name, server.arg("driver_name").c_str(), sizeof(userConfig.driver.name));
    strlcpy(userConfig.driver.height, server.arg("driver_height").c_str(), sizeof(userConfig.driver.height));
    strlcpy(userConfig.driver.weight, server.arg("driver_weight").c_str(), sizeof(userConfig.driver.weight));
    strlcpy(userConfig.driver.notes, server.arg("driver_notes").c_str(), sizeof(userConfig.driver.notes));

    // Engine Configuration
    strlcpy(userConfig.engine.brand, server.arg("engine_brand").c_str(), sizeof(userConfig.engine.brand));
    strlcpy(userConfig.engine.horsepower, server.arg("engine_hp").c_str(), sizeof(userConfig.engine.horsepower));
    strlcpy(userConfig.engine.type, server.arg("engine_type").c_str(), sizeof(userConfig.engine.type));
    strlcpy(userConfig.engine.timing, server.arg("engine_timing").c_str(), sizeof(userConfig.engine.timing));
    strlcpy(userConfig.engine.ignitionType, server.arg("engine_ignition").c_str(), sizeof(userConfig.engine.ignitionType));
    strlcpy(userConfig.engine.fuelPressure, server.arg("engine_fuel_pressure").c_str(), sizeof(userConfig.engine.fuelPressure));
    strlcpy(userConfig.engine.carbJetting, server.arg("engine_carb_jetting").c_str(), sizeof(userConfig.engine.carbJetting));
    strlcpy(userConfig.engine.rpmLimit, server.arg("engine_rpm_limit").c_str(), sizeof(userConfig.engine.rpmLimit));
    strlcpy(userConfig.engine.fuelType, server.arg("engine_fuel_type").c_str(), sizeof(userConfig.engine.fuelType));
    strlcpy(userConfig.engine.fuelSystem, server.arg("engine_fuel_system").c_str(), sizeof(userConfig.engine.fuelSystem));
    strlcpy(userConfig.engine.strokeNumber, server.arg("engine_stroke").c_str(), sizeof(userConfig.engine.strokeNumber));
    strlcpy(userConfig.engine.cylinderNumber, server.arg("engine_cylinders").c_str(), sizeof(userConfig.engine.cylinderNumber));
    strlcpy(userConfig.engine.coolingSystem, server.arg("engine_cooling").c_str(), sizeof(userConfig.engine.coolingSystem));
    strlcpy(userConfig.engine.notes, server.arg("engine_notes").c_str(), sizeof(userConfig.engine.notes));

    // Driveline Configuration
    strlcpy(userConfig.driveline.gearRatio, server.arg("driveline_gear_ratio").c_str(), sizeof(userConfig.driveline.gearRatio));
    strlcpy(userConfig.driveline.gearboxStyle, server.arg("driveline_gearbox").c_str(), sizeof(userConfig.driveline.gearboxStyle));
    strlcpy(userConfig.driveline.propDiameter, server.arg("driveline_prop_diameter").c_str(), sizeof(userConfig.driveline.propDiameter));
    strlcpy(userConfig.driveline.propPitch, server.arg("driveline_prop_pitch").c_str(), sizeof(userConfig.driveline.propPitch));
    strlcpy(userConfig.driveline.propRake, server.arg("driveline_prop_rake").c_str(), sizeof(userConfig.driveline.propRake));
    strlcpy(userConfig.driveline.propStyle, server.arg("driveline_prop_style").c_str(), sizeof(userConfig.driveline.propStyle));
    strlcpy(userConfig.driveline.propNumber, server.arg("driveline_prop_number").c_str(), sizeof(userConfig.driveline.propNumber));
    strlcpy(userConfig.driveline.notes, server.arg("driveline_notes").c_str(), sizeof(userConfig.driveline.notes));

    // Water Configuration
    strlcpy(userConfig.water.type, server.arg("water_type").c_str(), sizeof(userConfig.water.type));
    strlcpy(userConfig.water.temperature, server.arg("water_temp").c_str(), sizeof(userConfig.water.temperature));
    strlcpy(userConfig.water.notes, server.arg("water_notes").c_str(), sizeof(userConfig.water.notes));

    // Logger Settings
    strlcpy(userConfig.logger.rpm1MagnetsPerRev, server.arg("logger_rpm1_magnets").c_str(), sizeof(userConfig.logger.rpm1MagnetsPerRev));
    strlcpy(userConfig.logger.rpm2MagnetsPerRev, server.arg("logger_rpm2_magnets").c_str(), sizeof(userConfig.logger.rpm2MagnetsPerRev));
    strlcpy(userConfig.logger.rpm1CableLength, server.arg("logger_rpm1_cable").c_str(), sizeof(userConfig.logger.rpm1CableLength));
    strlcpy(userConfig.logger.rpm2CableLength, server.arg("logger_rpm2_cable").c_str(), sizeof(userConfig.logger.rpm2CableLength));
    strlcpy(userConfig.logger.notes, server.arg("logger_notes").c_str(), sizeof(userConfig.logger.notes));
}

void WebInterface::handleGetConfig() {
    String json = "{";
    json += "\"timestamp\":\"" + String(CURRENT_TIMESTAMP) + "\",";
    json += "\"user\":\"" + String(CURRENT_USER) + "\",";
    json += "\"config\":" + userConfig.toJSON();
    json += "}";
    
    server.send(200, "application/json", json);
}

WebInterface webInterface;