var IR = require("./build/Release/IRremote.node");
var IRsend = new IR.IRsend();

var timer = setInterval(test,100);

var i = 0;

function test()
{
	IRsend.sendNEC(0x20df10ef, 32, IR.SEND_PIN_3, function() {console.log("Callback");});
	i++;
	if (i > 3)
	{
		clearInterval(timer);
	}
}
