var serialport = null;
var State = 'IDLE';
var Command = 0;

exports.setUp = function ()
{
	var sp_class = require ('serialport').SerialPort;
	serialport = new sp_class ("/dev/ttyS0", {baudrate: 4800}, false);

	serialport.open (function (err)
	{
		if (err)
		{
			console.log ('Error during open:');
			console.dir (err);
			process.exit ();
		}

		console.log ('Serial port open');
	});  

	serialport.on ('data', function (data)
	{
		console.log ('Data received:'  + data.toString ('hex'));

		switch (State)
		{
			case 'IDLE':
				if (data[0] == 0xA5)
				{
					console.log ('Request for time set received');
					var buffer = new Buffer ([0x9B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]);
					serialport.write (buffer);
					console.log ('Time set sent');
					State = 'WAIT_TIME_ACK';
				}
				else
					console.log ('Unexpected data received in IDLE state');
				break;

			case 'WAIT_TIME_ACK':
				if (data[0] == 0x00)
					console.log ('Time ACK received');
				else
					console.log ('Bad time ACK  received');
				State = 'IDLE';
				break;

			case 'WAIT_ADDRESS_CHECKSUM':
				var buffer = new Buffer ([0x00]);
				serialport.write (buffer);
				console.log ('Address checksum confirmed');
				State = 'WAIT_ADDRESS_READY';
				break;

			case 'WAIT_ADDRESS_READY':
				if (data[0] != 0x55)
				{
					console.log ('Unexpected data received in WAIT_ADDRESS_READY state');
					State = 'IDLE';
					break;
				}

				var buffer = new Buffer ([0x06, Command]);
				serialport.write (buffer);
				console.log ('Function sent');
				State = 'WAIT_FUNCTION_CHECKSUM';
				break;

			case 'WAIT_FUNCTION_CHECKSUM':
				var buffer = new Buffer ([0x00]);
				serialport.write (buffer);
				console.log ('Function checksum confirmed');
				State = 'WAIT_FUNCTION_READY';
				break;

			case 'WAIT_FUNCTION_READY':
				if (data[0] != 0x55)
					console.log ('Unexpected data received in WAIT_FUNCTION_READY state');
				else
					console.log ('Function completed');
				State = 'IDLE';
				break;
		}
	});
};

exports.tearDown = function ()
{
	serialport.close ();
	console.log ('Serial port closed');
};

exports.turnOn = function ()
{
	Command = 0x62;
	var buffer = new Buffer ([0x04, 0x66]);
	serialport.write (buffer);
	console.log ('ON command started');
	State = 'WAIT_ADDRESS_CHECKSUM';
};

exports.turnOff = function ()
{
	Command = 0x63;
	var buffer = new Buffer ([0x04, 0x66]);
	serialport.write (buffer);
	console.log ('ON command started');
	State = 'WAIT_ADDRESS_CHECKSUM';
};
