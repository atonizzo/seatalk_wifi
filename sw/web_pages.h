const char homepage[] = R"=====(
<html>
<head>
  <script>
  function load_body()
  {
    const xhttp=new XMLHttpRequest();
    xhttp.onload = function() {update_page(this);}
    xhttp.open("GET", "/get_status");
    xhttp.send();
  }
  function update_page(xhttp) {
    const urlParams = new URLSearchParams(xhttp.responseText);
    document.getElementsByName("ipaddr")[0].innerHTML = urlParams.get('ipaddr');
    const slogger = urlParams.get('slogger');
    document.getElementsByName("slogger")[0].checked = (parseInt(slogger, 10) == 1);
    const tlogger = urlParams.get('tlogger');
    document.getElementsByName("tlogger")[0].checked = (parseInt(tlogger, 10) == 1);
    const baud = urlParams.get('slogger_baudrate');
    document.getElementsByName("slogger_baudrate")[0].selectedIndex = parseInt(baud, 10);
    const port = urlParams.get('server_port');
    document.getElementsByName("server_port")[0].valueAsNumber = parseInt(port, 10);
    document.getElementsByName("hostname")[0].value = urlParams.get('hostname');
    const colorize = urlParams.get('colorize');
    document.getElementsByName("colorize")[0].checked = (parseInt(colorize, 10) == 1);
  }
  function update_values()
  {
    url = "/updater?slogger=";
    url += (document.getElementsByName("slogger")[0].checked == true) ? 1 : 0
    url += "&tlogger=";
    url += (document.getElementsByName("tlogger")[0].checked == true) ? 1 : 0
    url += "&slogger_baud="
    url += document.getElementsByName("slogger_baudrate")[0].selectedIndex
    url += "&server_port=" + document.getElementsByName("server_port")[0].value
    url += "&hostname=" + document.getElementsByName("hostname")[0].value
    url += "&colorize=";
    url += (document.getElementsByName("colorize")[0].checked == true) ? 1 : 0
    const xhttp=new XMLHttpRequest();
    xhttp.onload = function() {post_update(this);}
    xhttp.open("GET", url);
    xhttp.send();
  }
  function post_update(xhttp) {
  }
</script>
</head>
<body onload="load_body()">
<fieldset>
<legend>Raymarine Seatalk Instrument Settings</legend>
<table>
<tr><th>Type</th><th>Value</th></tr>
<tr><td>Local IP Address</td><td><label name="ipaddr">0.0.0.0</label></td></tr>
<tr><td>Serial Logger</td><td><input type="checkbox" name="slogger" onclick="update_values()"></td></tr>
<tr>
<td>Serial Logger Baud Rate</td>
<td><select name="slogger_baudrate" onchange="update_values()">
  <option value="9600">9600</option>
  <option value="19200">19200</option>
  <option value="38400">38400</option>
  <option value="57600">57600</option>
  <option value="115200">115200</option></select>
</td></tr>
<tr>
<td>Telnet Logger</td><td><input type="checkbox" name="tlogger" onclick="update_values()"></td></tr>
<tr><td>Hostname</td><td><input type="text" name="hostname" required
       minlength="3" maxlength="31" size="10"></td>
       <td><button type="button" onclick="update_values()">Update</button></td></tr>
<td>NMEA Port</td><td><input type="number" name="server_port" ></input></td>
<td><button type="button" onclick="update_values()">Update</button></td></tr>
<tr><td>Colorize Prettyprint</td><td><input type="checkbox" name="colorize" onclick="update_values()"></td></tr>
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
