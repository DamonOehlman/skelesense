var skelesense = require('./build/Release/skelesense'),
    EventEmitter = require('events').EventEmitter,
    util = require('util');
    
util.inherits(skelesense.Scene, EventEmitter);

exports = module.exports = function() {
    return new skelesense.Scene();
};

exports.Scene = skelesense.Scene;