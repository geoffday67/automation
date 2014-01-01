var events = require ('events');
var child = require ('child_process');

function Handler ()
{
	events.EventEmitter.call (this);

	this.handle = function (query, answer)
	{
		var self = this;

		switch (query.command)
		{
			case 'on':
				child.exec('433mhz/bin/send on', function (error, stdout, stderr)
				{
					console.log ("Send done " + error);
					self.emit ('done', null, answer);
				});
				break;

			case 'off':
				child.exec('433mhz/bin/send off', function (error, stdout, stderr)
				{
					console.log ("Send done " + error);
					self.emit ('done', null, answer);
				});
				break;
		}
	};
};

Handler.prototype = Object.create (events.EventEmitter.prototype);

exports.createHandler = function()
{
	return new Handler();
};
