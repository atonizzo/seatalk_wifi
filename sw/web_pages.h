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

// All the HTML code is placed in FLASH memory.
const char homepage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <script>
  function load_body()
  {
    const xhttp_settings = new XMLHttpRequest();
    xhttp_settings.onload = function() {pull_settings(this);}
    xhttp_settings.open("GET", "/pull_settings");
    xhttp_settings.send();
  }
  function pull_settings(xhttp) {
    const urlParams = new URLSearchParams(xhttp.responseText)
    document.getElementById("fw_version").innerHTML = urlParams.get('fw_version')
    document.getElementById("wifi_power").valueAsNumber = parseInt(urlParams.get('wifi_power'), 10)
    document.getElementById("slider_wifi_power").innerHTML = "" + ((1.0 * parseInt(urlParams.get('wifi_power'), 10)) / 2) + "dBm"
    document.getElementById("ipaddr").innerHTML = urlParams.get('ipaddr')
    document.getElementById("slogger").checked = (parseInt(urlParams.get('slogger'), 10) == 1)
    document.getElementById("slogger_baudrate").selectedIndex = parseInt(urlParams.get('slogger_baudrate'), 10)
    document.getElementById("tlogger").checked = (parseInt(urlParams.get('tlogger'), 10) == 1)
    document.getElementById("hostname").value = urlParams.get('hostname')
    document.getElementById("nmea_port").valueAsNumber = parseInt(urlParams.get('nmea_port'), 10)
    document.getElementById("nmea_talker").value = urlParams.get('nmea_talker').substr(0, 2)
    document.getElementById("colorize").checked = (parseInt(urlParams.get('colorize'), 10) == 1)
    document.getElementById("led").checked = (parseInt(urlParams.get('led'), 10) == 1)
    document.getElementById("ota").checked = (parseInt(urlParams.get('ota'), 10) == 1)

    const xhttp_wind_settings = new XMLHttpRequest();
    xhttp_wind_settings.onload = function() {pull_wind_settings(this);}
    xhttp_wind_settings.open("GET", "/pull_wind_settings");
    xhttp_wind_settings.send();
  }
  function pull_wind_settings(xhttp) {
    const urlParams = new URLSearchParams(xhttp.responseText);
    var angle_filter_enabled = parseInt(urlParams.get('filter_angle_enable'), 10) == 1;
    document.getElementById("filter_angle_enable").checked = angle_filter_enabled;
    document.getElementById("angle_slider").attributes.max.value = parseInt(urlParams.get('filter_angle_max'), 10);
    document.getElementById("angle_slider").valueAsNumber = parseInt(urlParams.get('filter_angle_len'), 10);
    document.getElementById("slider_angle_len").innerHTML = "" + parseInt(urlParams.get('filter_angle_len'), 10)
    document.getElementById("angle_slider").disabled = speed_filter_enabled == false;

    var speed_filter_enabled = parseInt(urlParams.get('filter_speed_enable'), 10) == 1;
    document.getElementById("filter_speed_enable").checked = speed_filter_enabled == 1;
    document.getElementById("speed_slider").attributes.max.value = parseInt(urlParams.get('filter_speed_max'), 10);
    document.getElementById("speed_slider").valueAsNumber = parseInt(urlParams.get('filter_speed_len'), 10);
    document.getElementById("slider_speed_len").innerHTML = "" + parseInt(urlParams.get('filter_speed_len'), 10)
    document.getElementById("speed_slider").disabled = speed_filter_enabled == false;

    const xhttp_signalk_settings = new XMLHttpRequest();
    xhttp_signalk_settings.onload = function() {pull_signalk_settings(this);}
    xhttp_signalk_settings.open("GET", "/pull_signalk_settings");
    xhttp_signalk_settings.send();
  }
  function pull_signalk_settings(xhttp) {
    const urlParams = new URLSearchParams(xhttp.responseText);
    document.getElementById("signalk_udp_enable").checked = (parseInt(urlParams.get('signalk_udp_enable'), 10) == 1);
    document.getElementById("signalk_udp_ip_0").valueAsNumber = parseInt(urlParams.get('signalk_udp_ip_0'), 10);
    document.getElementById("signalk_udp_ip_1").valueAsNumber = parseInt(urlParams.get('signalk_udp_ip_1'), 10);
    document.getElementById("signalk_udp_ip_2").valueAsNumber = parseInt(urlParams.get('signalk_udp_ip_2'), 10);
    document.getElementById("signalk_udp_ip_3").valueAsNumber = parseInt(urlParams.get('signalk_udp_ip_3'), 10);
    document.getElementById("signalk_udp_port").valueAsNumber = parseInt(urlParams.get('signalk_udp_port'), 10);
  }
  function push_settings()
  {
    url = "/push_settings?wifi_power=" + document.getElementById("wifi_power").valueAsNumber
    url += "&slogger=" + ((document.getElementById("slogger").checked == true) ? 1 : 0)
    url += "&tlogger=" + ((document.getElementById("tlogger").checked == true) ? 1 : 0)
    url += "&slogger_baud=" + document.getElementById("slogger_baudrate").selectedIndex
    url += "&nmea_port=" + document.getElementById("nmea_port").value
    url += "&hostname=" + document.getElementById("hostname").value
    url += "&nmea_talker=" + document.getElementById("nmea_talker").value.substr(0, 2)
    url += "&colorize=" + ((document.getElementById("colorize").checked == true) ? 1 : 0)
    url += "&led=" + ((document.getElementById("led").checked == true) ? 1 : 0)
    url += "&ota=" + ((document.getElementById("ota").checked == true) ? 1 : 0)
    const xhttp=new XMLHttpRequest();
    xhttp.onload = function() {post_update(this);}
    xhttp.open("GET", url);
    xhttp.send();
  }
  function push_wind_settings()
  {
    url = "/push_wind_settings?filter_angle_enable=" + ((document.getElementById("filter_angle_enable").checked == true) ? 1 : 0)
    document.getElementById("angle_slider").disabled = (document.getElementById("filter_angle_enable").checked == false)
    url += "&filter_speed_enable=" + ((document.getElementById("filter_speed_enable").checked == true) ? 1 : 0)
    document.getElementById("speed_slider").disabled = (document.getElementById("filter_speed_enable").checked == false)
    url += "&angle_length=" + document.getElementById("angle_slider").valueAsNumber
    url += "&speed_length=" + document.getElementById("speed_slider").valueAsNumber
    const xhttp=new XMLHttpRequest();
    xhttp.onload = function() {post_update(this);}
    xhttp.open("GET", url);
    xhttp.send();
  }
  function push_signalk_settings()
  {
    url = "/push_signalk_settings?signalk_udp_enable="
    url += ((document.getElementById("signalk_udp_enable").checked == true) ? 1 : 0)
    url += "&signalk_udp_ip_0=" + document.getElementById("signalk_udp_ip_0").value
    url += "&signalk_udp_ip_1=" + document.getElementById("signalk_udp_ip_1").value
    url += "&signalk_udp_ip_2=" + document.getElementById("signalk_udp_ip_2").value
    url += "&signalk_udp_ip_3=" + document.getElementById("signalk_udp_ip_3").value
    url += "&signalk_udp_port=" + document.getElementById("signalk_udp_port").value
    const xhttp=new XMLHttpRequest();
    xhttp.onload = function() {post_update(this);}
    xhttp.open("GET", url);
    xhttp.send();
  }
  function post_update(xhttp) {
  }
  function oninput_angle_slider() {
      document.getElementById("slider_angle_len").innerHTML =
        "" + document.getElementById("angle_slider").valueAsNumber;
  }
  function oninput_speed_slider() {
      document.getElementById("slider_speed_len").innerHTML =
        "" + document.getElementById("speed_slider").valueAsNumber;
  }
  function oninput_wifi_slider() {
      document.getElementById("slider_wifi_power").innerHTML =
        "" + (1.0 * document.getElementById("wifi_power").valueAsNumber / 2) + " dBm";
  }
</script>
</head>
<body onload="load_body()">
<fieldset>
<legend>Raymarine Seatalk Instrument Settings</legend>
<table>
<tr><td>Firmware Version</td><td><label id="fw_version">1.0</label><td></tr>
<tr><td>WiFi Power</td><td><input type="range" min="0" max="41" value="10" class="slider" id="wifi_power" onchange="push_settings()" oninput="oninput_wifi_slider()"></td>
<td><label id="slider_wifi_power">0</label><td></tr>
<tr><td>Local IP Address</td><td><label id="ipaddr">0.0.0.0</label></td></tr>
<tr><td>Serial Logger</td><td><input type="checkbox" id="slogger" onclick="push_settings()"></td></tr>
<tr>
<td>Serial Logger Baud Rate</td>
<td><select id="slogger_baudrate" onchange="push_settings()">
  <option value="9600">9600</option>
  <option value="19200">19200</option>
  <option value="38400">38400</option>
  <option value="57600">57600</option>
  <option value="115200">115200</option></select>
</td></tr>
<tr>
<td>Telnet Logger</td><td><input type="checkbox" id="tlogger" onclick="push_settings()"></td></tr>
<tr><td>Hostname</td><td><input type="text" id="hostname" required minlength="3" maxlength="31" size="10"></td>
       <td><button type="button" onclick="push_settings()">Update</button></td></tr>
<td>NMEA Port</td><td><input type="number" max="1" max="65535" id="nmea_port" ></input></td>
<td><button type="button" onclick="push_settings()">Update</button></td></tr>
<tr><td>NMEA Talker</td><td><input type="text" id="nmea_talker" required
       minlength="2" maxlength="2" size="10"></td>
       <td><button type="button" onclick="push_settings()">Update</button></td></tr>
<tr><td>Colorize Prettyprint</td><td><input type="checkbox" id="colorize" onclick="push_settings()"></td></tr>
<tr><td>Activity LED</td><td><input type="checkbox" id="led" onclick="push_settings()"></td></tr>
<tr><td>Enable OTA</td><td><input type="checkbox" id="ota" onclick="push_settings()"></td></tr>
</table>
</fieldset>
<fieldset>
<legend>Wind Instrument Settings</legend>
<table>
<tr><td>Angle Filter Enable</td><td><input type="checkbox" id="filter_angle_enable" onclick="push_wind_settings()"></td></tr>
<tr><td>Angle Filter Lenght</td><td><input type="range" min="1" max="16" value="10" class="slider" id="angle_slider" onchange="push_wind_settings()" oninput="oninput_angle_slider()"></td>
<td><label id="slider_angle_len">0</label><td></tr>
<tr><td>Speed Filter Enable</td><td><input type="checkbox" id="filter_speed_enable" onclick="push_wind_settings()"></td></tr>
<tr><td>Speed Filter Lenght</td><td><input type="range" min="1" max="16" value="10" class="slider" id="speed_slider" onchange="push_wind_settings()" oninput="oninput_speed_slider()"></td>
<td><label id="slider_speed_len">0</label><td></tr>
</tr>
</table>
</fieldset>
<fieldset>
<legend>SignalK UDP Integration</legend>
<table>
<tr><td>Send JSON data to SignalK</td><td><input type="checkbox" id="signalk_udp_enable" onclick="push_signalk_settings()"></td></tr>
<tr>
<td>SignalK IP Address</td>
<td><input type="number" min="0" max="255" value="0" id="signalk_udp_ip_0" onchange="push_signalk_settings()"></td><td>.</td>
<td><input type="number" min="0" max="255" value="0" id="signalk_udp_ip_1" onchange="push_signalk_settings()"></td><td>.</td>
<td><input type="number" min="0" max="255" value="0" id="signalk_udp_ip_2" onchange="push_signalk_settings()"</td><td>.</td>
<td><input type="number" min="0" max="255" value="0" id="signalk_udp_ip_3" onchange="push_signalk_settings()"</td>
</tr>
<tr>
<td>SignalK IP Port</td>
<td><input type="number" min="0" max="255" value="0" id="signalk_udp_port" onchange="push_signalk_settings()"></td></tr>
</table>
</fieldset>
</fieldset>
<fieldset>
<legend>Supported Seatalk Sentences</legend>
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

// https://canvas-gauges.com/
// https://github.com/Mikhus/canvas-gauges
// Canvas Gauges under MIT LIcense.
const char gauges[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<script>
(function(b){function m(c,d){d||(d=b);if("undefined"!==typeof d[c])return d[c];for(var f=["webkit","moz","ms","o"],h=0,e=f.length,q=c.charAt(0).toUpperCase()+c.substr(1);h<e;h++){var k=d[f[h]+q];if("undefined"!==typeof k)return k}return null}function s(b,d,f,h,e,q){b-=f;var k=b/e;1<k&&(k=1);d&&d(1===k?k:h(k));b<e?requestAnimationFrame(function(b){s(b,d,f,h,e,q)}):q&&q()}function u(b,d,f,m){if(!(this instanceof u))return new u(b,d,f,m);this.duration=d||250;this.rule=h[b]||b||"linear";this.draw=f||
function(){};this.end=m||function(){};if("function"!==typeof this.rule)throw new TypeError("Invalid animation rule:",b);}b.requestAnimationFrame=m("requestAnimationFrame")||function(b){setTimeout(function(){b((new Date).getTime())},1E3/60)};var h={linear:function(b){return b},quad:function(b){return Math.pow(b,2)},dequad:function(b){return 1-h.quad(1-b)},quint:function(b){return Math.pow(b,5)},dequint:function(b){return 1-Math.pow(1-b,5)},cycle:function(b){return 1-Math.sin(Math.acos(b))},decycle:function(b){return Math.sin(Math.acos(1-
b))},bounce:function(b){return 1-h.debounce(1-b)},debounce:function(b){for(var d=0,f=1;;d+=f,f/=2)if(b>=(7-4*d)/11)return-Math.pow((11-6*d-11*b)/4,2)+Math.pow(f,2)},elastic:function(b){return 1-h.delastic(1-b)},delastic:function(b){return Math.pow(2,10*(b-1))*Math.cos(30*Math.PI/3*b)}};u.prototype.animate=function(c,d){var f=this,h=m("animationStartTime")||b.performance&&b.performance.now?b.performance.now():Date.now();c=c||this.draw;d=d||this.end;requestAnimationFrame(function(b){s(b,c,h,f.rule,
f.duration,d)})};u.rules=h;b.Animation=u})(window);
var Gauge=function(b){function m(b,a){for(var c in a)a.hasOwnProperty(c)&&("object"==typeof a[c]&&"[object Array]"!==Object.prototype.toString.call(a[c])&&"renderTo"!=c?("object"!=typeof b[c]&&(b[c]={}),m(b[c],a[c])):b[c]=a[c])}function s(){x.width=b.width*v;x.height=b.height*v;x.style.width=b.width+"px";x.style.height=b.height+"px";B=x.cloneNode(!0);y=B.getContext("2d");C=x.width;D=x.height;z=C/2;A=D/2;g=z<A?z:A;B.i8d=!1;y.translate(z,A);y.save();a.translate(z,A);a.save()}function u(){var a=E-t,
c=t;G.animate(function(d){t=parseFloat(c)+a*d;b.updateValueOnAnimation?k.setRawValue(t):k.draw()})}function h(b){return b*Math.PI/180}function c(b,c,d){d=a.createLinearGradient(0,0,0,d);d.addColorStop(0,b);d.addColorStop(1,c);return d}function d(a){var c=!1;a=0===b.majorTicksFormat.dec?Math.round(a).toString():a.toFixed(b.majorTicksFormat.dec);return 1<b.majorTicksFormat["int"]?(c=-1<a.indexOf("."),-1<a.indexOf("-")?"-"+(b.majorTicksFormat["int"]+b.majorTicksFormat.dec+2+(c?1:0)-a.length)+a.replace("-",
""):""+(b.majorTicksFormat["int"]+b.majorTicksFormat.dec+1+(c?1:0)-a.length)+a):a}function f(a){var c=b.valueFormat.dec,d=b.valueFormat["int"],e=0;a=parseFloat(a);var g=0>a;a=Math.abs(a);if(0<c){a=a.toFixed(c).toString().split(".");for(c=d-a[0].length;e<c;++e)a[0]="0"+a[0];a=(g?"-":"")+a[0]+"."+a[1]}else{a=Math.round(a).toString();for(c=d-a.length;e<c;++e)a="0"+a;a=(g?"-":"")+a}return a}function w(a,b){var c=Math.sin(b),d=Math.cos(b);return{x:0*d-a*c,y:0*c+a*d}}function e(){var d=g/100*b.needle.circle.size,
e=0.75*g/100*b.needle.circle.size,f=g/100*b.needle.end,n=b.needle.start?g/100*b.needle.start:0,l=20*(g/100),p=g/100*b.needle.width,r=g/100*b.needle.width/2,k=function(){a.shadowOffsetX=2;a.shadowOffsetY=2;a.shadowBlur=10;a.shadowColor=b.colors.needle.shadowDown};k();a.save();a.rotate(h(b.startAngle+(t-b.minValue)/(b.maxValue-b.minValue)*b.ticksAngle));"arrow"===b.needle.type?(a.beginPath(),a.moveTo(-r,-l),a.lineTo(-p,0),a.lineTo(-1*v,f),a.lineTo(1*v,f),a.lineTo(p,0),a.lineTo(r,-l),a.closePath(),a.fillStyle=
c(b.colors.needle.start,b.colors.needle.end,f-l),a.fill(),a.beginPath(),a.lineTo(-0.5*v,f),a.lineTo(-1*v,f),a.lineTo(-p,0),a.lineTo(-r,-l),a.lineTo(r/2*v-2*v,-l),a.closePath(),a.fillStyle=b.colors.needle.shadowUp):(a.beginPath(),a.moveTo(-r,f),a.lineTo(-r,n),a.lineTo(r,n),a.lineTo(r,f),a.closePath(),a.fillStyle=c(b.colors.needle.start,b.colors.needle.end,f-l));a.fill();a.restore();b.needle.circle&&(k(),b.needle.circle.outer&&(a.beginPath(),a.arc(0,0,d,0,2*Math.PI,!0),a.fillStyle=c(b.colors.needle.circle.outerStart,
b.colors.needle.circle.outerEnd,d),a.fill(),a.restore()),b.needle.circle.inner&&(a.beginPath(),a.arc(0,0,e,0,2*Math.PI,!0),a.fillStyle=c(b.colors.needle.circle.innerStart,b.colors.needle.circle.innerEnd,e),a.fill()))}Gauge.Collection.push(this);this.config={renderTo:null,width:200,height:200,title:!1,maxValue:100,minValue:0,majorTicks:[],minorTicks:10,ticksAngle:270,startAngle:45,strokeTicks:!0,units:!1,updateValueOnAnimation:!1,valueFormat:{"int":3,dec:2},majorTicksFormat:{"int":1,dec:0},glow:!0,
animation:{delay:10,duration:250,fn:"cycle"},colors:{plate:"#fff",majorTicks:"#444",minorTicks:"#666",title:"#888",units:"#888",numbers:"#444",needle:{start:"rgba(240, 128, 128, 1)",end:"rgba(255, 160, 122, .9)",circle:{outerStart:"#f0f0f0",outerEnd:"#ccc",innerStart:"#e8e8e8",innerEnd:"#f5f5f5"},shadowUp:"rgba(2, 255, 255, 0.2)",shadowDown:"rgba(188, 143, 143, 0.45)"},valueBox:{rectStart:"#888",rectEnd:"#666",background:"#babab2",shadow:"rgba(0, 0, 0, 1)"},valueText:{foreground:"#444",shadow:"rgba(0, 0, 0, 0.3)"},
circle:{shadow:"rgba(0, 0, 0, 0.5)",outerStart:"#ddd",outerEnd:"#aaa",middleStart:"#eee",middleEnd:"#f0f0f0",innerStart:"#fafafa",innerEnd:"#ccc"}},needle:{type:"arrow",start:0,end:77,width:4,circle:{size:10,inner:!0,outer:!0}},circles:{outerVisible:!0,middleVisible:!0,innerVisible:!0},valueBox:{visible:!0},valueText:{visible:!0},highlights:[{from:20,to:60,color:"#eee"},{from:60,to:80,color:"#ccc"},{from:80,to:100,color:"#999"}]};var q=0,k=this,t=0,E=0,F=!1;this.setValue=function(a){t=b.animation?
q:a;var c=(b.maxValue-b.minValue)/100;E=a>b.maxValue?b.maxValue+c:a<b.minValue?b.minValue-c:a;q=a;b.animation?u():this.draw();return this};this.setRawValue=function(a){t=q=a;this.draw();return this};this.clear=function(){q=t=E=this.config.minValue;this.draw();return this};this.getValue=function(){return q};this.onready=function(){};m(this.config,b);b.startAngle=parseInt(b.startAngle,10);b.ticksAngle=parseInt(b.ticksAngle,10);isNaN(b.startAngle)&&(b.startAngle=45);isNaN(b.ticksAngle)&&(b.ticksAngle=
270);360<b.ticksAngle&&(b.ticksAngle=360);0>b.ticksAngle&&(b.ticksAngle=0);0>b.startAngle&&(b.startAngle=0);360<b.startAngle&&(b.startAngle=360);this.config.minValue=parseFloat(this.config.minValue);this.config.maxValue=parseFloat(this.config.maxValue);b=this.config;t=q=b.minValue;if(!b.renderTo)throw Error("Canvas element was not specified when creating the Gauge object!");var x=b.renderTo.tagName?b.renderTo:document.getElementById(b.renderTo),a=x.getContext("2d"),v=window.devicePixelRatio||1,B,
C,D,z,A,g,y;s();this.updateConfig=function(a){m(this.config,a);s();this.setRawValue(q||0);return this};var G=new window.Animation(b.animation.fn,b.animation.duration);a.lineCap="round";this.isWaitingForInitialization=!1;this.drawWhenInitialized=function(){if(!Gauge.initialized)return this.isWaitingForInitialization=!0,!1;this.isWaitingForInitialization=!1;if(b.valueText.visible){a.save();a.font=40*(g/200)+"px Led";var c=f(q),d=g-33*(g/100);a.save();if(b.valueBox.visible){var h=0.12*g,n=a.measureText("-"+
f(0)).width,l=-n/2-0.025*g,p=d-h-0.04*g,n=n+0.05*g,h=h+0.07*g,r=0.025*g;a.beginPath();a.moveTo(l+r,p);a.lineTo(l+n-r,p);a.quadraticCurveTo(l+n,p,l+n,p+r);a.lineTo(l+n,p+h-r);a.quadraticCurveTo(l+n,p+h,l+n-r,p+h);a.lineTo(l+r,p+h);a.quadraticCurveTo(l,p+h,l,p+h-r);a.lineTo(l,p+r);a.quadraticCurveTo(l,p,l+r,p);a.closePath()}l=a.createRadialGradient(0,d-0.12*g-0.025*g+(0.12*g+0.045*g)/2,g/10,0,d-0.12*g-0.025*g+(0.12*g+0.045*g)/2,g/5);l.addColorStop(0,b.colors.valueBox.rectStart);l.addColorStop(1,b.colors.valueBox.rectEnd);
a.strokeStyle=l;a.lineWidth=0.05*g;a.stroke();a.shadowBlur=0.012*g;a.shadowColor=b.colors.valueBox.shadow;a.fillStyle=b.colors.valueBox.background;a.fill();a.restore();a.shadowOffsetX=0.004*g;a.shadowOffsetY=0.004*g;a.shadowBlur=0.012*g;a.shadowColor=b.colors.valueText.shadow;a.fillStyle=b.colors.valueText.foreground;a.textAlign="center";a.fillText(c,-0,d);a.restore()}e();F||(k.onready&&k.onready(),y.scale(v,v),y.save(),F=!0);return!0};this.draw=function(){if(!B.i8d){y.clearRect(-z,-A,C,D);y.save();
var e={ctx:a};a=y;var f=93*(g/100),k=g-f,n=91*(g/100),l=88*(g/100),p=85*(g/100);a.save();b.glow&&(a.shadowBlur=k,a.shadowColor=b.colors.circle.shadow);b.circles.outerVisible&&(a.beginPath(),a.arc(0,0,f,0,2*Math.PI,!0),a.fillStyle=c(b.colors.circle.outerStart,b.colors.circle.outerEnd,f),a.fill());a.restore();b.circles.middleVisible&&(a.beginPath(),a.arc(0,0,n,0,2*Math.PI,!0),a.fillStyle=c(b.colors.circle.middleStart,b.colors.circle.middleEnd,n),a.fill());b.circles.innerVisible&&(a.beginPath(),a.arc(0,
0,l,0,2*Math.PI,!0),a.fillStyle=c(b.colors.circle.innerStart,b.colors.circle.innerEnd,l),a.fill());a.beginPath();a.arc(0,0,p,0,2*Math.PI,!0);a.fillStyle=b.colors.plate;a.fill();a.save();a.save();f=81*(g/100);k=f-15*(g/100);n=0;for(l=b.highlights.length;n<l;n++){var p=b.highlights[n],r=(b.maxValue-b.minValue)/b.ticksAngle,t=h(b.startAngle+(p.from-b.minValue)/r),r=h(b.startAngle+(p.to-b.minValue)/r);a.beginPath();a.rotate(h(90));a.arc(0,0,f,t,r,!1);a.restore();a.save();var m=w(k,t),q=w(f,t);a.moveTo(m.x,
m.y);a.lineTo(q.x,q.y);var q=w(f,r),s=w(k,r);a.lineTo(q.x,q.y);a.lineTo(s.x,s.y);a.lineTo(m.x,m.y);a.closePath();a.fillStyle=p.color;a.fill();a.beginPath();a.rotate(h(90));a.arc(0,0,k,t-0.2,r+0.2,!1);a.restore();a.closePath();a.fillStyle=b.colors.plate;a.fill();a.save()}f=81*(g/100);a.lineWidth=1*v;a.strokeStyle=b.colors.minorTicks;a.save();k=b.minorTicks*(b.majorTicks.length-1);for(n=0;n<k;++n)a.rotate(h(b.startAngle+n*(b.ticksAngle/k))),a.beginPath(),a.moveTo(0,f),a.lineTo(0,f-7.5*(g/100)),a.stroke(),
a.restore(),a.save();f=81*(g/100);k=0;n=b.majorTicks.length;a.lineWidth=2*v;a.strokeStyle=b.colors.majorTicks;a.save();if(0===n){for(l=(b.maxValue-b.minValue)/5;5>k;k++)b.majorTicks.push(d(b.minValue+l*k));b.majorTicks.push(d(b.maxValue))}for(k=0;k<n;++k)a.rotate(h(b.startAngle+k*(b.ticksAngle/(n-1)))),a.beginPath(),a.moveTo(0,f),a.lineTo(0,f-15*(g/100)),a.stroke(),a.restore(),a.save();b.strokeTicks&&(a.rotate(h(90)),a.beginPath(),a.arc(0,0,f,h(b.startAngle),h(b.startAngle+b.ticksAngle),!1),a.stroke(),
a.restore(),a.save());f=55*(g/100);k={};n=0;for(l=b.majorTicks.length;n<l;++n)p=b.startAngle+n*(b.ticksAngle/(l-1)),t=w(f,h(p)),360===p&&(p=0),k[p]||(k[p]=!0,a.font=20*(g/200)+"px Arial",a.fillStyle=b.colors.numbers,a.lineWidth=0,a.textAlign="center",a.fillText(b.majorTicks[n],t.x,t.y+3));b.title&&(a.save(),a.font=24*(g/200)+"px Arial",a.fillStyle=b.colors.title,a.textAlign="center",a.fillText(b.title,0,-g/4.25),a.restore());b.units&&(a.save(),a.font=22*(g/200)+"px Arial",a.fillStyle=b.colors.units,
a.textAlign="center",a.fillText(b.units,0,g/3.25),a.restore());B.i8d=!0;a=e.ctx;delete e.ctx}a.clearRect(-z,-A,C,D);a.save();a.drawImage(B,-z,-A,C,D);this.drawWhenInitialized()}};Gauge.initialized=!1;
(function(){function b(){Gauge.initialized=!0;for(var b=0;b<Gauge.Collection.length;++b)Gauge.Collection[b].isWaitingForInitialization&&Gauge.Collection[b].drawWhenInitialized()}function m(){var d=s.getElementsByTagName("head")[0],m="@font-face { font-family: '"+c+"'; src: "+h+"; }",e=s.createElement("style");e.type="text/css";if(u)d.appendChild(e),d=e.styleSheet,d.cssText=m;else{try{e.appendChild(s.createTextNode(m))}catch(q){e.cssText=m}d.appendChild(e)}var k=setInterval(function(){if(s.body){clearInterval(k);
var d=s.createElement("div");d.style.fontFamily=c;d.style.position="absolute";d.style.height=d.style.width=0;d.style.overflow="hidden";d.innerHTML=".";s.body.appendChild(d);setTimeout(function(){b();d.parentNode.removeChild(d)},250)}},1)}var s=document,u=-1!=navigator.userAgent.toLocaleLowerCase().indexOf("msie"),h="url('"+(window.CANV_GAUGE_FONTS_PATH||"fonts")+"/digital-7-mono."+(u?"eot":"ttf")+"')",c="Led";if(void 0===document.fonts)m();else{var d=new window.FontFace(c,h);document.fonts.add(d);
d.load().then(function(){b()},function(b){window.console&&window.console.log(b);m()})}})();Gauge.Collection=[];Gauge.Collection.get=function(b){if("string"==typeof b)for(var m=0,s=this.length;m<s;m++){if((this[m].config.renderTo.tagName?this[m].config.renderTo:document.getElementById(this[m].config.renderTo||"")).getAttribute("id")==b)return this[m]}else return"number"==typeof b?this[b]:null};
function domReady(b){window.addEventListener?window.addEventListener("DOMContentLoaded",b,!1):window.attachEvent("onload",b)}
domReady(function(){function b(b){for(var c=b[0],d=1,a=b.length;d<a;d++)c+=b[d].substr(0,1).toUpperCase()+b[d].substr(1,b[d].length-1);return c}for(var m=document.getElementsByTagName("canvas"),s=0,u=m.length;s<u;s++)if("canv-gauge"==m[s].getAttribute("data-type")){var h=m[s],c={},d,f=parseInt(h.getAttribute("width"),10),w=parseInt(h.getAttribute("height"),10);c.renderTo=h;f&&(c.width=f);w&&(c.height=w);f=0;for(w=h.attributes.length;f<w;f++)if(d=h.attributes.item(f).nodeName,"data-type"!=d&&"data-"==
d.substr(0,5)){var e=d.substr(5,d.length-5).toLowerCase().split("-");if(d=h.getAttribute(d))switch(e[0]){case "needle":c.needle||(c.needle={});"circle"==e[1]?(c.needle.circle||(c.needle.circle={}),e[2]?c.needle.circle[e[2]]="false"===d?!1:d:c.needle.circle="false"==d?!1:d):c.needle[e[1]]=d;break;case "ticksangle":c.ticksAngle=parseInt(d,10);360<c.ticksAngle&&(c.ticksAngle=360);0>c.ticksAngle&&(c.ticksAngle=0);break;case "startangle":c.startAngle=parseInt(d,10);360<c.startAngle&&(c.startAngle=360);
0>c.startAngle&&(c.startAngle=0);break;case "colors":if(e[1])if(c.colors||(c.colors={}),c.colors.needle||(c.colors.needle={}),"needle"==e[1]){if(e[2])switch(e[2]){case "start":c.colors.needle.start=d;break;case "end":c.colors.needle.end=d}else{var q=d.split(/\s+/);q[0]&&q[1]?(c.colors.needle.start=q[0],c.colors.needle.end=q[1]):c.colors.needle.start=d}if(e[2])if(c.colors.needle.circle||(c.colors.needle.circle={}),"circle"===e[2]){switch(e[3]){case "outerstart":c.colors.needle.circle.outerStart=d;
break;case "outerend":c.colors.needle.circle.outerEnd=d;break;case "innerstart":c.colors.needle.circle.innerStart=d;break;case "innerend":c.colors.needle.circle.innerEnd=d;break}c.colors.needle.shadowUp=d}else"shadowup"===e[2]?c.colors.needle.shadowUp=d:"shadowdown"===e[2]&&(c.colors.needle.shadowDown=d)}else if("valuebox"==e[1]){if(c.colors.valueBox||(c.colors.valueBox={}),e[2])switch(e[2]){case "rectstart":c.colors.valueBox.rectStart=d;break;case "rectend":c.colors.valueBox.rectEnd=d;break;case "background":c.colors.valueBox.background=
d;break;case "shadow":c.colors.valueBox.shadow=d}}else if("valuetext"==e[1]){if(c.colors.valueText||(c.colors.valueText={}),e[2])switch(e[2]){case "foreground":c.colors.valueText.foreground=d;break;case "shadow":c.colors.valueText.shadow=d}}else if("circle"==e[1]){if(c.colors.circle||(c.colors.circle={}),e[2])switch(e[2]){case "shadow":c.colors.circle.shadow=d;break;case "outerstart":c.colors.circle.outerStart=d;break;case "outerend":c.colors.circle.outerEnd=d;break;case "middlestart":c.colors.circle.middleStart=
d;break;case "middleend":c.colors.circle.middleEnd=d;break;case "innerstart":c.colors.circle.innerStart=d;break;case "innerend":c.colors.circle.innerEnd=d}}else e.shift(),c.colors[b(e)]=d;break;case "circles":c.circles||(c.circles={});if(e[1])switch(e[1]){case "outervisible":c.circles.outerVisible="true"===d.toLowerCase();break;case "middlevisible":c.circles.middleVisible="true"===d.toLowerCase();break;case "innervisible":c.circles.innerVisible="true"===d.toLowerCase()}break;case "valuebox":c.valueBox||
(c.valueBox={});if(e[1])switch(e[1]){case "visible":c.valueBox.visible="true"===d.toLowerCase()}break;case "valuetext":c.valueText||(c.valueText={});if(e[1])switch(e[1]){case "visible":c.valueText.visible="true"===d.toLowerCase()}break;case "highlights":if("false"===d){c.highlights=[];break}c.highlights||(c.highlights=[]);e=d.match(/(?:(?:-?\d*\.)?(-?\d+){1,2} ){2}(?:(?:#|0x)?(?:[0-9A-F|a-f]){3,8}|rgba?\(.*?\))/g);d=0;for(q=e.length;d<q;d++){var k=e[d].replace(/^\s+|\s+$/g,"").split(/\s+/),t={};k[0]&&
""!=k[0]&&(t.from=k[0]);k[1]&&""!=k[1]&&(t.to=k[1]);k[2]&&""!=k[2]&&(t.color=k[2]);c.highlights.push(t)}break;case "animation":e[1]&&(c.animation||(c.animation={}),"fn"==e[1]&&/^\s*function\s*\(/.test(d)&&(d=eval("("+d+")")),c.animation[e[1]]=d);break;default:e=b(e);if("onready"==e)continue;if("majorTicks"==e)d=d.split(/\s+/);else if("strokeTicks"==e||"glow"===e)d="true"===d;else if("valueFormat"==e)if(d=d.split("."),2==d.length)d={"int":parseInt(d[0],10),dec:parseInt(d[1],10)};else continue;else"updateValueOnAnimation"===
e&&(d="true"===d);c[e]=d}}c=new Gauge(c);h.getAttribute("data-value")&&c.setRawValue(parseFloat(h.getAttribute("data-value")));h.getAttribute("data-onready")&&(c.onready=function(){eval(this.config.renderTo.getAttribute("data-onready"))});c.draw()}});window.Gauge=Gauge;
</script>
</head>
<body onload="create_gauge()">
<script>
var gauge = []
var dimension = [300, 300]
var log_index = [-1, -1]
const log_maxlen = [400, 400]
var data_log = [[], []]
var interval = [2, 4]
var minValue = [0, 0]
var maxValue = [90, 360]

function update_display(gauge, bins) {
  for (var i = 0; i < bins.length; i++) {
    if (bins[i].count >= 70)
      gauge.config.highlights[i].color = "rgba(255, 0, 0, 1)"
    else {
      if (bins[i].count >= 60)
        gauge.config.highlights[i].color = "rgba(240, 5, 5, 1)"
      else {
        if (bins[i].count >= 50)
          gauge.config.highlights[i].color = "rgba(255, 44, 5, 1)"
        else {
          if (bins[i].count >= 40)
            gauge.config.highlights[i].color = "rgba(253, 97, 4, 1)"
          else {
            if (bins[i].count >= 30)
              gauge.config.highlights[i].color = "rgba(253, 154, 1, 1)"
            else {
              if (bins[i].count >= 20)
                gauge.config.highlights[i].color = "rgba(255, 206, 3, 1)"
              else {
                if (bins[i].count >= 10)
                  gauge.config.highlights[i].color = "rgba(254, 240, 1, 1)"
                else
                  gauge.config.highlights[i].color = "rgba(255, 255, 255, 1)"
              }
            }
          }
        }
      }
    }
  }
  gauge.updateConfig()
}

function compute_histogram(index, value) {
  gauge[index].setValue(value);
  gauge[index].updateConfig();

  log_index[index] += 1;
  if (log_index[index] == log_maxlen[index])
      log_index[index] = 0;
  data_log[index][log_index[index]] = value;

  var bins = [];
  var binCount = 0;
  for(var i = 0; i < maxValue[index]; i += interval[index])
  {
    bins.push({binNum: binCount, minNum: i, maxNum: i + interval[index], count: 0})
    binCount++;
  }
  // Loop through data and add to bin's count
  for (var i = 0; i < data_log[index].length; i++){
    for (var j = 0; j < bins.length; j++){
      if(data_log[index][i] >= bins[j].minNum && data_log[index][i] < bins[j].maxNum){
        bins[j].count++;
        break;  // An item can only be in one bin.
      }
    }
  }
  update_display(gauge[index], bins)
}

function update_histograms(xhttp) {
  const urlParams = new URLSearchParams(xhttp.responseText);
  var wind_speed = parseFloat(urlParams.get('wind_speed'))
  // Leading + returns a float not a string.
  compute_histogram(0, +wind_speed.toFixed(1))
  var wind_angle = parseFloat(urlParams.get('wind_angle'))
  if (wind_angle < 180)
    wind_angle += 180
  else
    wind_angle -= 180
  compute_histogram(1, +wind_angle.toFixed(1))
}

function pull_wind_data() {
  const wind_data = new XMLHttpRequest();
  wind_data.onload = function() {update_histograms(this)}
  wind_data.open("GET", "/pull_wind_data");
  wind_data.send();
}

function create_gauge() {
  gauge[0] = new Gauge({
    renderTo: 'gauge_speed',
    width: dimension[0],
    height: dimension[0],
    units: "Knots",
    title: "Wind Speed",
    minValue: minValue[0],
    maxValue: maxValue[0],
    majorTicks: [0, 10, 20, 30, 40, 50, 60, 70, 80, 90],
    valueFormat : {"int" : 2, "dec" : 1},
    colors: {
      plate: 'rgba(255, 255, 240, 1)',
      title: 'rgba(0, 0, 0, 1)',
      units: 'rgba(0, 0, 0, 1)',
      },
    animation: {duration: 1000, fn: 'linear'},
    valueBox: {visible: true},
    valueText: {visible: true},
  });
  gauge[0].config.highlights = []
  for(var i = 0; i < maxValue[0]; i += interval[0])
    gauge[0].config.highlights.push({"from": i, "to": i + interval[0], "color": "rgba(255, 255, 255, 1)"})
  gauge[0].setRawValue(20);
  gauge[0].draw();


  gauge[1] = new Gauge({
    renderTo: 'gauge_angle',
    width: dimension[1],
    height: dimension[1],
    title: "Wind Direction",
    minValue: minValue[1],
    maxValue: maxValue[1],
    ticksAngle: 360,
    startAngle: 0,
    majorTicks: ['S', 'SW', 'W', 'NW', 'N', 'NE', 'E', 'SE', 'S'],
    valueFormat : {"int" : 2, "dec" : 1},
    colors: {
      plate: 'rgba(255, 255, 240, 1)',
      title: 'rgba(0, 0, 0, 1)',
      units: 'rgba(0, 0, 0, 1)',
      },
    animation: {duration: 1000, fn: 'linear'},
    valueBox: {visible: false},
    valueText: {visible: false},
  });

  gauge[1].config.highlights = []
  for(var i = 0; i < maxValue[1]; i += interval[1])
    gauge[1].config.highlights.push({"from": i, "to": i + interval[1], "color": "rgba(255, 255, 255, 1)"})
  gauge[1].onready = function () {setInterval(pull_wind_data, 1000)};
  gauge[1].setRawValue(20);
  gauge[1].draw();

}
</script>
<canvas id="gauge_speed"></canvas>
<canvas id="gauge_angle"></canvas>
</body>
</html>
)=====";
