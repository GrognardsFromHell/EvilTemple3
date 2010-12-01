/**
 * Watches goals as they're started by objects and manages the
 * advancement of time for these goals automatically.
 *
 * It may be used to globally cancel all goals.
 */
var GoalManager = {};

(function () {

    /**
     * Cancels all goals that are currently queued.
     */
    GoalManager.cancelAllGoals = function() {
        for (var objId in watchedObjects) {
            watchedObjects[objId].cancelGoal();
        }

        watchedObjects = {};
    };

    const GoalAdvanceInterval = 20; // Time interval in ms for advancing goals

    var watchedObjects = {};

    function watchObject(object) {
        watchedObjects[object.id] = object;
    }

    function advanceGoals(time) {
        for (var objId in watchedObjects) {
            var obj = watchedObjects[objId];

            // Remove the object from the watched objects if the goal has already been completed
            if (!obj.goal) {
                delete watchedObjects[objId];
                continue;
            }

            try {
                obj.goal.advance(obj, time / 1000);

                if (obj.goal.isFinished()) {
                    /*
                     Please note that another participant could possibly queue a new goal upon receiving
                     the GoalFinished event. So we have to clean up the state before triggering the event.
                     Otherwise we may delete the goal that has been set by the event listener.
                     */
                    delete watchedObjects[objId];
                    delete obj.goal;
                    EventBus.notify(EventTypes.GoalFinished, obj, obj.goal);
                }
            } catch(e) {
                print("Goal advancement failed for object " + objId + ": " + e);
            }
        }

        gameView.addVisualTimer(GoalAdvanceInterval, advanceGoals);
    }

    function startup() {
        gameView.addVisualTimer(GoalAdvanceInterval, advanceGoals);
    }

    EventBus.addListener(EventTypes.GoalStarted, watchObject);

    StartupListeners.add(startup, 'goalmanager', []);

})();
