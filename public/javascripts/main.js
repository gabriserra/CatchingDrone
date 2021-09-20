
// -----------------------------------------------------------------------------
// CONSTANTS
// -----------------------------------------------------------------------------

const SSE_API = "/streaming"
const RATIO = 0.5;

// -----------------------------------------------------------------------------
// GLOBAL VARIABLES
// -----------------------------------------------------------------------------

var graphs;
var project;

const config = {
    type: Phaser.WEBGL,
    transparent: true,
    scale: {
        mode: Phaser.Scale.HEIGHT_CONTROLS_WIDTH,
        autoCenter: Phaser.Scale.CENTER_BOTH,
    },
    parent: "scene",
    scene: [MainScene],
    ...Canvas()
}

window.addEventListener('load', () => {

    // scene build
    enable3d(() => new Phaser.Game(config)).withPhysics('/javascripts/vendor')

    // graph init
    graphs = new Graphs;
    graphs.init();
})
