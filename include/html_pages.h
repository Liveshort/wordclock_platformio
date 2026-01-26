#ifndef _HTML_PAGES_H_
#define _HTML_PAGES_H_

#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>WoordKlok</title>
    <meta name="viewport" content="width=device-width">
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 0;
            padding: 0;
        }
        h1 {
            background-color: #2c3e50;
            color: white;
            margin: 0;
            padding: 15px;
            margin-bottom: 10px;
        }
        h2 {
            background-color: #2c3e50;
            color: white;
            margin: 0;
            padding: 15px;
            margin-top: 10px;
            margin-bottom: 10px;
        }
        input, button {
            margin: 10px;
            padding: 10px;
            font-size: 16px;
        }
        button {
            background-color: #008CBA;
            color: white;
            border: none;
            cursor: pointer;
            border-radius: 4px;
            min-width: 150px;
        }
        button:hover {
            background-color: #005f73;
        }
        .checkbox-group {
            margin: 15px;
        }
        .checkbox-group label {
            margin: 10px;
            display: inline-block;
        }
    </style>
    <script>
        function sendCheckbox(checkbox) {
            fetch("/checkbox", {
                method: "POST",
                headers: { "Content-Type": "application/x-www-form-urlencoded" },
                body: "theme=" + checkbox.value + "&checked=" + (checkbox.checked ? "1" : "0")
            });
        }
        function sendCalibration(action) {
            fetch("/calibrate", {
                method: "POST",
                headers: { "Content-Type": "application/x-www-form-urlencoded" },
                body: "action=" + action
            });
        }
        function updateStatus() {
            fetch("/status").then(response => response.json()).then(data => {
                document.getElementById("wifi_status").textContent = data.status;
                document.getElementById("wifi_ssid").textContent = data.ssid;
                document.getElementById("wifi_last_connected").textContent = data.last_connected;
                document.getElementById("time").textContent = data.time;
                document.getElementById("timezone").textContent = data.timezone;
                document.getElementById("time_last_updated").textContent = data.time_last_updated;
            });
        }
        function updateLogShort() {
            fetch("/log_short").then(response => response.json()).then(data => {
                let logTable = document.getElementById("log");
                logTable.innerHTML = "";
                data.timestamps.forEach((timestamp, i) => {
                    let row = logTable.insertRow(i);

                    let timestampCell = row.insertCell(0);
                    timestampCell.style.textAlign = "left";
                    timestampCell.style.padding = "10px";
                    timestampCell.style.fontWeight = "bold";
                    timestampCell.innerHTML = timestamp;

                    let contentCell = row.insertCell(1);
                    contentCell.style.textAlign = "left";
                    contentCell.style.padding = "10px";
                    contentCell.style.fontWeight = "bold";
                    contentCell.innerHTML = data.contents[i];

                    if (i % 2 === 0) {
                        row.style.backgroundColor = "#54718f";
                        timestampCell.style.color = "white";
                        contentCell.style.color = "white";
                    } else {
                        row.style.backgroundColor = "#f5f9fc";
                        timestampCell.style.color = "black";
                        contentCell.style.color = "black";
                    }
                });
            });
        }

        window.onload = function() {
            updateStatus();
            setInterval(updateStatus, 1000);
            updateLogShort();
            setInterval(updateLogShort, 2500);
        }
    </script>
</head>
<body>
    <h1>WoordKlok</h1>
    <h2>Status</h2>
    <div style="display: flex; justify-content: center;">
        <table style="border-collapse: collapse; margin: 10px; background-color: #eef2f5;">
            <tr style="background-color: #54718f;">
                <td style="text-align: right; padding: 10px; font-weight: bold; color: white;">Netwerkstatus:</td>
                <td style="text-align: left; padding: 10px; color: white;" id="wifi_status">Laden...</td>
            </tr>
            <tr style="background-color: #f5f9fc;">
                <td style="text-align: right; padding: 10px; font-weight: bold;">Netwerknaam:</td>
                <td style="text-align: left; padding: 10px;" id="wifi_ssid">Laden...</td>
            </tr>
            <tr style="background-color: #54718f;">
                <td style="text-align: right; padding: 10px; font-weight: bold; color: white;">Laatst verbonden:</td>
                <td style="text-align: left; padding: 10px; color: white;" id="wifi_last_connected">Laden...</td>
            </tr>
            <tr style="background-color: #f5f9fc;">
                <td style="text-align: right; padding: 10px; font-weight: bold;">Interne tijd:</td>
                <td style="text-align: left; padding: 10px;" id="time">Laden...</td>
            </tr>
            <tr style="background-color: #54718f;">
                <td style="text-align: right; padding: 10px; font-weight: bold; color: white;">Tijdzone:</td>
                <td style="text-align: left; padding: 10px; color: white;" id="timezone">Laden...</td>
            </tr>
            <tr style="background-color: #f5f9fc;">
                <td style="text-align: right; padding: 10px; font-weight: bold;">Gesynchroniseerd:</td>
                <td style="text-align: left; padding: 10px;" id="time_last_updated">Laden...</td>
            </tr>
        </table>
    </div>
    <button onclick="window.location.href='/wifi_settings'">Netwerk</button>
    <button onclick="window.location.href='/settings'">Instellingen</button>
    <button onclick="window.location.href='/drawing_board'">Tekenbord</button>
    <h2>Thema's</h2>
    <label><input type="checkbox" value="Geel" onchange="sendCheckbox(this)"> Geel</label><br>
    <label><input type="checkbox" value="Blauw" onchange="sendCheckbox(this)"> Blauw</label><br>
    <label><input type="checkbox" value="Wit" onchange="sendCheckbox(this)"> Wit</label><br>
    <h2>Andere instellingen</h2>
    <button onclick="sendCalibration('dark')">Kalibreer donker</button>
    <button onclick="sendCalibration('light')">Kalibreer licht</button>
    <h2>Log</h2>
    <div style="display: flex; justify-content: center;">
        <table style="border-collapse: collapse; margin: 10px; background-color: #eef2f5; width: 100%; max-height: 300px; overflow-y: auto; font-family: 'Courier New', Courier, monospace; font-size: 0.9em;">
            <tbody id="log">
                <tr style="background-color: #54718f;">
                    <td style="text-align: left; padding: 10px; color: white; white-space: nowrap;"></td>
                    <td style="text-align: left; padding: 10px; color: white; width: 100%;">Laden...</td>
                </tr>
            </tbody>
        </table>
    </div>
    <button onclick="window.location.href='/log_page'">Uitgebreide log</button>
</body>
</html>
)rawliteral";

const char wifi_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>WoordKlok - Netwerk</title>
    <meta name="viewport" content="width=device-width">
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 0;
            padding: 0;
        }
        h1 {
            background-color: #2c3e50;
            color: white;
            margin: 0;
            padding: 15px;
            margin-bottom: 10px;
        }
        h2 {
            background-color: #2c3e50;
            color: white;
            margin: 0;
            padding: 15px;
            margin-top: 10px;
            margin-bottom: 10px;
        }
        .form-container {
            display: flex;
            justify-content: center;
            margin: 20px;
        }
        .form-group {
            display: flex;
            align-items: center;
            margin: 10px;
            width: 100%;
            max-width: 400px;
        }
        .form-group label {
            width: 120px;
            text-align: right;
            margin-right: 10px;
        }
        .form-group select,
        .form-group input {
            width: 300px;
            padding: 8px;
            border: 1px solid #ccc;
            border-radius: 4px;
            box-sizing: border-box;
            height: 38px;
            font-size: 16px;
            color: #000000;
        }
        input, button {
            margin: 10px;
            padding: 10px;
            font-size: 16px;
        }
        button {
            background-color: #008CBA;
            color: white;
            border: none;
            cursor: pointer;
            border-radius: 4px;
            min-width: 150px;
        }
        button:hover {
            background-color: #005f73;
        }
    </style>
    <script>
        function loadSSIDs() {
            fetch("/ssids").then(response => response.json()).then(data => {
                let select = document.getElementById("ssid");
                select.innerHTML = "";
                data.forEach(ssid => {
                    let option = document.createElement("option");
                    option.value = ssid;
                    option.textContent = ssid;
                    select.appendChild(option);
                });
            });
        }
        function handleSubmit(event) {
            event.preventDefault();
            const formData = new FormData(event.target);
            fetch("/submit", {
                method: "POST",
                body: new URLSearchParams(formData)
            }).then(() => {
                window.location.href = '/';
            });
        }
        function updateStatus() {
            fetch("/status").then(response => response.json()).then(data => {
                document.getElementById("wifi_status").textContent = data.status;
                document.getElementById("wifi_ssid").textContent = data.ssid;
                document.getElementById("wifi_last_connected").textContent = data.last_connected;
            });
        }
        function scanNetworks() {
            document.getElementById("scan_networks_button").textContent = "Netwerkscan bezig...";
            fetch('/scan_wifi_networks').then(() => {
                window.location.reload();
            });
        }
        window.onload = function() {
            loadSSIDs();
            updateStatus();
            setInterval(updateStatus, 2000);
        }
    </script>
</head>
<body>
    <h1>WoordKlok - Netwerk</h1>
    <button onclick="window.location.href='/'">Terug naar hoofdmenu</button>
    <h2>Netwerkstatus</h2>
    <div style="display: flex; justify-content: center;">
        <table style="border-collapse: collapse; margin: 10px; background-color: #eef2f5;">
            <tr style="background-color: #54718f;">
                <td style="text-align: right; padding: 10px; font-weight: bold; color: white;">Netwerkstatus:</td>
                <td style="text-align: left; padding: 10px; color: white;" id="wifi_status">Laden...</td>
            </tr>
            <tr style="background-color: #f5f9fc;">
                <td style="text-align: right; padding: 10px; font-weight: bold;">Netwerknaam:</td>
                <td style="text-align: left; padding: 10px;" id="wifi_ssid">Laden...</td>
            </tr>
            <tr style="background-color: #54718f;">
                <td style="text-align: right; padding: 10px; font-weight: bold; color: white;">Laatst verbonden:</td>
                <td style="text-align: left; padding: 10px; color: white;" id="wifi_last_connected">Laden...</td>
            </tr>
        </table>
    </div>
    <h2>Verbinden met netwerk</h2>
    <button id="scan_networks_button" onclick="scanNetworks()">Start netwerkscan</button>
    <div class="form-container">
        <form onsubmit="handleSubmit(event)">
            <div class="form-group">
                <label for="ssid">Netwerknaam:</label>
                <select id="ssid" name="ssid"></select>
            </div>
            <div class="form-group">
                <label for="password">Wachtwoord:</label>
                <input type="password" id="password" name="password" required>
            </div>
            <button type="submit">Verbinden</button>
        </form>
    </div>
</body>
</html>
)rawliteral";

const char settings_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>WoordKlok - Netwerk</title>
    <meta name="viewport" content="width=device-width">
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 0;
            padding: 0;
        }
        h1 {
            background-color: #2c3e50;
            color: white;
            margin: 0;
            padding: 15px;
            margin-bottom: 10px;
        }
        h2 {
            background-color: #2c3e50;
            color: white;
            margin: 0;
            padding: 15px;
            margin-top: 10px;
            margin-bottom: 10px;
        }
        .form-container {
            display: flex;
            justify-content: center;
            margin: 20px;
        }
        .form-group {
            display: flex;
            align-items: center;
            margin: 10px;
            width: 100%;
            max-width: 400px;
        }
        .form-group label {
            width: 120px;
            text-align: right;
            margin-right: 10px;
        }
        .form-group select,
        .form-group input {
            width: 300px;
            padding: 8px;
            border: 1px solid #ccc;
            border-radius: 4px;
            box-sizing: border-box;
            height: 38px;
            font-size: 16px;
            color: #000000;
        }
        input, button {
            margin: 10px;
            padding: 10px;
            font-size: 16px;
        }
        button {
            background-color: #008CBA;
            color: white;
            border: none;
            cursor: pointer;
            border-radius: 4px;
            min-width: 150px;
        }
        button:hover {
            background-color: #005f73;
        }
    </style>
    <script>
        // send a single setting to the server
        function sendSetting(setting, value) {
            const valToSend = Number(value);
            fetch("/set_setting", {
                method: "POST",
                headers: { "Content-Type": "application/x-www-form-urlencoded" },
                body: "setting=" + setting + "&value=" + encodeURIComponent(valToSend)
            }).then(response => {
                if (!response.ok) {
                    console.error("Failed to set setting", setting);
                } else {
                    console.log("Setting", setting, "set to", valToSend);
                    loadSettings();
                }
            }).catch(err => console.error(err));
        }

        // try to load current values from the server on load
        window.addEventListener('load', function() {
            loadSettings();
        });

        function loadSettings() {
            fetch("/get_settings").then(r => r.json()).then(data => {
                if (data && data.round_down_time !== undefined) {
                    const sel = document.getElementById("select_round_down_time");
                    if (sel) sel.value = data.round_down_time ? "1" : "0";
                }
                if (data && data.sayings_enabled !== undefined) {
                    const s = document.getElementById("select_sayings_enabled");
                    if (s) s.value = data.sayings_enabled ? "1" : "0";
                }
                if (data && data.saying_interval_s !== undefined) {
                    const iv = document.getElementById("input_saying_interval_s");
                    if (iv) iv.value = String(data.saying_interval_s);
                }
                if (data && data.saying_duration_s !== undefined) {
                    const dv = document.getElementById("input_saying_duration_s");
                    if (dv) dv.value = String(data.saying_duration_s);
                }
                if (data && data.fade_cycle_s !== undefined) {
                    const fc = document.getElementById("select_fade_cycle_s");
                    if (fc) fc.value = String(data.fade_cycle_s);
                }
                if (data && data.palette_interval_s !== undefined) {
                    const pi = document.getElementById("input_palette_interval_s");
                    if (pi) pi.value = String(data.palette_interval_s);
                }
                if (data && data.palette_cycle_s !== undefined) {
                    const pc = document.getElementById("input_palette_cycle_s");
                    if (pc) pc.value = String(data.palette_cycle_s);
                }
            }).catch(err => console.error(err));
        }
    </script>
</head>
<body>
    <h1>WoordKlok - Instellingen</h1>
    <div style="display: flex; justify-content: center;">
        <table style="border-collapse: collapse; margin: 10px; background-color: #eef2f5;">
            <tr style="background-color: #54718f;">
                <td style="text-align: right; padding: 10px; font-weight: bold; color: white;">Tijdweergave</td>
                <td style="text-align: left; padding: 10px;" id="option_round_down_time">
                    <select id="select_round_down_time" onchange="sendSetting(0, this.value)">
                        <option value="0">13:33 -> 13:35</option>
                        <option value="1">13:33 -> 13:30</option>
                    </select>
                </td>
            </tr>
            <tr style="background-color: #f5f9fc;">
                <td style="text-align: right; padding: 10px; font-weight: bold; color: black;">Gezegden</td>
                <td style="text-align: left; padding: 10px;" id="option_sayings_enabled">
                    <select id="select_sayings_enabled" onchange="sendSetting(1, this.value)">
                        <option value="0">Geen gezegden</option>
                        <option value="1">Wel gezegden</option>
                    </select>
                </td>
            </tr>
            <tr style="background-color: #54718f;">
                <td style="text-align: right; padding: 10px; font-weight: bold; color: white;">Gezegden interval</td>
                <td style="text-align: left; padding: 0px;" id="option_saying_interval_s">
                    <input type="number" id="input_saying_interval_s" min="40" max="10800"
                           onchange=""
                           onblur="sendSetting(2, this.value)"
                           onkeydown="if (event.key === 'Enter') { this.blur(); }"
                           style="width:100px; padding:0px;" />
                </td>
            </tr>
            <tr style="background-color: #f5f9fc">
                <td style="text-align: right; padding: 10px; font-weight: bold; color: black;">Gezegden duur</td>
                <td style="text-align: left; padding: 0px;" id="option_saying_duration_s">
                    <input type="number" id="input_saying_duration_s" min="5" max="30"
                           onchange=""
                           onblur="sendSetting(3, this.value)"
                           onkeydown="if (event.key === 'Enter') { this.blur(); }"
                           style="width:100px; padding:0px;" />
                </td>
            </tr>
            <tr style="background-color: #54718f;">
                <td style="text-align: right; padding: 10px; font-weight: bold; color: white;">Fade in/out duur</td>
                <td style="text-align: left; padding: 10px;" id="option_fade_cycle_s">
                    <select id="select_fade_cycle_s" onchange="sendSetting(4, this.value)">
                        <option value="0">0 seconden</option>
                        <option value="1">1 seconde</option>
                        <option value="2">2 seconden</option>
                        <option value="4">4 seconden</option>
                        <option value="8">8 seconden</option>
                    </select>
                </td>
            </tr>
            <tr style="background-color: #f5f9fc">
                <td style="text-align: right; padding: 10px; font-weight: bold; color: black;">Kleurpalet interval</td>
                <td style="text-align: left; padding: 0px;" id="option_palette_interval_s">
                    <input type="number" id="input_palette_interval_s" min="300" max="604800"
                           onchange=""
                           onblur="sendSetting(6, this.value)"
                           onkeydown="if (event.key === 'Enter') { this.blur(); }"
                           style="width:100px; padding:0px;" />
                </td>
            </tr>
            <tr style="background-color: #54718f;">
                <td style="text-align: right; padding: 10px; font-weight: bold; color: white;">Kleurstroom duur</td>
                <td style="text-align: left; padding: 0px;" id="option_palette_cycle_s">
                    <input type="number" id="input_palette_cycle_s" min="5" max="3600"
                           onchange=""
                           onblur="sendSetting(7, this.value)"
                           onkeydown="if (event.key === 'Enter') { this.blur(); }"
                           style="width:100px; padding:0px;" />
                </td>
            </tr>
        </table>
    </div>
    <h2>Legenda</h2>
    <div style="display: flex; justify-content: center; max-width: 600px; margin: auto;">
        <table style="border-collapse: collapse; margin: 10px; background-color: #eef2f5;">
            <tr style="background-color: #54718f;">
                <td style="text-align: center; padding: 10px; font-weight: bold; color: white;">Tijdweergave</td>
            </tr>
            <tr style="background-color: #f5f9fc;">
                <td style="text-align: left; padding: 10px; color: black;">Als "13:33 -> 13:35", dan wordt de tijd afgerond naar het dichtsbijzijnde vijftal. Als "13:33 -> 13:30", dan wordt de tijd altijd naar beneden afgerond. De minuut-lampjes onderaan de klok geven de verstreken tijd tussen de vijf-minuutintervallen aan.</td>
            </tr>
            <tr style="background-color: #54718f;">
                <td style="text-align: center; padding: 10px; font-weight: bold; color: white;">Gezegden</td>
            </tr>
            <tr style="background-color: #f5f9fc;">
                <td style="text-align: left; padding: 10px; color: black;">Bepaalt of er met vaste intervallen gezegden worden weergegeven of niet.</td>
            </tr>
            <tr style="background-color: #54718f;">
                <td style="text-align: center; padding: 10px; font-weight: bold; color: white;">Gezegden interval</td>
            </tr>
            <tr style="background-color: #f5f9fc;">
                <td style="text-align: left; padding: 10px; color: black;">Tijd tussen gezegden in seconden. Minimaal 40 seconden, maximaal 3 uur.</td>
            </tr>
            <tr style="background-color: #54718f;">
                <td style="text-align: center; padding: 10px; font-weight: bold; color: white;">Gezegden duur</td>
            </tr>
            <tr style="background-color: #f5f9fc;">
                <td style="text-align: left; padding: 10px; color: black;">Weergavetijd gezegden in seconden. Minimaal 5 seconden, maximaal 30 seconden.</td>
            </tr>
            <tr style="background-color: #54718f;">
                <td style="text-align: center; padding: 10px; font-weight: bold; color: white;">Fade in/out duur</td>
            </tr>
            <tr style="background-color: #f5f9fc;">
                <td style="text-align: left; padding: 10px; color: black;">Duur fade in/out in seconden. De duur geeft de totale tijd van een out & in cyclus aan. Minimaal 0 seconden (geen fade), maximaal 8 seconden.</td>
            </tr>
            <tr style="background-color: #54718f;">
                <td style="text-align: center; padding: 10px; font-weight: bold; color: white;">Kleurpalet interval</td>
            </tr>
            <tr style="background-color: #f5f9fc;">
                <td style="text-align: left; padding: 10px; color: black;">Tijd tussen verschillende kleurenpaletten in seconden. Minimaal 5 minuten, maximaal 1 week.</td>
            </tr>
            <tr style="background-color: #54718f;">
                <td style="text-align: center; padding: 10px; font-weight: bold; color: white;">Kleurstroom duur</td>
            </tr>
            <tr style="background-color: #f5f9fc;">
                <td style="text-align: left; padding: 10px; color: black;">Duur van de kleurstroom in de paletten in seconden. Bepaalt de tijd die het duurt voordat het kleurenpalet helemaal rond is geweest. Minimaal 5 seconden, maximaal 1 uur.</td>
            </tr>
        </table>
    </div>
    <button onclick="window.location.href='/'">Terug naar hoofdmenu</button>
</body>
</html>
)rawliteral";

const char log_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>WoordKlok - Log</title>
    <meta name="viewport" content="width=device-width">
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 0;
            padding: 0;
        }
        h1 {
            background-color: #2c3e50;
            color: white;
            margin: 0;
            padding: 15px;
            margin-bottom: 10px;
        }
        h2 {
            background-color: #2c3e50;
            color: white;
            margin: 0;
            padding: 15px;
            margin-top: 10px;
            margin-bottom: 10px;
        }
        input, button {
            margin: 10px;
            padding: 10px;
            font-size: 16px;
        }
        button {
            background-color: #008CBA;
            color: white;
            border: none;
            cursor: pointer;
            border-radius: 4px;
            min-width: 150px;
        }
        button:hover {
            background-color: #005f73;
        }
    </style>
    <script>
        function updateLogLong() {
            fetch("/log_long").then(response => response.json()).then(data => {
                let logTable = document.getElementById("log");
                logTable.innerHTML = "";
                data.timestamps.forEach((timestamp, i) => {
                    let row = logTable.insertRow(i);

                    let timestampCell = row.insertCell(0);
                    timestampCell.style.textAlign = "left";
                    timestampCell.style.padding = "10px";
                    timestampCell.style.fontWeight = "bold";
                    timestampCell.innerHTML = timestamp;

                    let contentCell = row.insertCell(1);
                    contentCell.style.textAlign = "left";
                    contentCell.style.padding = "10px";
                    contentCell.style.fontWeight = "bold";
                    contentCell.innerHTML = data.contents[i];

                    if (i % 2 === 0) {
                        row.style.backgroundColor = "#54718f";
                        timestampCell.style.color = "white";
                        contentCell.style.color = "white";
                    } else {
                        row.style.backgroundColor = "#f5f9fc";
                        timestampCell.style.color = "black";
                        contentCell.style.color = "black";
                    }
                });
            });
        }

        window.onload = function() {
            updateLogLong();
            setInterval(updateLogLong, 2000);
        }
    </script>
</head>
<body>
    <h1>WoordKlok - Log</h1>
    <button onclick="window.location.href='/'">Terug naar hoofdmenu</button>
    <h2>Log</h2>
    <div style="display: flex; justify-content: center;">
        <table style="border-collapse: collapse; margin: 10px; background-color: #eef2f5; width: 100%; max-height: 300px; overflow-y: auto; font-family: 'Courier New', Courier, monospace; font-size: 0.9em;">
            <tbody id="log">
                <tr style="background-color: #54718f;">
                    <td style="text-align: left; padding: 10px; color: white; white-space: nowrap;"></td>
                    <td style="text-align: left; padding: 10px; color: white; width: 100%;">Laden...</td>
                </tr>
            </tbody>
        </table>
    </div>
</body>
</html>
)rawliteral";

const char drawing_board_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Tekenbord - WoordKlok</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 0;
            padding: 0;
            background-color: #f0f0f0;
        }
        h1 {
            background-color: #2c3e50;
            color: white;
            margin: 0;
            padding: 15px;
        }
        #grid-container {
            display: inline-block;
            margin: 20px auto;
            padding: 10px;
            background-color: #34495e;
            border-radius: 10px;
        }
        #led-grid {
            display: grid;
            grid-template-columns: repeat(13, 1fr);
            gap: 3px;
            max-width: 90vw;
            margin: 0 auto;
        }
        /* Center the last 5 LEDs (minute dots) in the bottom row */
        /* LEDs 169-173 (cells 170-174 in CSS) should be centered */
        .led-cell:nth-child(170) {
            grid-column: 5 / 6;
        }
        .led-cell:nth-child(171) {
            grid-column: 6 / 7;
        }
        .led-cell:nth-child(172) {
            grid-column: 7 / 8;
        }
        .led-cell:nth-child(173) {
            grid-column: 8 / 9;
        }
        .led-cell:nth-child(174) {
            grid-column: 9 / 10;
        }
        .led-cell {
            aspect-ratio: 1;
            background-color: #1a1a1a;
            border: 1px solid #555;
            border-radius: 3px;
            cursor: pointer;
            transition: background-color 0.1s;
            min-width: 20px;
            display: flex;
            align-items: center;
            justify-content: center;
            font-family: 'Courier New', monospace;
            font-weight: bold;
            font-size: clamp(10px, 2vw, 16px);
            color: #666;
        }
        .led-cell.on {
            background-color: #ffffff;
            box-shadow: 0 0 10px rgba(255, 255, 255, 0.8);
            color: #000;
        }
        .led-cell:active {
            transform: scale(0.95);
        }
        .button-container {
            margin: 20px;
        }
        button {
            margin: 10px;
            padding: 15px 30px;
            font-size: 16px;
            background-color: #008CBA;
            color: white;
            border: none;
            cursor: pointer;
            border-radius: 4px;
            min-width: 150px;
        }
        button:hover {
            background-color: #005f73;
        }
        button.danger {
            background-color: #e74c3c;
        }
        button.danger:hover {
            background-color: #c0392b;
        }
    </style>
</head>
<body>
    <h1>Tekenbord</h1>
    <div id="grid-container">
        <div id="led-grid"></div>
    </div>
    <div class="button-container">
        <div style="margin: 20px;">
            <label for="colorPicker" style="font-size: 18px; font-weight: bold; margin-right: 10px;">Kleur:</label>
            <input type="color" id="colorPicker" value="#ffffff" style="width: 60px; height: 40px; border: none; cursor: pointer; border-radius: 4px;">
        </div>
        <button onclick="clearAll()" class="danger">Wis Alles</button>
        <button onclick="window.location.href='/'">Terug</button>
    </div>

    <script>
        const GRID_SIZE = 13;
        const TOTAL_LEDS = 174;
        let ledStates = new Array(TOTAL_LEDS).fill(0);
        let ledColors = new Array(TOTAL_LEDS).fill('#ffffff');

        // Letter mapping for the word clock (LED index to letter)
        const LED_LETTERS = [
            'H','E','T','D','I','E','R','E','N','E','E','M','C',
            'E','Q','I','S','D','E','E','N','L','A','T','E','R',
            'M','E','E','R','W','H','O','O','G','T','I','E','N',
            'K','W','A','R','T','I','J','D','V','I','J','F','Y',
            'I','S','Q','V','E','R','V','O','O','R','B','I','J',
            'I','E','T','S','D','A','N','D','E','R','S','J','E',
            'H','A','L','F','N','I','E','U','W','S','V','A','N',
            'Z','E','L','F','K','O','M','T','D','E','N','K','T',
            'A','C','H','T','W','E','E','N','K','O','M','E','N',
            'G','A','A','N','E','G','E','N','Z','E','V','E','N',
            'V','L','I','E','G','T','I','E','N','V','I','E','R',
            'D','R','I','E','N','O','G','T','W','A','A','L','F',
            'V','I','J','F','Z','E','S','X','U','U','R','K','!',
            '.','.','.','.','.'
        ];

        // Create the grid with letters
        function createGrid() {
            const grid = document.getElementById('led-grid');
            for (let i = 0; i < TOTAL_LEDS; i++) {
                const cell = document.createElement('div');
                cell.className = 'led-cell';
                cell.dataset.index = i;
                cell.textContent = LED_LETTERS[i] || '';
                cell.onclick = () => toggleLED(i);
                grid.appendChild(cell);
            }
        }

        // Toggle LED state
        function toggleLED(index) {
            if (ledStates[index] === 1) {
                // Turn off
                ledStates[index] = 0;
            } else {
                // Turn on with selected color
                ledStates[index] = 1;
                ledColors[index] = document.getElementById('colorPicker').value;
            }
            updateCellDisplay(index);
            sendLEDUpdate(index, ledStates[index], ledColors[index]);
        }

        // Update cell visual state
        function updateCellDisplay(index) {
            const cell = document.querySelector(`[data-index="${index}"]`);
            if (ledStates[index] === 1) {
                cell.classList.add('on');
                cell.style.backgroundColor = ledColors[index];
                cell.style.boxShadow = `0 0 10px ${ledColors[index]}`;
            } else {
                cell.classList.remove('on');
                cell.style.backgroundColor = '';
                cell.style.boxShadow = '';
            }
        }

        // Send LED update to ESP32
        function sendLEDUpdate(led, state, color) {
            // Convert hex color to RGB
            const r = parseInt(color.substr(1,2), 16);
            const g = parseInt(color.substr(3,2), 16);
            const b = parseInt(color.substr(5,2), 16);

            fetch('/set_led', {
                method: 'POST',
                headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                body: `led=${led}&state=${state}&r=${r}&g=${g}&b=${b}`
            }).catch(err => console.error('Error:', err));
        }

        // Clear all LEDs
        function clearAll() {
            if (confirm('Weet je zeker dat je alles wilt wissen?')) {
                fetch('/clear_leds', {method: 'POST'})
                    .then(() => {
                        ledStates.fill(0);
                        for (let i = 0; i < TOTAL_LEDS; i++) {
                            updateCellDisplay(i);
                        }
                    })
                    .catch(err => console.error('Error:', err));
            }
        }

        // Load current LED states
        function loadLEDStates() {
            fetch('/get_leds')
                .then(response => response.json())
                .then(data => {
                    ledStates = data.leds;
                    for (let i = 0; i < TOTAL_LEDS; i++) {
                        updateCellDisplay(i);
                    }
                })
                .catch(err => console.error('Error loading LEDs:', err));
        }

        // Initialize
        createGrid();
        loadLEDStates();

        // Handle page unload - return to normal operation
        window.addEventListener('beforeunload', function() {
            fetch('/exit_drawing_board', {method: 'POST', keepalive: true});
        });
    </script>
</body>
</html>
)rawliteral";

#endif
