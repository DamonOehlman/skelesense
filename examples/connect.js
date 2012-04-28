var skelesense = require('../'),
    scene = skelesense();

console.log('attempting connection');
scene.init(function(err) {
    console.log('connected', err);
    
    if (! err) {
        scene.detectUser(function(user) {
            console.log('user detected');
        });
    }
});