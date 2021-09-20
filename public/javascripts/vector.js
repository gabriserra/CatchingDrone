
// -----------------------------------------------------------------------------
// MATH VECTOR ALGEBRA
// -----------------------------------------------------------------------------

var Vector = {

    Rn: {
        X: 0,
        Y: 1,
        Z: 2,
        Axis: ['x', 'y', 'z'],
        Component: { x: 0, y: 1, z: 2 }
    },

    // extract the ith component from the vector v
    _ext: (v, ith) => {
        if (Array.isArray(v) && v.length > ith)
            return v[ith];
        else if (v[Vector.Rn.Axis[ith]] != undefined)
            return v.v[Rn.Axis[ith]];
        throw new Error(`Unable to extract ${v[Vector.Rn.Axis[ith]]} component from given vector`);

    },

    // perform the operator between each component of v1 and v0
    _compute: (v1, v0, operator) => {
        let res = {};

        v1 = Vector.objectize(v1);
        v0 = Vector.objectize(v0);

        for (const comp in v1) {
            res[comp] = operator(v1[comp], v0[comp]);
        }

        return res;
    },

    // vectorize the given object
    vectorize: (v) => {
        if (Array.isArray(v))
            return v;

        var vec = [];

        for (const comp in v) {
            vec[Vector.Rn.Component[comp]] = v[comp];
        }

        return vec;
    },

    // objectize the given vector
    objectize: (v) => {
        if (!Array.isArray(v))
            return v;

        var obj = {};

        v.forEach((comp, index) => {
            obj[Vector.Rn.Axis[index]] = comp;
        });

        return obj;
    },

    diff: (v1, v0) => {
        return Vector._compute(v1, v0, (comp1, comp0) => {
            return comp1 - comp0;
        });
    },

    mul: (v, t) => {
        let scalar = { x: t, y: t, z: t };

        return Vector._compute(v, scalar, (comp1, comp0) => {
            return comp1 * comp0;
        });
    },

    add: (v1, v0) => {
        return Vector._compute(v1, v0, (comp1, comp0) => {
            return comp1 + comp0;
        });
    },

    norm: (v) => {
        let vect = Vector._compute(v, v, (comp1, comp0) => {
            return comp1 * comp0;
        });

        let mag = 0;

        for (comp in vect) {
            mag += vect[comp];
        }

        return Math.sqrt(mag);
    },

    normalize: (n, v) => {
        return n / Vector.norm(v);
    },

    // compute the line that connects p1 and p0
    line: (p1, p0) => {
        return [Vector.objectize(p0), Vector.diff(p1, p0)];
    },

    point: (line, t) => {
        return Vector.add(line[0], Vector.mul(line[1], t));
    },



}

