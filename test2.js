var IR = require("./build/Release/IRremote.node");

var IRsend = new IR.IRsend();

var powerOn = [];

IRsend.sendRaw(0x68B92, 20, 38, IR.SEND_PIN_1, function() {
		console.log("Test Callback");
		});
