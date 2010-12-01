function Action(name, description) {
    /**
     * A human readable name for this action.
     */
    this.name = name;

    /**
     * A human readable description of this action.
     */
    this.description = description;

    /**
     * The cursor to show for selecting this action. This is either the cursor
     * shown for objects that have this action as their default action, or
     * if a target needs to be selected for this action.
     */
    this.cursor = Cursors.Default;

    /**
     * Allows the use of this action during combat.
     */
    this.combat = true;

    /**
     * The range at which this action may be performed in standard world units.
     * This should never be zero unless the range is personal.
     */
    this.range = 15;
}

/**
 * The cost to perform this action during combat.
 */
Action.prototype.cost = ActionCost.Standard;

Action.prototype.perform = function() {
    throw "perform must be overriden for actions.";
};

function MeleeAttackAction(target) {
    if (!(this instanceof MeleeAttackAction))
        throw "Use the new keyword to construct actions";

    this.target = target;
}

MeleeAttackAction.prototype = new Action('Attack', 'Performs a melee attack');

MeleeAttackAction.prototype.animation = Animations.AttackRight;

MeleeAttackAction.prototype.cursor = Cursors.Sword;

MeleeAttackAction.prototype.cost = ActionCost.Standard;

MeleeAttackAction.prototype.perform = function(critter) {
    this.target.dealDamage(2, critter);
};

function LootAction(target) {
    if (!(this instanceof LootAction))
        throw "Use the new keyword to construct actions";

    this.target = target;
}

LootAction.prototype = new Action('Loot', 'Rummage through the belongings');

LootAction.prototype.cursor = Cursors.Hand;

LootAction.prototype.cost = ActionCost.Standard;

LootAction.prototype.perform = function(critter) {
    showInventory(this.target);
};
