var express = require ('express');
var app = express ();
var server = require ('http').createServer (app);

app.use (express.bodyParser());
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
});
