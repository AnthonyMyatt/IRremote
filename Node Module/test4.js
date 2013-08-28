// Load the IRremote module
var IR = require("./build/Release/IRremote.node");
var IRsend = new IR.IRsend();

// Load the device DB file
var fs = require('fs');
var file = __dirname + '/testDevices.json';

console.log("Reading Device File...");
var data = fs.readFileSync(file, 'utf8');
var devices = JSON.parse(data);

printMainMenu();

function printMainMenu()
{
	console.log("----------------------------");
	console.log("IRremote Test (1.0)");
	console.log("");
	console.log("Options:");
	console.log("1. Send Code");
	//console.log("2. Learn Code");
	console.log("Q. Exit");
	
	ask("<main>", /.+/, function(opt) {
		if (opt == "1")
		{
			printDeviceMenu();
		}
		else if (opt == "Q" || opt == "q")
		{
			process.exit();
		}
	});
}

function printDeviceMenu()
{
	console.log("----------------------------");
	console.log("Please select a device or 'R' to return to previous menu");
	console.log("Devices:");
	var i;
	for (i = 0; i < devices.length; i++)
	{
		console.log(i+1 + ". " + devices[i].name);
	}
	
	ask("<device>", /.+/, function(opt) {
		if (opt == "R" || opt == "r")
		{
			printMainMenu();
		}
		else if (parseInt(opt, 10)-1 < devices.length)
		{
			printCodeMenu(parseInt(opt, 10)-1);
		}
		else
		{
			console.log("Device '" + opt + "' is not a valid option");
			printDeviceMenu();
		}
	});
}

function printCodeMenu(deviceIdx)
{
	var device = devices[deviceIdx];
	console.log("--------------------------");
	console.log("Please select a code or 'R' to return to previous menu");
	console.log(device.name + " Codes:");
	var i;
	for (i = 0; i < device.codes.length; i++)
	{
		console.log(i+1 + ". " + device.codes[i].name);
	}

	ask("<codes>", /.+/, function(opt) {
		if (opt == "R" || opt == "r")
		{
			printDeviceMenu();
		}
		else if (parseInt(opt, 10)-1 < device.codes.length)
		{
			var code = device.codes[parseInt(opt, 10)-1];
			if (code.type == "NEC")
			{
				IRsend.sendNEC(parseInt(code.data, 16), code.bits, IR.SEND_PIN_3, function() {console.log("Sent");});
				printCodeMenu(deviceIdx);
			}
		}
		else
		{
			console.log("Code '" + opt + "' is not a valid option");
			printCodeMenu(deviceIdx);
		}
	});
}

function ask(question, format, callback) {
 var stdin = process.stdin, stdout = process.stdout;
 
 stdin.resume();
 stdout.write(question + ": ");
 
 stdin.once('data', function(data) {
   data = data.toString().trim();
 
   if (format.test(data)) {
     callback(data);
   } else {
     stdout.write("It should match: "+ format +"\n");
     ask(question, format, callback);
   }
 });
}
