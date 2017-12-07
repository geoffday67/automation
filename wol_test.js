wol = require('node-wol');
wol.wake('E0:3F:49:7F:78:7A', function(err){
    console.log(err);
});
