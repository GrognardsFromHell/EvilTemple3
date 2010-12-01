/**
 * This will control an active battle. Since there can only be one active battle
 * at once, a singleton will suffice here.
 */
var Combat = {
};

(function() {

    /*
     The following variables describe the action budget for the current round and participant.
     */
    var moveActionAvailable = true;
    var moveEquivalentUsedForMovement = false;
    var standardActionAvailable = true;
    var standardActionUsedForMovement = false;
    var swiftActionAvailable = true;
    var swiftActionUsedForMovement = false;
    var remainingMovementBudget = 0;

    var combatStartListeners = new ListenerQueue;

    var combatEndListeners = new ListenerQueue;

    var active = false; // Is combat active right now?

    var participants = {
    };

    var activeParticipant = ''; // Gives the *id* of the active participant

    var activeParticipantChangedListener = new ListenerQueue;

    /**
     * Gets a descending list of participant ids, sorted by initiative.
     */
    function getDescendingInitiativeList() {
        var result = [];
        for (var k in participants) {
            result.push(k);
        }

        result.sort(function(a, b) {
            var participantA = participants[a];
            var participantB = participants[b];

            if (participantA.initiative != participantB.initiative) {
                return participantB.initiative - participantA.initiative;
            } else {
                return participantB.mobile.getEffectiveDexterity() - participantA.mobile.getEffectiveDexterity();
            }
        });

        return result;
    }

    /**
     * Resets the action budget for the current round and participant.
     */
    function resetActionBudget() {
        moveActionAvailable = true;
        moveEquivalentUsedForMovement = false;
        standardActionAvailable = true;
        standardActionUsedForMovement = false;
        swiftActionAvailable = true;
        swiftActionUsedForMovement = false;
        remainingMovementBudget = 0;
    }

    Combat.isSwiftActionAvailable = function() {
        return swiftActionAvailable;
    };

    Combat.isMoveActionAvailable = function() {
        return moveActionAvailable;
    };

    Combat.isStandardActionAvailable = function() {
        return standardActionAvailable;
    };

    /**
     * Returns the remaining movement budget. This will be 0 unless an action is converted into movement (see
     * useActionForMovement). Otherwise it will be a number between 0 and 1 (inclusive).
     */
    Combat.getRemainingMovementBudget = function() {
        return remainingMovementBudget;
    };

    /**
     * Will consume the given action type for movement and set the movement budget to 1.
     *
     * Whenever another action type is consumed, the budget will be reset to 0.
     *
     * A typical use-case is this:
     * The player clicks around the world map during combat to move there. The first click consumes the move
     * action of that turn and sets the movement budget to 1. All following clicks, as long as the movement
     * budget is non-zero will simply reduce the movement budget, but not consume any actions. Once the movement
     * budget runs out, he may either spend his standard action on movement, which will reset the movement
     * budget to 1, or use the standard action otherwise, which will reset the movement budget to 0.
     *
     * @param cost The cost of the action to consume. This may depend on the circumstances and the caller
     * is responsible for storing how far the character can move for the budget gained.
     */
    Combat.useActionForMovement = function(cost) {
        this.deductActionCost(cost);
        remainingMovementBudget = 1;
    };

    /**
     * Reduces the available movement budget.
     *
     * If the movement budget is insufficient, it will be set to zero instead of becoming negative.
     *
     * @param value The movement budget will be reduced by this value.
     */
    Combat.reduceMovementBudget = function(value) {
        remainingMovementBudget -= value;
        if (remainingMovementBudget < 0)
            remainingMovementBudget = 0;
    };

    /**
     * Performs an action for the active participant.
     *
     * @param action The action to perform.
     * @return One of the constants in CombatActionResult.
     */
    Combat.performAction = function(action) {

        var participant = Combat.getActiveParticipant();

        if (!participant)
            return CombatActionResult.OutOfCombat;

        if (!action.combat)
            return CombatActionResult.NotDuringCombat;

        if (!Combat.canAffordActionCost(action.cost))
            return CombatActionResult.NotEnoughTime;

        // TODO: We actually need to handle the case where we do NOT need to move separately.

        var path = Maps.currentMap.findPathIntoRange(participant, action.target, action.range);

        if (path.isEmpty())
            return CombatActionResult.TargetUnreachable;

        var length = path.length();

        // Two cases: Move budget is remaining, check that, or move action is remaining, deduct that
        if (Combat.isMoveActionAvailable()) {
            // Moving in range will cost us the move action, we can not move afterwards
            // (TODO: Spring Attack?)
            Combat.deductActionCost(ActionCost.Move);
        } else {
            var moveRemaining = participant.movementRange * Combat.getRemainingMovementBudget();

            if (length > moveRemaining)
                return CombatActionResult.TargetTooFarAway;
        }

        var movementGoal = new MovementGoal(path, false);
        var secondaryGoal = new ActionGoal(action);

        participant.setGoal(new GoalSequence(movementGoal, secondaryGoal));

        Combat.deductActionCost(action.cost);

        return CombatActionResult.Success;
    };

    /**
     * Select the next combat participant and give him a chance to act.
     */
    function proceedWithNextParticipant() {
        var initiative = getDescendingInitiativeList();

        // Look for the *current* id
        var currentIdx = initiative.indexOf(activeParticipant);

        // TODO: Find the next VALID participant, this currently doesn't ignore dead creatures

        var participant = null;
        if (currentIdx == -1 || currentIdx + 1 >= initiative.length) {
            print("Start of first round or end of round reached, continuing @ participant 0");
            participant = participants[initiative[0]];
        } else {
            participant = participants[initiative[currentIdx + 1]];
        }

        // Reset the action budget
        resetActionBudget();

        var previous;
        if (activeParticipant)
            previous = participants[activeParticipant].mobile;
        activeParticipant = participant.mobile.id;
        activeParticipantChangedListener.notify(previous);
    }

    /**
     * Returns whether a battle is running at the moment.
     */
    Combat.isActive = function() {
        return active;
    };

    /**
     * Determines whether the current participant can afford an action with the given cost.
     * @param cost The action cost. Use constants from the ActionCost object.
     */
    Combat.canAffordActionCost = function(cost) {
        switch (cost) {
            case ActionCost.Immediate:
            case ActionCost.Free:
                return swiftActionAvailable;
            case ActionCost.Move:
                return moveActionAvailable || standardActionAvailable;
            case ActionCost.Standard:
                return standardActionAvailable;
            case ActionCost.FullRound:
                return moveActionAvailable && standardActionAvailable;
            default:
                throw "Unknown action cost type: " + cost;
        }
    };

    /**
     * Deducts the given action cost from the budget for the active participant's round.
     * @param cost The action cost. Use constants from the ActionCost object.
     */
    Combat.deductActionCost = function(cost) {
        remainingMovementBudget = 0;

        switch (cost) {
            case ActionCost.Immediate:
            case ActionCost.Free:
                swiftActionAvailable = false;
                break;
            case ActionCost.Move:
                moveActionAvailable = false;
                break;
            case ActionCost.Standard:
                standardActionAvailable = false;
                break;
            case ActionCost.FullRound:
                moveActionAvailable = false;
                standardActionAvailable = false;
                break;
            default:
                throw "Unknown action cost type: " + cost;
        }
    };

    /**
     * Adds a mobile to the currently running battle.
     *
     * @param participant The mobile to add.
     */
    Combat.addParticipant = function(participant) {
        if (!active)
            throw "Cannot add participants to a battle if combat isn't running.";

        var id = participant.id;

        if (!id)
            throw "Cannot add a participant without a GUUID to the combat.";

        if (participants[id]) {
            print("Trying to add participant " + id + ", which is already in combat.");
            return;
        }

        participants[id] = {
            id: id,
            initiative: participant.rollInitiative(),
            mobile: participant
        };
    };

    /**
     * Start combat, given a triggerer and initial list of participants.
     * @param triggerer The critter who caused combat to be started.
     * @param _participants An array of additional participants in the combat.
     */
    Combat.start = function(triggerer, _participants) {
        if (active)
            return;

        GoalManager.cancelAllGoals();

        active = true;

        // Play combat start sound
        combatStartListeners.notify();

        Party.getMembers().forEach(Combat.addParticipant);

        _participants.forEach(Combat.addParticipant);

        proceedWithNextParticipant();
    };

    Combat.getParticipants = function() {
        var result = [];
        for (var k in participants) {
            result.push(participants[k]);
        }
        return result;
    };

    /**
     * Returns the unique id of the combat participant who is currently acting.
     */
    Combat.getActiveParticipantId = function() {
        return activeParticipant;
    };

    /**
     * Returns the mobile that is currently taking it's turn at combat.
     */
    Combat.getActiveParticipant = function() {
        var participant = participants[activeParticipant];
        if (!participant)
            return null;
        else
            return participants[activeParticipant].mobile;
    };

    /**
     * Ends the turn of the current participant.
     */
    Combat.endTurn = function() {
        if (Combat.checkEndConditions())
            return;

        if (!activeParticipant)
            throw "Cannot end the turn, since there is no active participant!";

        proceedWithNextParticipant();
    };

    Combat.checkEndConditions = function() {

        var endCombat = Combat.getParticipants().every(function (participant) {
            return participant.mobile.getReaction() != Reaction.Hostile || participant.mobile.isUnconscious()
                    || participant.mobile.map !== Maps.currentMap;
        });

        if (endCombat) {
            if (activeParticipant) {
                participants[activeParticipant].mobile.setSelected(false);
            }

            var prevParticipants = participants;

            participants = {};
            activeParticipant = '';
            active = false;
            combatEndListeners.notify();

            // Update the idle animation of every participant
            for (var k in prevParticipants) {
                prevParticipants[k].mobile.updateIdleAnimation();
            }
        }

        return endCombat;

    };

    /**
     * Checks whether a single party member will cause combat to start. Use this function if this
     * party member was moved or updated.
     *
     * @param member The party member to check.
     */
    Combat.checkCombatConditionsForPlayer = function(member) {

        var combatTriggerRange = 500; // How is this determined?

        // Gather all NPCs in the vicinity
        var vicinity = Maps.currentMap.vicinity(member.position, combatTriggerRange, NonPlayerCharacter);

        var participants = vicinity.filter(function (critter) {

            // Never take disabled/dont-draw creatures into account
            if (critter.disabled || critter.dontDraw)
                return false;

            // Skip disabled NPCs (TODO: Update this when isDisabled is available)
            if (critter.isUnconscious())
                return false;

            // Skip non-hostile NPCs
            if (critter.getReaction() != Reaction.Hostile)
                return false;

            // Check for visibility
            if (!critter.canSee(member))
                return false;

            // TODO: check other conditions
            return true;
        });

        if (participants.length > 0 && !active) {
            Combat.start(null, participants);
        }

        return participants.length > 0;
    };

    /**
     * Checks whether combat should be started due to hostile NPCs coming into view.
     */
    Combat.checkCombatConditions = function() {

        if (active || !Maps.currentMap)
            return;

        Party.getMembers().forEach(Combat.checkCombatConditionsForPlayer);
    };

    /**
     * Tests whether a given critter is participating in combat.
     * @param critter The critter to test for.
     */
    Combat.isParticipant = function(critter) {
        return participants[critter.id] !== undefined;
    };

    Combat.addCombatStartListener = function(listener) {
        combatStartListeners.append(listener);
    };

    Combat.addCombatEndListener = function(listener) {
        combatEndListeners.append(listener);
    };

    Combat.addActiveParticipantChangedListener = function(listener) {
        activeParticipantChangedListener.append(listener);
    };

    function critterDied(critter, damage, source) {
        Combat.checkEndConditions();
    }

    function initialize() {
        var check = function() {
            Combat.checkCombatConditions();
            gameView.addVisualTimer(1000, check);
        };
        gameView.addVisualTimer(1000, check);

        EventBus.addListener(EventTypes.CritterDied, critterDied);
    }

    StartupListeners.add(initialize, "combat");

})();
