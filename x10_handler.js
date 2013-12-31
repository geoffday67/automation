var events = require ('events');
var x10 = require ('./x10.js');

function Handler ()
{
	events.EventEmitter.call (this);
};

Handler.prototype = Object.create (events.EventEmitter.prototype);

Handler.prototype.handle = function (query, answer)
{
	switch (query.command)
	{
		case 'on':
			x10.turnOn ();
			break;

		case 'off':
			x10.turnOff ();
			break;
	}
			
	this.emit ('done', null, answer);
};

exports.createHandler = function()
{
	return new Handler();
};
