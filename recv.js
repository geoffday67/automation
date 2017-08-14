function hex (b)
{
	var digits = '0123456789ABCDEF';
	var result = digits[Math.floor(b/16)];
	result += digits[b%16];
	return result;
}

var needle = require ('needle');

var serial = require ('serialport').SerialPort;
var port = new serial ('/dev/ttyUSB0', {baudrate: 2400});
port.on ('open', function ()
{
	console.log ('Port open');

	// Initialise state machine
	var state = 0;
	var packet = {};
	var previous_sequence = 0;

	port.on ('data', function (data)
	{
		for (var n = 0; n < data.length; n++)
		{
			//TODO HANDLE ESCAPED DATA, E.G. WITH MINI STATE MACHINE

			var b = data[n];
			//process.stdout.write ('0x' + hex (b) + ' ');

			// Whatever the state, watch for a start of packet byte so we can re-sync
			if (b == 0xFF)
			{
				packet = {data: []};
				state = 1;
				continue;
			}

			// Otherwise process each received byte through the state machine
			switch (state)
			{
			case 0:		// Idle
				break;

			case 1:		// Byte is address
				if (b & 0x80)
					packet.from_server = true;
				else
					packet.from_server = false;
				packet.address = (b & 0x7F);
				state = 2;
				break;

			case 2:		// Byte is sequence number
				packet.sequence = b;
				state = 3;
				break;

			case 3:		// Byte is packet type
				packet.type = b;
				state = 4;
				break;

			case 4:		// Byte is data specific to packet type; record until packet stop byte
				if (b != 0xFE)
				{
					packet.data.push (b);
					break;
				}

				// Complete packet received; check it's a different sequence from the last one
				//console.log (packet);
				if (packet.sequence != previous_sequence)
				{
					previous_sequence = packet.sequence;
					processPacket (packet);
				}

				state = 0;
				break;
			}
		}
	});
});

function processPacket (packet)
{
	switch (packet.type)
	{
	case 1:		// Wand type
		//console.log ('Wand: %d', packet.data[0]);
		if (packet.data[0] == 6 || packet.data[0] == 10)
			var command = 'on';
		else if (packet.data[0] == 5 || packet.data[0] == 13)
			var command = 'off';
		else
			break;

		if (packet.data[0] == 6 || packet.data[0] == 5)
			var channel = 2;
		else if (packet.data[0] == 10 || packet.data[0] == 13)
			var channel = 1;
		else
			break;

		needle.post ('127.0.0.1:1968/interface', {operation: '433', channel: channel, command: command},
		function (error, responder, body)
		{
			if (body.result != 'ok')
				console.error ('Error calling 433 interface');
		});
		break;
	}
}
