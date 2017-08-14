var fs = require ('fs');
var events = require ('events');

function Handler ()
{
	events.EventEmitter.call (this);

	this.handle = function (query, answer)
	{
		var self = this;

		// Read devices file, parse contents as JSON and return
		fs.readFile ('devices.json', {encoding: 'utf8'}, function (err, data)
		{
			if (err)
				self.emit ('done', 'Error reading devices.json', null);
			else
			{
				answer.devices = JSON.parse (data);
				self.emit ('done', null, answer);
			}
		});
	};
};

Handler.prototype = Object.create (events.EventEmitter.prototype);

exports.createHandler = function()
{
	return new Handler();
};
