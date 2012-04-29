var async = require('async'),
    skelesense = require('./build/Release/skelesense'),
    inherits = require('inherits'),
    EventEmitter = require('events').EventEmitter,
    util = require('util');

inherits(skelesense.Scene, EventEmitter, {
    init: function(callback) {
        this._init(callback);
        /*
        async.series([
            this._init.bind(this),
            this._createDepthGenerator.bind(this),
            this._createUserGenerator.bind(this),
            this._createImageGenerator.bind(this),
            this._start.bind(this)
        ], callback);
        */
    },
    
    capture: function() {
        var scene = this,
            cap = this.capture.bind(this);
        
        this._capture(function(err) {
            
            var users = Array.prototype.slice.call(arguments, 1);
            
            users.forEach(function(user) {
                scene.emit('skeleton', user);
            });
            
            process.nextTick(cap);
        });
    }
});

exports = module.exports = function() {
    return new skelesense.Scene();
};

exports.Scene = skelesense.Scene;