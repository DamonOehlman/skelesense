var skelesense = require('../');

describe('connectivity tests', function() {
    it('should be able to create a new scene, and receive a connected event', function(done) {
        var scene = skelesense();

        console.log(skelesense.Scene.prototype);
        
        scene.connect(function() {
            console.log('connected');
            
            scene.detectUser(function(user) {
                console.log('user detected');
                
                while (true) {
                }

                done();
            });
        });
        
        
        console.log(scene);
    });
});