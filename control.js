var x10 = require ('./x10.js');

process.on ('SIGINT', function ()
{
	console.log ('SIGINT received, starting exit');
	process.exit ();
});

process.on ('exit', function ()
{
	x10.tearDown ();
	console.log ('Closing down');
});

process.stdin.on ('data', function (data)
{
	switch (data.trim ())
	{
		case 'quit':
			process.exit ();
			break;

		case 'on':
			x10.turnOn ();
			break;

		case 'off':
			x10.turnOff ();
			break;
	}
});

x10.setUp ();

process.stdin.resume ();
process.stdin.setEncoding ('utf8');
