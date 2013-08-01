# IRremote for Beaglebone


## What is it?

**IRremote for Beaglebone** is a cape and relevant software that work together to make controlling things like television sets, DVD players, or any other IR controlled device with your Beaglebone or Beaglebone-Black.

### The Cape
The IR Cape consists of three LED outputs that can be connected to IR extender cables such as this one
<img src="http://anthonymyatt.github.io/images/ir_cable.jpg" width="150px" alt="IR Cable"/>
<br/>
and can then be used to control up to three devices (or more, by using one IR cable for multiple devices if they don't share IR codes).

### The Node.JS Module
The module for node.js is where the magic happens. With only three lines of javascript you can turn your TV on.

```javascript
var IR = require("./build/Release/IRremote.node");
var IRsend = new IR.IRsend();
IRsend.sendSony(0xa90, 12, IR.SEND_PIN_1, function() {});
```

## Recognition
#### IRremote for Arduino (Ken Shirriff)
The Node.JS module is based off of Ken Shirriff's IRremote library for the Arduino.

***Please note***: Although the function names are the same as Ken's library, the arguments that the functions take are different and the internal workings of the functions are different due to the differences in hardware.



## License

<a rel="license" href="http://creativecommons.org/licenses/by-sa/3.0/deed.en_US"><img alt="Creative Commons License" style="border-width:0" src="http://i.creativecommons.org/l/by-sa/3.0/88x31.png" /></a><br /><span xmlns:dct="http://purl.org/dc/terms/" property="dct:title">IRremote for Beaglebone</span> by <a xmlns:cc="http://creativecommons.org/ns#" href="https://anthonymyatt.net" property="cc:attributionName" rel="cc:attributionURL">Anthony Myatt</a> is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-sa/3.0/deed.en_US">Creative Commons Attribution-ShareAlike 3.0 Unported License</a>.

A copy of the license can be found within the LICENSE file that should be in the same directory as this README file. If the license file is not included, you can find a copy of the license on the Creative Commons website.