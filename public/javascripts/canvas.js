// -----------------------------------------------------------------------------
// CONSTANTS
// -----------------------------------------------------------------------------

const SAMPLES_DISCARED = 1;         // keep 1 sample every 10 
const SAMPLES_NUM = 30;             // keep an history of 20 samples

const CHART_VEL_DATA_ID = "velgraphic";
const INIT_TIME_MS = Date.now();

// -----------------------------------------------------------------------------
// GRAPHS CLASS: draws charts
// -----------------------------------------------------------------------------

class Graphs {
    // sse event source
    evtSource;

    // graphs
    velGraph;

    // data from server
    ts = new Array(SAMPLES_NUM).fill(Number(0).toFixed(2));
    dVel = new Array(SAMPLES_NUM).fill(Number(0).toFixed(2));
    bVel = new Array(SAMPLES_NUM).fill(Number(0).toFixed(2));

    // current sample count
    sampleCount = 0;

    constructor() {
        // draw graphs
        Chart.defaults.global.defaultFontSize = 15;

        this.velGraph =
            this.drawGraph(CHART_VEL_DATA_ID, velGraphData, velGraphOptions, this.ts, this.dVel, this.bVel);
    }

    init() {
        this.evtSource = new EventSource(SSE_API);
        this.evtSource.onmessage = (event) => {
            if (this.sampleCount++ < SAMPLES_DISCARED)
                return;

            this.sampleCount = 0;
            var data = JSON.parse(event.data);

            this.asyncUpdate(data);
        }
    }

    asyncUpdate(data) {

        var dVelModule = vNorm(data.droneVel) * 3.6;
        var bVelModule = vNorm(data.ballVel) * 3.6;

        var toBeUpdated
            = [this.ts, this.dVel, this.bVel];

        var refreshedData
            = [data.ts - INIT_TIME_MS, dVelModule, bVelModule];

        toBeUpdated.forEach((element, index) => {
            element.shift();
            element.push(refreshedData[index]);
        });

        this.updateData(this.velGraph, this.ts, [this.dVel, this.bVel]);
    }

    drawGraph(id, data, options, ...updatedData) {
        var ctx =
            document.getElementById(id).getContext('2d');

        return new Chart(ctx, {
            type: 'line',
            data: data(...updatedData),
            options: options()
        });
    }

    updateData(chart, label, data) {
        chart.data.labels = label;
        var count = 0;
        chart.data.datasets.forEach((dataset) => {
            dataset.data = data[count];
            count++;
        });
        chart.update();
    }
}

// -----------------------------------------------------------------------------
// GRAPHS OPTIONS AND PROPERTIES
// -----------------------------------------------------------------------------

/**
 * Currently there only one kind of chart
 *  - VELOCITY STATUS
 */

/**
 * VELOCITY STATUS
 */

var velGraphData =
    function (timestamps, dVel, bVel) {
        return {
            labels: timestamps,
            datasets: [
                {
                    label: 'Drone Velocity',
                    borderColor: "#900",
                    data: dVel,
                    fill: false,
                },
                {
                    label: 'Ball Velocity',
                    borderColor: "#090",
                    data: bVel,
                    fill: false,
                }]
        };
    }

var velGraphOptions =
    function () {
        return {
            scales: {
                yAxes: [{
                    ticks: {
                        min: 0,
                        max: 200
                    }
                }]
            },
            animation: false,
            responsive: true,
            title: {
                display: true,
                text: 'State Variables (Velocities km/h)'
            },
            cubicInterpolationMode: "monotone"
        };
    }

// -----------------------------------------------------------------------------
// UTILITIES
// -----------------------------------------------------------------------------

const nanoToMilli = (nano) => {
    return Math.round(nano / 1000000);
};

const nanoToSec = (nano) => {
    return (nano / 1000000000);
};

const nanoRound = (nano) => {
    return Number(nanoToMilli(nano)).toFixed(2);
}

const vNorm = (vec) => {
    return Math.sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}
