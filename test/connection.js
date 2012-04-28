var skelesense = require('../'),
    expect = require('expect.js');

describe('connectivity tests', function() {
    it('should be able to create a new scene, and receive a connected event', function(done) {
        var scene = skelesense();

        console.log(skelesense.Scene.prototype);
        
        scene.init(function(err) {
            expect(err).to.not.be.ok();
            console.log('connected');
            
            scene.detectUser(function(user) {
                console.log('user detected');

                done();
            });
        });
        
        
        console.log(scene);
    });
});