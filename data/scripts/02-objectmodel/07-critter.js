/**
 * The base class for every living creature.
 *
 * @constructor
 */
var Critter = function() {
};

Critter.prototype = new BaseObject;

Critter.prototype.concealed = false;

Critter.prototype.getReactionColor = function() {
    if (this.killsOnSight) {
        return [1, 0, 0]; // Friendly
    } else {
        return [0.33, 1, 0]; // Friendly
    }
};

/**
 * The movement range of a single move in world units.
 */
Critter.prototype.movementRange = 250;

/**
 * The distance a character can run per round is determined by multiplying the
 * movement range with this factor.
 */
Critter.prototype.runFactor = 3;

Critter.prototype.getReaction = function() {
    return Reaction.Neutral;
};

Critter.prototype.drawBehindWalls = true;

Critter.prototype.killsOnSight = false;

// classLevels = [], (MUST NOT BE in the prototype, otherwise it's shared by all)
Critter.prototype.experiencePoints = 0;
// feats = [], (MUST NOT BE in the prototype)
// domains = [], (MUST NOT be in the prototype)

/**
 * Checks for line-of-sight and other visibility modifiers.
 * @param object The target object.
 */
Critter.prototype.canSee = function(object) {
    // Objects are not on the same map -> can never see each other
    if (object.map !== this.map)
        return false;

    return Maps.currentMap.checkLineOfSight(this.position, object.position);
};

/**
 * Returns an object that describes the current equipment of this critter.
 */
Critter.prototype.getEquipment = function() {
    if (!this.equipment)
        this.equipment = {};
    return this.equipment;
};

Critter.prototype.updateIdleAnimation = function() {

    var renderState = this.getRenderState();

    if (!renderState || !renderState.modelInstance)
        return;

    var model = renderState.modelInstance.model;

    var idleAnimation;

    if (this.isUnconscious() && model.hasAnimation('dead_idle')) {
        idleAnimation = 'dead_idle';
    } else if (Combat.isParticipant(this) && model.hasAnimation('unarmed_unarmed_combatidle')) {
        idleAnimation = 'unarmed_unarmed_combatidle';
    } else {
        idleAnimation = 'unarmed_unarmed_idle';
    }

    if (model.hasAnimation('item_idle')) {
        idleAnimation = 'item_idle';
    }

    if (renderState.modelInstance.idleAnimation != idleAnimation)
        renderState.modelInstance.idleAnimation = idleAnimation;
};

/**
 * Checks whether this character has a certain feat.
 *
 * @param featInstance This is either the id of the feat (if the feat has no arguments)
 * or an array, which contains the feat id as the first element and subsequently all feat
 * arguments.
 */
Critter.prototype.hasFeat = function(featInstance) {
    if (featInstance instanceof Array) {
        for (var i = 0; i < this.feats.length; ++i) {
            if (featInstance.equals(this.feats[i]))
                return true;
        }
        return false;
    }
    return this.feats.indexOf(featInstance) != -1;
};

Critter.prototype.removeFeat = function(featInstance) {
    if (featInstance instanceof Array) {
        for (var i = 0; i < this.feats.length; ++i) {
            if (this.feats[i].equals(featInstance)) {
                this.feats.splice(i, 1);
                return true;
            }
        }
        return false;
    }

    var idx = this.feats.indexOf(featInstance);
    if (idx != -1)
        this.feats.splice(idx, 1);

    return idx != -1;
};

Critter.prototype.getSkillRank = function(skillId) {
    return this.skills[skillId] ? Math.floor(this.skills[skillId] / 2) : 0;
};

Critter.prototype.clicked = function(event) {

    if (Combat.isActive()) {
        CombatUi.objectClicked(this, event);
        return;
    }

    if (editMode || Party.isMember(this)) {
        if (event.modifiers & KeyModifiers.Shift) {
            if (Selection.isSelected(this)) {
                Selection.remove([this]);
            } else {
                Selection.add([this]);
            }
        } else {
            Selection.clear();
            Selection.add([this]);
        }
    }
};

/**
 * Returns the effective initiative bonus for this character, which includes the effect
 * from feats such as improved initiative and other influences.
 */
Critter.prototype.getInitiativeBonus = function() {
    return getAbilityModifier(this.getEffectiveDexterity());
};

/**
 * Gets the effective land speed of this character. This uses the character's race as the
 * base land speed, and then applies any bonuses from the class or spells/effects.
 */
Critter.prototype.getEffectiveLandSpeed = function() {
    var landSpeed = 30; // This is a fallback

    // Use the character's race as a base
    if (this.race) {
        var race = Races.getById(this.race);
        landSpeed = race.landSpeed;
    }

    // Allows a per-creature override of landspeed
    if (this.landSpeed) {
        landSpeed = this.landSpeed;
    }

    // Apply bonuses granted by the class.

    // TODO: Influence land-speed through encumbrance and armor

    return landSpeed;
};

/**
 * Returns the base attack bonus for this critter.
 */
Critter.prototype.getBaseAttackBonus = function() {
    var bonus = 0;

    // This should probably be cached
    for (var i = 0; i < this.classLevels.length; ++i) {
        var classObj = Classes.getById(this.classLevels[i].classId);
        bonus += classObj.getBaseAttackBonus(this.classLevels[i].count);
    }

    return bonus;
};

/**
 * Gets the character's effective strength ability, which includes bonuses gained by spells and
 * other, temporary means.
 */
Critter.prototype.getEffectiveStrength = function() {
    return this.strength;
};

/**
 * Gets the character's effective dexterity ability, which includes bonuses gained by spells and
 * other, temporary means.
 */
Critter.prototype.getEffectiveDexterity = function() {
    return this.dexterity;
};

/**
 * Gets the character's effective constitution ability, which includes bonuses gained by spells and
 * other, temporary means.
 */
Critter.prototype.getEffectiveConstitution = function() {
    return this.constitution;
};

/**
 * Gets the character's effective intelligence ability, which includes bonuses gained by spells and
 * other, temporary means.
 */
Critter.prototype.getEffectiveIntelligence = function() {
    return this.intelligence;
};

/**
 * Gets the character's effective wisdom ability, which includes bonuses gained by spells and
 * other, temporary means.
 */
Critter.prototype.getEffectiveWisdom = function() {
    return this.wisdom;
};

/**
 * Gets the character's effective charisma ability, which includes bonuses gained by spells and
 * other, temporary means.
 */
Critter.prototype.getEffectiveCharisma = function() {
    return this.charisma;
};

Critter.prototype.getReflexSave = function() {
    var bonus = 0;

    // This should probably be cached
    for (var i = 0; i < this.classLevels.length; ++i) {
        var classObj = Classes.getById(this.classLevels[i].classId);
        bonus += classObj.getReflexSave(this.classLevels[i].count);
    }

    // Add dexterity modifier
    bonus += getAbilityModifier(this.getEffectiveDexterity());

    return bonus;
};

Critter.prototype.getWillSave = function() {
    var bonus = 0;

    // This should probably be cached
    for (var i = 0; i < this.classLevels.length; ++i) {
        var classObj = Classes.getById(this.classLevels[i].classId);
        bonus += classObj.getWillSave(this.classLevels[i].count);
    }

    // Add dexterity modifier
    bonus += getAbilityModifier(this.getEffectiveWisdom());

    return bonus;
};

Critter.prototype.getFortitudeSave = function() {
    var bonus = 0;

    // This should probably be cached
    for (var i = 0; i < this.classLevels.length; ++i) {
        var classObj = Classes.getById(this.classLevels[i].classId);
        bonus += classObj.getFortitudeSave(this.classLevels[i].count);
    }

    // Add dexterity modifier
    bonus += getAbilityModifier(this.getEffectiveConstitution());

    return bonus;
};

/**
 * Returns the effective level of this character. For now, this is the sum of all class-levels.
 */
Critter.prototype.getEffectiveCharacterLevel = function() {
    var sum = 0;
    if (this.classLevels) {
        for (var i = 0; i < this.classLevels.length; ++i) {
            sum += this.classLevels[i].count;
        }
    }
    return sum;
};

/**
 * Gets the number of levels this critter has of the given class.
 * @param classId The class's id.
 */
Critter.prototype.getClassLevels = function(classId) {
    var sum = 0;
    for (var i = 0; i < this.classLevels.length; ++i) {
        if (this.classLevels[i].classId == classId)
            sum += this.classLevels[i].count;
    }
    return sum;
};

/**
 * Gets the object representing the levels of a certain class this character has.
 *
 * @param classId The class's id.
 * @returns null if this character doesnt have any levels of that class.
 */
Critter.prototype.getClassLevel = function(classId) {
    for (var i = 0; i < this.classLevels.length; ++i) {
        if (this.classLevels[i].classId == classId)
            return this.classLevels[i];
    }
    return null;
};

Critter.prototype.joinedParty = function() {
    if (this.OnJoin)
        LegacyScripts.OnJoin(this.OnJoin, this);
};

Critter.prototype.leftParty = function() {
    if (this.OnDisband)
        LegacyScripts.OnDisband(this.OnDisband, this);
};

Critter.prototype.rollInitiative = function() {
    var dice = new Dice(Dice.D20);
    dice.bonus = this.getInitiativeBonus();
    // TODO Dice Log
    return dice.roll();
};

Critter.prototype.getDefaultAction = function(forWhom) {
    if (Party.isMember(this))
        return null;

    if (!this.isUnconscious()) {
        return new MeleeAttackAction(this);
    } else {
        return new LootAction(this);
    }
};

Critter.prototype.giveItem = function(item) {
    item.removeFromContainer();

    item.containedIn = this;

    if (!this.content)
        this.content = [];

    this.content.push(item);

    EventBus.notify(EventTypes.ItemAddedToInventory, this, item);
};

Critter.prototype.equip = function(item, slot) {

    if (!(item instanceof Item))
        throw "Only objects of type Item can be equipped.";

    if (!item.equipmentSlots.indexOf(slot) == -1)
        throw "Item " + item.id + " cannot be equiped on slot " + slot;

    if (!this.equipment)
        this.equipment = {};

    if (this.equipment[slot]) {
        this.giveItem(this.equipment[slot]);
        delete this.equipment[slot];
    }

    item.removeFromContainer();

    this.equipment[slot] = item;
    item.equippedBy = this;

    // Notify that a new item has been equipped
    EventBus.notify(EventTypes.EquippedItem, this, item);
};

function addNodeBoundModelNode(modelFilename, boneName, renderState) {
    print("Trying to load additional model: " + modelFilename);
    var modelObj = gameView.models.load(modelFilename);

    print("Shield model loaded: " + (modelObj !== undefined));

    var modelInstance = new ModelInstance(gameView.scene);
    modelInstance.model = modelObj;
    modelInstance.drawBehindWalls = this.drawBehindWalls;
    modelInstance.idleAnimation = 'unarmed_unarmed_idle';
    modelInstance.animationEvent.connect(this, handleAnimationEvent);

    var attachedNode = new ModelInstanceAttachedSceneNode(gameView.scene);
    attachedNode.parentNode = renderState.sceneNode;
    attachedNode.attachObject(modelInstance);
    attachedNode.modelInstance = renderState.modelInstance;
    attachedNode.boneName = boneName;

    return attachedNode;
}

/**
 * Resets the render state of this critter's equipment.
 */
Critter.prototype.updateEquipmentRenderState = function() {
    var renderState = this.getRenderState();
    if (renderState) {
        Equipment.addRenderEquipment(this, renderState.modelInstance);

        if (renderState.nodes) {
            renderState.nodes.forEach(function (node) {
                gameView.scene.removeNode(node);
            });
            delete renderStates.nodes;
        }

        if (this.equipment) {
            var shield = this.equipment[Slots.Shield];

            var nodes = [];
                        
            if (shield && !shield.equipmentId && shield.model)
                nodes.push(addNodeBoundModelNode(shield.model, 'Bip01 L Forearm', renderState));
            
            var primaryWeapon = this.equipment[Slots.PrimaryWeapon];
            
            if (primaryWeapon && !primaryWeapon.equipmentId && primaryWeapon.model)
                nodes.push(addNodeBoundModelNode(primaryWeapon.model, 'HandR_ref', renderState));
            
            var secondaryWeapon = this.equipment[Slots.SecondaryWeapon];
            
            if (secondaryWeapon && !secondaryWeapon.equipmentId && secondaryWeapon.model)
                nodes.push(addNodeBoundModelNode(secondaryWeapon.model, 'HandL_ref', renderState));

            if (nodes.length > 0)
                renderState.nodes = nodes;
        }
    }
};

EventBus.addListener(EventTypes.EquippedItem, function (critter, item) {
    critter.updateEquipmentRenderState();
});

EventBus.addListener(EventTypes.UnequippedItem, function (critter, item) {
    critter.updateEquipmentRenderState();
});
