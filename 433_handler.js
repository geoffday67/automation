var events = require ('events');

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

		// Convert to parameter for Arduino
		channel = channel - 1;
		command = command == 'on' ? 1 : 0;

		// Send the command
		global.remote.send(channel, command, (error) => {
			if (error) {
				self.emit ('done', error);
				return;
			}
			
			// Check for optional 'duration' parameter (in seconds)
			// Only apply to 'on' commands
			var duration = query.duration;
			if (duration && command == 1)
			{
				setTimeout (function ()
				{
						global.remote.send(channel, 0, null);
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
