// -----------------------------------------------------------------------------
// LOAD DEPENDENCIES (BOOTSTRAP + PHASER + ENABLE3D + CHART JS) 
// -----------------------------------------------------------------------------

loadBootstrap();
loadPhaser();
loadEnable3D();
loadChartJS();

// load bootstrap
function loadBootstrap() {
    // jquery global
    const $ = require('jquery')
    window.jQuery = $;

    // bootstrap doesn't have a "main" field / export anything
    window.Bootstrap = require('bootstrap/dist/js/bootstrap');

    // get Bootstrap styles
    require("bootstrap/dist/css/bootstrap.min.css");
}

// load phaser
function loadPhaser() {
    window.Phaser = require('phaser/dist/phaser');
}

// load enable3d
function loadEnable3D() {
    window.ENABLE3D = require('@enable3d/phaser-extension/dist/bundle');
}

// load chart js
function loadChartJS() {
    // export Chart js
    window.Chart = require('chart.js/dist/Chart.min');

    // export Chart js annotation plugin
    window.annotationPlugin
        = require('chartjs-plugin-annotation/chartjs-plugin-annotation.min');
}