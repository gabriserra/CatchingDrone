const express = require('express');
const router = express.Router();

/* GET home page (main view) */
router.get('/', function (req, res, next) {
  res.render('index');
});

module.exports = router;
