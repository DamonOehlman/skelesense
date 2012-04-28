var skelesense = require('../'),
    scene = skelesense();

console.log('attempting connection');
scene.init(function(err) {
    console.log('connected', err, err instanceof Error);
    
    if (! err) {
        scene.capture();
        
        console.log('capturing');
        /*
        scene.watchForUsers(function() {
            console.log('watching for users');
        });
        
        scene.on('newuser', function() {
            console.log('user detected');
        });
        */
        
        // process.stdin.resume();
    }
});