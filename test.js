var IR = require("./build/Release/IRremote.node");

var IRsend = new IR.IRsend();

IRsend.sendNEC(0x68B92, 20, IR.SEND_PIN_1, function() {
		console.log("Test Callback");
		});
