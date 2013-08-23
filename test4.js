// Load the IRremote module
var IR = require("./build/Release/IRremote.node");
var IRsend = new IR.IRsend();

// Load the device DB file
var fs = require('fs');
var file = __dirname + '/testDevices.json';

console.log("Reading Device File...");
var devices = fs.readFileSync(file, 'utf8');

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
	for (i = 0; i != devices.count(); i++)
	{
		console.log(i+1 + ". " + devices[i].name);
	}
	
	ask("<device>", /.+/, function(opt) {
		if (opt == "R" || opt == "r")
		{
			printMainMenu();
		}
		else if (parseInt(opt, 10)-1 < devices.count())
		{
			
		}
		else
		{
			console.log("Device '" + opt + "' is not a valid option");
			printDeviceMenu();
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