var skelesense = require('../');

describe('connectivity tests', function() {
    it('should be able to create a new scene, and receive a connected event', function(done) {
        var scene = skelesense();

        console.log(skelesense.Scene.prototype);
        scene.on('connect', done);
        
        scene.connect();
        console.log(scene);
    });
});