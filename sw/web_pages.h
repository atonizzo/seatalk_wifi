// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: Anthony Tonizzo - 2022

const char homepage[] = R"=====(
<html>
<head>
  <script>
  function load_body()
  {
    const xhttp_settings = new XMLHttpRequest();
    xhttp_settings.onload = function() {pull_settings(this);}
    xhttp_settings.open("GET", "/pull_settings");
    xhttp_settings.send();

    const xhttp_wind_settings = new XMLHttpRequest();
    xhttp_wind_settings.onload = function() {pull_wind_settings(this);}
    xhttp_wind_settings.open("GET", "/pull_wind_settings");
    xhttp_wind_settings.send();

  }
  function pull_settings(xhttp) {
    const urlParams = new URLSearchParams(xhttp.responseText);
    document.getElementsByName("ipaddr")[0].innerHTML = urlParams.get('ipaddr');
    document.getElementsByName("slogger")[0].checked = (parseInt(urlParams.get('slogger'), 10) == 1);
    document.getElementsByName("tlogger")[0].checked = (parseInt(urlParams.get('tlogger'), 10) == 1);
    document.getElementsByName("slogger_baudrate")[0].selectedIndex = parseInt(urlParams.get('slogger_baudrate'), 10);
    document.getElementsByName("server_port")[0].valueAsNumber = parseInt(urlParams.get('server_port'), 10);
    document.getElementsByName("hostname")[0].value = urlParams.get('hostname');
    document.getElementsByName("nmea_talker")[0].value = urlParams.get('nmea_talker').substr(0, 2);
    document.getElementsByName("colorize")[0].checked = (parseInt(urlParams.get('colorize'), 10) == 1);
    document.getElementsByName("led")[0].checked = (parseInt(urlParams.get('led'), 10) == 1);
  }
  function pull_wind_settings(xhttp) {
    const urlParams = new URLSearchParams(xhttp.responseText);
    document.getElementsByName("filter_angle_enable")[0].checked = (parseInt(urlParams.get('filter_angle_enable'), 10) == 1);
    document.getElementsByName("angle_slider")[0].disabled = (parseInt(urlParams.get('filter_angle_enable'), 10) == 0)
    document.getElementsByName("filter_speed_enable")[0].checked = (parseInt(urlParams.get('filter_speed_enable'), 10) == 1);
    document.getElementsByName("speed_slider")[0].disabled = (parseInt(urlParams.get('filter_speed_enable'), 10) == 0)
    document.getElementsByName("angle_slider")[0].attributes.max.value = parseInt(urlParams.get('filter_angle_max'), 10);
    document.getElementsByName("speed_slider")[0].attributes.max.value = parseInt(urlParams.get('filter_speed_max'), 10);
    document.getElementsByName("angle_slider")[0].valueAsNumber = parseInt(urlParams.get('filter_angle_len'), 10);
    document.getElementsByName("speed_slider")[0].valueAsNumber = parseInt(urlParams.get('filter_speed_len'), 10);
    document.getElementsByName("slider_angle_len")[0].innerHTML = "" + parseInt(urlParams.get('filter_angle_len'), 10)
    document.getElementsByName("slider_speed_len")[0].innerHTML = "" + parseInt(urlParams.get('filter_speed_len'), 10)
  }
  function push_settings()
  {
    url = "/push_settings?slogger="
    url += (document.getElementsByName("slogger")[0].checked == true) ? 1 : 0
    url += "&tlogger="
    url += (document.getElementsByName("tlogger")[0].checked == true) ? 1 : 0
    url += "&slogger_baud="
    url += document.getElementsByName("slogger_baudrate")[0].selectedIndex
    url += "&server_port="
    url += document.getElementsByName("server_port")[0].value
    url += "&hostname="
    url += document.getElementsByName("hostname")[0].value
    url += "&nmea_talker="
    url += document.getElementsByName("nmea_talker")[0].value.substr(0, 2)
    url += "&colorize="
    url += (document.getElementsByName("colorize")[0].checked == true) ? 1 : 0
    url += "&led="
    url += (document.getElementsByName("led")[0].checked == true) ? 1 : 0
    const xhttp=new XMLHttpRequest();
    xhttp.onload = function() {post_update(this);}
    xhttp.open("GET", url);
    xhttp.send();
  }
  function push_wind_settings()
  {
    url = "/push_wind_settings?filter_angle_enable=" + (document.getElementsByName("filter_angle_enable")[0].checked == true) ? 1 : 0
    document.getElementsByName("angle_slider")[0].disabled = (document.getElementsByName("filter_angle_enable")[0].checked == false)
    url += "&filter_speed_enable=" + (document.getElementsByName("filter_speed_enable")[0].checked == true) ? 1 : 0
    document.getElementsByName("speed_slider")[0].disabled = (document.getElementsByName("filter_speed_enable")[0].checked == false)
    url += "&angle_length=" + document.getElementsByName("angle_slider")[0].valueAsNumber
    url += "&speed_length=" + document.getElementsByName("speed_slider")[0].valueAsNumber
    const xhttp=new XMLHttpRequest();
    xhttp.onload = function() {post_update(this);}
    xhttp.open("GET", url);
    xhttp.send();
  }
  function post_update(xhttp) {
  }
  function oninput_angle_slider() {
      document.getElementsByName("slider_angle_len")[0].innerHTML =
        "" + document.getElementsByName("angle_slider")[0].valueAsNumber;
  }
  function oninput_speed_slider() {
      document.getElementsByName("slider_speed_len")[0].innerHTML =
        "" + document.getElementsByName("speed_slider")[0].valueAsNumber;
  }
</script>
</head>
<body onload="load_body()">
<fieldset>
<legend>Raymarine Seatalk Instrument Settings</legend>
<table>
<tr><th>Type</th><th>Value</th></tr>
<tr><td>Local IP Address</td><td><label name="ipaddr">0.0.0.0</label></td></tr>
<tr><td>Serial Logger</td><td><input type="checkbox" name="slogger" onclick="pull_settings()"></td></tr>
<tr>
<td>Serial Logger Baud Rate</td>
<td><select name="slogger_baudrate" onchange="pull_settings()">
  <option value="9600">9600</option>
  <option value="19200">19200</option>
  <option value="38400">38400</option>
  <option value="57600">57600</option>
  <option value="115200">115200</option></select>
</td></tr>
<tr>
<td>Telnet Logger</td><td><input type="checkbox" name="tlogger" onclick="push_settings()"></td></tr>
<tr><td>Hostname</td><td><input type="text" name="hostname" required
       minlength="3" maxlength="31" size="10"></td>
       <td><button type="button" onclick="push_settings()">Update</button></td></tr>
<td>NMEA Port</td><td><input type="number" name="server_port" ></input></td>
<td><button type="button" onclick="push_settings()">Update</button></td></tr>
<tr><td>NMEA Talker</td><td><input type="text" name="nmea_talker" required
       minlength="2" maxlength="2" size="10"></td>
       <td><button type="button" onclick="push_settings()">Update</button></td></tr>
<tr><td>Colorize Prettyprint</td><td><input type="checkbox" name="colorize" onclick="push_settings()"></td></tr>
<tr><td>Activity LED</td><td><input type="checkbox" name="led" onclick="push_settings()"></td></tr>
</table>
</fieldset>
<fieldset>
<legend>Wind Instrument Settings</legend>
<table>
<tr><th>Type</th><th>Value</th></tr>
<tr><td>Angle Filter Enable</td><td><input type="checkbox" name="filter_angle_enable" onclick="push_wind_settings()"></td></tr>
<tr><td>Angle Filter Lenght</td><td><input type="range" min="1" max="16" value="10" class="slider" name="angle_slider" onchange="push_wind_settings()" oninput="oninput_angle_slider()"></td>
<td><label name="slider_angle_len">0</label><td></tr>
<tr><td>Speed Filter Enable</td><td><input type="checkbox" name="filter_speed_enable" onclick="push_wind_settings()"></td></tr>
<tr><td>Speed Filter Lenght</td><td><input type="range" min="1" max="16" value="10" class="slider" name="speed_slider" onchange="push_wind_settings()" oninput="oninput_speed_slider()"></td>
<td><label name="slider_speed_len">0</label><td></tr>
</tr>
</table>
</fieldset>
<fieldset>
<legend>Supported Seatalk Commands</legend>
<table>
<tr><th>Code</th><th>Description</th></tr>
<tr><td>0x10</td><td>APPARENT WIND ANGLE</td></tr>
<tr><td>0x11</td><td>APPARENT WIND SPEED</td></tr>
<tr><td>0x30</td><td>LAMP INTENSITY</td></tr>
</tr>
</table>
</fieldset>
</body>
</html>
)=====";
