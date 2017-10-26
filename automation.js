var program = require ('commander')

program
	.version(0.1)
	.option('--command <n>', 'Command', parseInt)
	.option('--channel <n>', 'Channel', parseInt)
	.option('--port <name>', 'Port')
	.parse(process.argv);

global.remote = new (require ('./remote.js')) (program.port);

if (('command' in program) && ('channel' in program)) {
	setTimeout (() => {
		global.remote.send(program.channel, program.command);
	}, 2000);

	return;
}

var express = require ('express');
var bodyParser = require('body-parser')

var app = express ();
var server = require ('http').createServer (app);

app.use(bodyParser.urlencoded({extended: false}))
app.use (express.static (__dirname + '/'));

var handlers = [];
var files = require('fs').readdirSync('./');
files.forEach(function(file)
{
	var match = file.match(/(.+)_handler\.js$/);
	if (match != null)
		handlers[match[1]] = require('./' + file);
});

app.post ('/interface', function (request, response)
{
	try
	{
		if (!('operation' in request.body))
			throw 'Missing parameter: operation';

		if (!(request.body.operation in handlers))
			throw 'Unknown operation: ' + request.body.operation;

		var handler = handlers [request.body.operation].createHandler();
		handler.on ('done', function (err, answer)
		{
			if (err)
			{
				answer = {result: 'error', message: err};
			}

			response.setHeader ('Access-Control-Allow-Origin', '*');
			response.send (answer);
		});
		handler.handle (request.body, {result: 'ok'});
	}
	catch (e)
	{
		console.error (e);
	}
});

server.listen (1968, function ()
{
	console.log ('Server listening');
});

process.on ('SIGINT', function ()
{
	console.log ('SIGINT received, starting exit');
	process.exit ();
});

process.on ('exit', function ()
{
	console.log ('Closing down');
	global.remote.close();
});
