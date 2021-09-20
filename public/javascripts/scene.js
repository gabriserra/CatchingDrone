// -----------------------------------------------------------------------------
// IMPORT OBJECTS FROM LIBRARIES
// -----------------------------------------------------------------------------

const { enable3d, Scene3D, PhysicsLoader, THREE, ExtendedObject3D, Canvas, LinearEncoding } = ENABLE3D

// -----------------------------------------------------------------------------
// CONSTANTS
// -----------------------------------------------------------------------------

const X = 0
const Y = 1
const Z = 2

const DRONE_SCALE = 0.1;
const BALL_SCALE = 0.75;

const SPACEBAR_KEY = 32;

// -----------------------------------------------------------------------------
// OBJECTS
// -----------------------------------------------------------------------------

class Ball {
    third;
    geometry;
    color = "#E86219";

    constructor(third) {
        this.third = third;
    }

    draw() {
        const color = new THREE.Color(this.color);

        this.geometry = this.third.add.sphere({ y: 1 }, { standard: { color } });
        this.geometry.scale.set(BALL_SCALE, BALL_SCALE, BALL_SCALE);
    }

    _moveBall(position) {
        this.geometry.position.set(position[X], position[Z], position[Y]);
    }

    update(position) {
        this._moveBall(position);
    }
}

class Camera {
    scene;
    cameras;
    cameraIdx;
    canCameraChange;
    defaultCameraPos;
    cameraOffset = -7;

    constructor(scene) {
        this.scene = scene;
        this.cameras = [];
        this.cameraIdx = 0;
        this.canCameraChange = true;
    }

    init() {
        // create default camera
        this.create(this.scene.third.camera.position);
    }


    create(position) {
        const cam = new THREE.Object3D();

        cam.position.copy(position);
        this.cameras.push(cam);
    }

    _update(start, lookAt) {
        if (this.cameraIdx == 0)
            return;

        const line = Vector.line(lookAt, start);
        const pos = Vector.point(line, Vector.normalize(this.cameraOffset, line[1]));

        this.scene.third.camera.position.set(pos.x, pos.z + 3, pos.y + 3);
        this.scene.third.camera.lookAt(lookAt[X], lookAt[Z], lookAt[Y]);
    }

    get(idx) {
        return this.cameras[idx];
    }

    change() {
        if (this.canCameraChange == false)
            return;

        this.canCameraChange = false;
        this.scene.time.addEvent({
            delay: 250,
            callback: () => (this.canCameraChange = true)
        });

        // leave default camera
        if (this.cameraIdx == 0) {
            this.defaultCameraPos = this.scene.third.camera.position.clone();
            this.scene.warpFeatures.orbitControls.enabled = false;
        }

        this.cameraIdx = (this.cameraIdx + 1) % this.cameras.length;

        // restore to default camera
        if (this.cameraIdx == 0) {
            this.scene.warpFeatures.orbitControls.enabled = true;
        }
    }

    update(start, lookAt) {
        this._update(start, lookAt);
    }
}

class Drone {
    third;
    geometry;
    drone;

    constructor(third) {
        this.third = third;
        this.geometry = new THREE.Group();
    }

    _prepareMaterial(mesh) {
        const encoding = LinearEncoding;

        // ensure
        if (!mesh.isMesh)
            return

        mesh.castShadow = false;
        mesh.receiveShadow = false;
        mesh.material.metalness = 0;
        mesh.material.roughness = 1;

        if (mesh.material.map)
            mesh.material.map.encoding = encoding;
    }

    _loadGeometry() {
        this.drone = new ExtendedObject3D()

        this.third.load.gltf('../geometry/drone/drone.gltf')
            .then((object) => {
                this.drone.add(object.scene);
                this.geometry.add(this.drone);

                object.scene.traverse(child => this._prepareMaterial(child));

                this.third.animationMixers.add(this.drone.animation.mixer)
                this.drone.anims.mixer.timeScale = 4;

                // play all animations together
                object.animations.forEach((clip) => {
                    this.drone.anims.mixer.clipAction(clip).reset().play();
                });

            });

        // add geometry to the scene
        this.third.add.existing(this.geometry);

        // scale and move geometry
        this.geometry.scale.set(DRONE_SCALE, DRONE_SCALE, DRONE_SCALE);
        this.geometry.position.set(0, 0.7, 0);
    }

    _moveDrone(position) {
        this.geometry.position.set(position[X], position[Z], position[Y]);
    }

    _rotateDrone(angle) {
        this.geometry.rotation.set(angle[X], angle[Y], angle[Z]);
    }

    draw() {
        this._loadGeometry();
    }

    addCamera(camera) {
        this.geometry.add(camera);
    }

    update(position, angle) {
        this._moveDrone(position);
        this._rotateDrone(angle);
    }
}

// -----------------------------------------------------------------------------
// MAIN SCENE
// -----------------------------------------------------------------------------

class MainScene extends Scene3D {

    drone;
    ball;
    camera;
    evtSource;
    warpFeatures;

    constructor() {
        super({ key: 'MainScene' })
    }

    init() {
        this.evtSource = new EventSource(SSE_API);

        // call asynchUpdate fun for each update sent
        this.evtSource.onmessage = (event) => {
            this.asynchUpdate(event.data);
        }

        // unlock phaser 3rd dimension via enable3d
        this.accessThirdDimension();

        // preload textures
        this.third.load.preload('grass', '/textures/grass.jpg');

        // instantiate drone, ball and cameras
        this.ball = new Ball(this.third);
        this.drone = new Drone(this.third, this);
        this.camera = new Camera(this);
    }

    async _drawGround() {
        const texture = await this.third.load.texture('grass');

        // repeate texture wrapping both vertically and horizontally
        texture.wrapS = THREE.RepeatWrapping;
        texture.wrapT = THREE.RepeatWrapping;
        texture.repeat.set(20, 20);

        const geometry = {
            name: 'ground',
            width: 200,
            height: 200,
            depth: 1,
            y: -1.5
        };

        const material = {
            phong: {
                map: texture,
                opacity: 0.5,
                transparent: true
            },
        };

        this.third.factories.add.ground(geometry, material);
    }

    create() {
        // create main scene with light, sky, orbit controls but w/o ground
        this.third.warpSpeed('-ground')
            .then((result) => {
                this.warpFeatures = result
            });

        // add ground
        this._drawGround();

        // draw drone and ball mesh
        this.drone.draw();
        this.ball.draw();

        // init main camera
        this.camera.init();
        this.camera.create(new THREE.Vector3());

        // save space key in keys prop
        this.keys = {
            space: this.input.keyboard.addKey(SPACEBAR_KEY)
        };
    }

    update() {
        // change camera when spacebar pressed
        if (this.keys.space.isDown) {
            this.camera.change();
        }
    }


    /**
     * Called at each update coming from server via SSE
     */
    asynchUpdate(data) {
        // parse data as JSON string
        let dataObj = JSON.parse(data);

        // update drone, ball and camera pos
        this.drone.update(dataObj.dronePos, dataObj.droneAng);
        this.ball.update(dataObj.ballPos);
        this.camera.update(dataObj.dronePos, dataObj.ballPos);
    }

}



