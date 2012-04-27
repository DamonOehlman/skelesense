var skelesense = require('./build/Release/skelesense'),
    inherits = require('inherits'),
    EventEmitter = require('events').EventEmitter,
    util = require('util');

inherits(skelesense.Scene, EventEmitter);

exports = module.exports = function() {
    return new skelesense.Scene();
};

exports.Scene = skelesense.Scene;