/* Test for LG MKJ40653802 TV Remote */

/* IR Data from: http://lirc.sourceforge.net/remotes/lg/MKJ40653802 */

var IR = require("./build/Release/IRremote.node");

var IRsend = new IR.IRsend();

var powerOn = [9061,4473,591,521,591,521,591,521,591,1660,591,521,591,521,591,521,591,521,591,1660,591,1660,591,1660,591,521,591,1660,591,1660,591,1660,591,1660,591,521,591,521,591,521,591,1660,591,521,591,521,591,521,591,521,591,1660,591,1660,591,1660,591,521,591,1660,591,1660,591,1660,591,1660,590];

IRsend.sendRaw(powerOn, 69, 38, IR.SEND_PIN_1, function() {
		console.log("Test Callback");
		});
