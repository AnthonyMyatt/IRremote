var IR = require("./build/Release/IRremote.node");
var IRsend = new IR.IRsend();

setInterval(test,50);


function test()
{
	IRsend.sendNEC(0x20df10ef, 32, IR.SEND_PIN_1, function() {});
}
