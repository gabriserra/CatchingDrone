// -----------------------------------------------------------------------------
// INITIALIZE OBJECTS
// -----------------------------------------------------------------------------

const createError = require('http-errors');
const express = require('express');
const path = require('path');
const logger = require('morgan');
const dgram = require('dgram');

// -----------------------------------------------------------------------------
// INITIALIZE ROUTERS
// -----------------------------------------------------------------------------

var indexRouter = require('./routes/index');
var streamingRouter = require('./routes/streaming');

// -----------------------------------------------------------------------------
// INITIALIZE APP & SERVER SOCKET
// -----------------------------------------------------------------------------

var app = express();
var server;

// -----------------------------------------------------------------------------
// VIEW ENGINE SETUP
// -----------------------------------------------------------------------------

app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'jade');

// -----------------------------------------------------------------------------
// APP BASIC SETUP
// -----------------------------------------------------------------------------

app.use(logger('dev'));
app.use(express.json());
app.use(express.urlencoded({ extended: false }));
app.use(express.static(path.join(__dirname, 'public')));

// -----------------------------------------------------------------------------
// USE ROUTERS
// -----------------------------------------------------------------------------

app.use('/', indexRouter);
app.use('/streaming', streamingRouter);

// -----------------------------------------------------------------------------
// SERVER LOGIC
// -----------------------------------------------------------------------------

server = dgram.createSocket('udp4');

server.on('error', (err) => {
  console.log("Error: " + err);
});

server.on('message', (data) => {
  app.set('simulation-state', data.toString());
});

server.on('listening', () => {
  const address = server.address();
  console.log(`opened server on ${address.address}:${address.port}`);
});

server.bind(8585);

// -----------------------------------------------------------------------------
// CATCH ALL + ERROR HANDLERS
// -----------------------------------------------------------------------------

// catch all 404 and forward to error handler
app.use(function (req, res, next) {
  next(createError(404));
});

// error handler
app.use(function (err, req, res, next) {
  // set locals, only providing error in development
  res.locals.message = err.message;
  res.locals.error = req.app.get('env') === 'development' ? err : {};

  // render the error page
  res.status(err.status || 500);
  res.render('error');
});

// -----------------------------------------------------------------------------
// DEFAULT STATES
// -----------------------------------------------------------------------------

const simulationDefaultState = {
  ts: 0,                // int [timestamp since boot ns]
  dronePos: [0, 0, 0],  // float [drone position x, y, z in meters]
  droneAng: [0, 0, 0],  // float [drone angles r,p,y in radians]
  droneVel: [0, 0, 0],  // float [drone velocity x,y,z in meters/second]
  ballPos: [0, 0, 0],   // float [ball position x, y, z in meters]
  ballVel: [0, 0, 0],   // float [ball velocity x, y, z in meters/second]
}

app.set('simulation-state-default', JSON.stringify(simulationDefaultState));

// -----------------------------------------------------------------------------
// EXPORTS
// -----------------------------------------------------------------------------

module.exports = app;
