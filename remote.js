var serialport = require('serialport');

module.exports = Remote;

function Remote(port, callback) {
    this._open = false;

    this._port = new serialport(port, { baudRate: 19200 });
    this._parser = new serialport.parsers.Readline({ delimiter: '\r\n' });
    this._port.pipe(this._parser);

    this._port.on('open', () => {
        console.log('Serial port open');
        this._open = true;
    });

    this._port.on('close', () => {
        console.log('Serial port closed');
        this._open = false;
    });

    this._port.on('error', err => {
        console.log(`Serial error: ${err.message}`);
        this._open = false;
    });

    this._parser.on('data', data => {
        var obj = JSON.parse(data);
        console.dir(obj);
        /*if (obj.status == 'ok') {
            callback();
        }*/
    });

    this.send = function (channel, command, callback) {
        if (!this._open) {
            if (callback) {
                callback('Port not open');
            }
        } else {
            var obj = { channel: channel, command: command };
            this._port.write(JSON.stringify(obj), 'utf8');
            this._port.drain((error) => {
                if (callback) {
                    callback(error);
                }
            });
        }
    }

    this.close = function () {
        this._port.close();
    }
}