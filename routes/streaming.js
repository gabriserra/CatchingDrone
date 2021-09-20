const express = require('express');
const router = express.Router();

/* receive streaming updates for pendulum status */
router.get('/', (req, res, next) => {

    // set SSE equest header
    res.setHeader('Cache-Control', 'no-cache');
    res.setHeader('Content-Type', 'text/event-stream');
    res.setHeader("Access-Control-Allow-Origin", "*");

    // flush the headers to establish SSE with client
    res.flushHeaders();

    // TODO: estimate loop interval in latency based way
    let loopInterval = 10;

    // execute every loopinterval ms
    loop = setInterval(function () {

        // avoid sending null data via sse
        data = req.app.get('simulation-state');

        if (data == undefined)
            data = req.app.get('simulation-state-default');

        res.write(`data: ${data.toString()}\n\n`);

    }, loopInterval);

    // stop sending events if client closes connection 
    res.on('close', () => {
        if (req.app.get('env') === 'development')
            console.log('client dropped the connection');
        clearInterval(loop);
    });

});

module.exports = router;
