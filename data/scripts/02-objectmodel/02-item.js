/**
 * The base prototype for all items in the game.
 * @constructor
 */
function Item() {
}

Item.prototype = new BaseObject;

Item.prototype.equipmentSlots = [];

Item.prototype.doubleClicked = function(event) {
    if (Combat.isActive()) {
        CombatUi.objectDoubleClicked(this, event);
        return;
    }

    if (event.button == Mouse.LeftButton) {
        if (this.OnUse) {
            LegacyScripts.OnUse(this.OnUse, this);
        }
    }
};

/**
 * Removes this item from its current container (if any).
 */
Item.prototype.removeFromContainer = function() {
    /*
        Remove the item from a critter's equipment.
     */
    if (this.equippedBy) {
        var critter = this.equippedBy;
        print("Removing item " + this.id + " from equipment of " + critter.id);

        if (!critter.equipment)
            throw "Item is equipped on " + critter.id + ", but critter has no equipment.";

        for (var slot in critter.equipment) {
            if (critter.equipment[slot] === this) {
                delete critter.equipment[slot];
                break;
            }
        }

        delete this.equippedBy;

        // Notify that an item has been unequipped
        EventBus.notify(EventTypes.UnequippedItem, critter, this);
    }

    /*
        Remove the item from a potential container.
     */
    if (this.containedIn) {
        var container = this.containedIn;
        print("Removing item " + this.id + " from container " + container.id);

        if (!container.content)
            throw "Item is contained in " + container.id + ", but container has no content.";

        for (var i = 0; i < container.content.length; ++i) {
            if (container.content[i] === this) {
                container.content.splice(i, 1);
                break;
            }
        }

        delete this.containedIn;

        // Notify that an item has been removed from a container
        EventBus.notify(EventTypes.ItemRemovedFromInventory, container, this);
    }
};
