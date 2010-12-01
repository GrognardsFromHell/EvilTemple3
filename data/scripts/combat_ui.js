var CombatUi = {};

(function() {

    var combatBarDialog = null;
    var initiativeBarDialog = null;
    var movementIndicatorNode = null;
    var movementIndicatorRootNode = null;

    /**
     * Make the combat UI visible.
     */
    CombatUi.show = function() {
        if (!combatBarDialog) {
            combatBarDialog = gameView.addGuiItem('interface/CombatBar.qml');

            combatBarDialog.endTurn.connect(Combat.endTurn);
        }
        if (!initiativeBarDialog) {
            initiativeBarDialog = gameView.addGuiItem('interface/InitiativeBar.qml');
        }
    };

    CombatUi.hide = function() {
        if (combatBarDialog) {
            combatBarDialog.deleteLater();
            combatBarDialog = null;
        }
        if (initiativeBarDialog) {
            initiativeBarDialog.deleteLater();
            initiativeBarDialog = null;
        }
    };

    /**
     * This function is called whenever the world is clicked while combat is active.
     *
     * @param event The mouse event that triggered the function call.
     * @param worldPosition The position in the world that was clicked.
     */
    function worldClicked(event, worldPosition) {
        if (!Combat.isActive())
            return;

        var participant = Combat.getActiveParticipant();

        if (participant && Party.isMember(participant) && !participant.isBusy()) {
            var path = Maps.currentMap.findPath(participant, worldPosition);

            if (!path.isEmpty()) {

                var length = path.length();

                if (Combat.isMoveActionAvailable()) {
                    Combat.useActionForMovement(ActionCost.Move);
                }

                var moveRemaining = participant.movementRange * Combat.getRemainingMovementBudget();

                if (length > moveRemaining) {
                    if (Combat.isStandardActionAvailable()) {
                        // Movement budget itself is insufficient, but a standard action remains

                        if (length - moveRemaining > participant.movementRange)
                            return; // Even with the standard action, the budget is insufficient

                        Combat.useActionForMovement(ActionCost.Standard);

                        /**
                         * We fully consumed the remaining movement budget and reset it to 1 with the
                         * call above. So we need to adjust the remaining length accordingly.
                         */
                        length -= moveRemaining;
                    } else {
                        // Movement budget is insufficient and no standard action remains
                        return;
                    }
                }

                Combat.reduceMovementBudget(length / participant.movementRange);
                updateCombatBar();

                var movementGoal = new MovementGoal(path, false);
                participant.setGoal(movementGoal);
            }
        }
    }

    CombatUi.objectMouseEnter = function(object, event) {
        hideMovementIndicator();

        var participant = Combat.getActiveParticipant();

        if (participant && Party.isMember(participant) && !participant.isBusy()) {

            var action = object.getDefaultAction(participant);

            if (!action)
                return;

            if (!action.combat) {
                // TODO: Notify the user
                print("Cannot perform this action during combat.");
                return;
            }

            if (!Combat.canAffordActionCost(action.cost)) {
                print("Not enough time left.");
                return;
            }

            var path = Maps.currentMap.findPathIntoRange(participant, object, action.range);

            if (path.isEmpty())
                return;

            var to = path.getEnd();

            updateMovementIndicatorFromPath(participant, path, to);
        }
    };

    CombatUi.objectMouseLeave = function(object, event) {
        return false;
    };

    /**
     * Notifies the combat UI of an object being clicked.
     *
     * @param object The object that was clicked.
     * @param event The corresponding mouse event.
     */
    CombatUi.objectClicked = function(object, event) {

        if (event.button == Mouse.RightButton) {

            var renderState = object.getRenderState();
            if (renderState)
                showMobileInfo(object, renderState.modelInstance);

        } else if (event.button == Mouse.LeftButton) {

            var participant = Combat.getActiveParticipant();

            if (Party.isMember(participant) && !participant.isBusy()) {
                var action = object.getDefaultAction(participant);

                if (!action)
                    return;

                var result = Combat.performAction(action);
                actOnCombatActionResult(result);
            }

        }

    };

    function actOnCombatActionResult(critter, result) {
        with (CombatActionResult) {
            switch (result) {
                case OutOfCombat:
                    throw "Combat UI received an event outside of combat!";

                case NotDuringCombat:
                    critter.say("This action cannot be performed during combat.");
                    break;

                case NotEnoughTime:
                    critter.say("I don't have enough time left!");
                    break;

                case TargetTooFarAway:
                    critter.say('I cannot get there in time!');
                    break;

                case TargetUnreachable:
                    critter.say('I don\'t know how to get there!');
                    break;
                
                case Success:
                    updateCombatBar();
                    break;
            }
        }
    }

    /**
     * Notifies the combat UI of an object being double-clicked.
     *
     * @param object The object that was clicked.
     * @param event The corresponding mouse event.
     */
    CombatUi.objectDoubleClicked = function(object, event) {
        print("Object double clicked");
    };

    function updateInitiative() {
        if (!initiativeBarDialog)
            return;

        var participants = Combat.getParticipants();
        var activeId = Combat.getActiveParticipantId();

        var initiative = participants.map(function (obj) {
            return {
                id: obj.mobile.id,
                name: obj.mobile.getName(),
                initiative: obj.initiative,
                portrait: Portraits.getImage(obj.mobile.portrait, Portrait.Small),
                effectiveDex: obj.mobile.getEffectiveDexterity(),
                active: obj.mobile.id == activeId
            };
        });

        // Sort by initiative first, then by dex modifier
        initiative.sort(function (a, b) {
            if (a.initiative != b.initiative) {
                return b.initiative - a.initiative;
            } else {
                return b.effectiveDex - a.effectiveDex
            }
        });

        initiativeBarDialog.initiative = initiative;
    }

    function updateCombatBar() {
        // If the active participant is not user-controlled, grey out the bar and disable the button
        var activeParticipant = Combat.getActiveParticipant();

        if (!Party.isMember(activeParticipant)) {
            combatBarDialog.state = 'gray';
            combatBarDialog.fillPercentage = 0;
        } else {
            combatBarDialog.state = '';

            var actionsAvailable = 0;

            if (Combat.isMoveActionAvailable()) {
                actionsAvailable++;
            }

            if (Combat.isStandardActionAvailable()) {
                actionsAvailable++;
            }

            // This can be the replacement for an action.
            actionsAvailable += Combat.getRemainingMovementBudget();

            combatBarDialog.fillPercentage = actionsAvailable / 2;
        }
    }

    function participantChanged(previousParticipant) {
        hideMovementIndicator();

        updateInitiative();
        updateCombatBar();

        if (previousParticipant)
            previousParticipant.setSelected(false);

        var activeParticipant = Combat.getActiveParticipant();
        activeParticipant.setSelected(true); // Show on the battlefield who is acting
    }

    function updateMovementIndicatorFromPath(participant, path, to) {
        movementIndicatorNode = gameView.scene.createNode();
        movementIndicatorNode.position = to;

        var indicator = new MovementIndicator(gameView.scene, gameView.materials);
        indicator.radius = participant.radius;
        indicator.circleWidth = 3;
        movementIndicatorNode.attachObject(indicator);

        if (path.isEmpty()) {
            indicator.fillColor = [1, 0, 0, 0.5];
            return;
        }

        movementIndicatorRootNode = gameView.scene.createNode();

        var line = new DecoratedLineRenderable(gameView.scene, gameView.materials);
        line.lineWidth = 32;

        var pos = path.from;
        line.add(path.from, []);

        var length = 0;

        var segment1Length = 0;
        var segment2Length = 0;

        var segment1Color = [0, 1, 0, 1];
        var segment2Color = [1, 1, 0, 1];
        var segment3Color = [1, 0, 0, 1];

        if (Combat.isMoveActionAvailable()) {
            segment1Length = participant.movementRange;
            segment2Length = participant.movementRange;
        } else if (Combat.isStandardActionAvailable()) {
            segment1Length = participant.movementRange * Combat.getRemainingMovementBudget();
            segment2Length = participant.movementRange;
        } else {
            segment2Length = participant.movementRange * Combat.getRemainingMovementBudget();
        }

        var color;
        if (length < segment1Length)
            color = segment1Color;
        else if (length < segment1Length + segment2Length)
            color = segment2Color;
        else
            color = segment3Color;

        path.segments.forEach(function (segment) {
            var segmentLength = segment.length;

            var remainingLength, p;

            if (length < segment1Length && length + segmentLength > segment1Length) {

                // Do we need to split the segment?
                remainingLength = segment1Length - length;

                p = V3.add(pos, V3.scale(segment.vector, remainingLength));
                line.add(p, color);
                pos = p;

                color = segment2Color;

                segmentLength -= remainingLength;
            } else if (length < segment1Length + segment2Length && length + segmentLength > segment1Length + segment2Length) {

                // Do we need to split the segment?
                remainingLength = segment1Length + segment2Length - length;

                p = V3.add(pos, V3.scale(segment.vector, remainingLength));
                line.add(p, color);
                pos = p;

                color = segment3Color;

                segmentLength -= remainingLength;
            }

            var newPos = V3.add(pos, V3.scale(segment.vector, segmentLength));
            line.add(newPos, color);
            pos = newPos;

            length += segment.length;
        });

        movementIndicatorRootNode.attachObject(line);
    }

    function mouseMoved(event, worldPos) {
        hideMovementIndicator();

        var participant = Combat.getActiveParticipant();

        if (participant && Party.isMember(participant) && !participant.isBusy()) {
            var path = Maps.currentMap.findPath(participant, worldPos);

            updateMovementIndicatorFromPath(participant, path, worldPos);
        }
    }

    function hideMovementIndicator() {
        if (movementIndicatorNode) {
            gameView.scene.removeNode(movementIndicatorNode);
            movementIndicatorNode = null;
        }
        if (movementIndicatorRootNode) {
            gameView.scene.removeNode(movementIndicatorRootNode);
            movementIndicatorRootNode = null;
        }
    }

    function goalCompleted(critter, goal) {

        if (!Combat.isActive() || !combatBarDialog)
            return;

        var participant = Combat.getActiveParticipant();

        if (participant && Party.isMember(participant) && critter === participant) {
            combatBarDialog.enabled = true;
        }
    }

    function goalStarted(critter, goal) {

        if (!Combat.isActive() || !combatBarDialog)
            return;

        var participant = Combat.getActiveParticipant();

        if (participant && Party.isMember(participant) && critter === participant) {
            combatBarDialog.enabled = false;
            hideMovementIndicator();
        }

    }

    function initialize() {
        EventBus.addListener(EventTypes.GoalStarted, goalStarted);
        EventBus.addListener(EventTypes.GoalFinished, goalCompleted);

        Combat.addCombatStartListener(CombatUi.show);
        Combat.addCombatStartListener(updateInitiative);
        Combat.addCombatEndListener(CombatUi.hide);
        Combat.addActiveParticipantChangedListener(participantChanged);

        Maps.addMouseClickListener(worldClicked);
        Maps.addMouseMoveListener(mouseMoved);
        Maps.addMouseLeaveListener(hideMovementIndicator);
    }

    StartupListeners.add(initialize, 'combat-ui', ['combat']);

}

        )
        ();
