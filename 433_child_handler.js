var events = require ('events');
var child = require ('child_process');

function Handler ()
{
	events.EventEmitter.call (this);

	this.handle = function (query, answer)
	{
		var self = this;

		// Get channel; default to 1
		var channel = parseInt (query.channel) || 1;

		// Get command; default to on
		var command = query.command || 'on';

		// Construct parameters
		var params = '--channel ' + channel + ' --command ' + command;

		// Launch executable with parameters
		child.exec('433mhz/bin/send ' + params, function (error, stdout, stderr)
		{
			// Check for optional 'duration' parameter (in seconds)
			// Only apply to 'on' commands
			var duration = query.duration;
			if (duration && command == 'on')
			{
				setTimeout (function ()
				{
					child.exec('433mhz/bin/send --command off --channel ' + channel);
				}, duration * 1000);
			}

			self.emit ('done', null, answer);
		});
	};
};

Handler.prototype = Object.create (events.EventEmitter.prototype);

exports.createHandler = function()
{
	return new Handler();
};
