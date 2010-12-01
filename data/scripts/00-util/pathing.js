/**
 * Describes a path between two points in the world.
 * The path may be updated and traversed.
 */
function Path(points) {

    if (!(this instanceof Path))
        throw "Only use the Path constructor.";

    if (points && points.length > 1) {
        this.from = points[0];
        this.init(points);
    } else {
        this.segments = [];
    }
}

(function() {

    function pointsToSegments(points) {
        var result = [];

        for (var i = 0; i < points.length - 1; ++i) {
            var d = V3.sub(points[i + 1], points[i]);
            var l = V3.length(d);
            V3.normalize(d, d); // We want a unit-length directional vector
            result.push({
                vector: d,
                length: l
            });
        }

        return result;
    }

    Path.prototype.init = function(points) {
        this.segments = pointsToSegments(points);
    };

    /**
     * Returns whether this path is empty.
     */
    Path.prototype.isEmpty = function() {
        return !this.segments.length;
    };

    /**
     * Returns the current directional vector or undefined if this path is empty.
     */
    Path.prototype.currentDirection = function() {
        if (this.segments.length > 0)
            return this.segments[0].vector;
        else
            return undefined;
    };

    /**
     * Gets the last point on this path.
     *
     * Don't call this method if the path is empty.
     */
    Path.prototype.getEnd = function() {
        var p = this.from;

        this.segments.forEach(function (segment) {
            p = V3.add(p, V3.scale(segment.vector, segment.length));
        });

        return p;
    };

    /**
     * Returns the length of this path in world units.
     */
    Path.prototype.length = function() {
        var l = 0;
        for (var i = 0; i < this.segments.length; ++i) {
            l += this.segments[i].length;
        }
        return l;
    };

    /**
     * Advances along this path by a given distance. The function will return the new
     * position along the path.
     *
     * @param distance The distance by which to advance.
     */
    Path.prototype.advance = function(distance) {
        while (distance > 0 && this.segments.length > 0) {
            var segment = this.segments[0];

            // We only partially consume this segment
            if (distance < segment.length) {
                segment.length -= distance;
                var v = V3.scale(segment.vector, distance);
                return V3.add(this.from, v, this.from);
            }

            // We will consume this segment in it's entirety
            V3.scale(segment.vector, segment.length, segment.vector); // We can reuse this as it'll be removed
            V3.add(this.from, segment.vector, this.from);
            this.segments.splice(0, 1);
        }

        return this.from;
    };

})();