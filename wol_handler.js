var fs = require ('fs');
var events = require ('events');
var wol = require('node-wol');

function Handler ()
{
	events.EventEmitter.call (this);

	this.handle = function (query, answer)
	{
        var self = this;

        wol.wake(query.target, function(err) {
            if (err) {
				self.emit ('done', 'Error sending WOL packet', null);                
            } else {
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
