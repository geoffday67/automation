var serial = require ('serialport');
serial.list (function (err, ports)
{
	ports.forEach (function (port)
	{
		console.dir (port.comName);
	});
});
