// Constants for mouse buttons
var Mouse = {
    NoButton: 0,
    LeftButton: 1,
    RightButton: 2,
    MidButton: 4,
    XButton1: 8,
    XButton2: 16
};

/**
 * NPC reaction towards the player
 */
var Reaction = {
    Great: 0,
    Good: 1,
    Neutral: 2,
    Bad: 3,
    Worst: 4,
    Hostile: 5
};

/*
 Equipment slot constants
 */
var Slots = {
    Helmet: 'helmet',
    Amulet: 'amulet',
    Gloves: 'gloves',
    PrimaryWeapon: 'primaryWeapon',
    SecondaryWeapon: 'secondaryWeapon',
    Armor: 'armor',
    Ring1: 'ring1',
    Ring2: 'ring2',
    Boots: 'boots',
    Ammo: 'ammo',
    Cloak: 'cloak',
    Shield: 'shield',
    Robes: 'robes',
    Bracers: 'bracers',
    Instrument: 'instrument',
    Lockpicks: 'lockpicks'
};

/*
 Genders.
 */
var Gender = {
    Female: 'female',
    Male: 'male',
    Other: 'other'
};

/**
 * Keyboard modifiers.
 */
var KeyModifiers = {
    Shift: 0x02000000,
    Control: 0x04000000,
    Alt: 0x08000000,
    Meta: 0x10000000,
    Keypad: 0x20000000
};

/**
 * Alignments can be checked via the == operator, but no numeric relation should be assumed.
 * This is why they're objects and not numbers.
 */
var Alignment = {
    LawfulGood: 'lawful_good',
    NeutralGood: 'neutral_good',
    ChaoticGood: 'chaotic_good',
    LawfulNeutral: 'lawful_neutral',
    TrueNeutral: 'true_neutral',
    ChaoticNeutral: 'chaotic_neutral',
    LawfulEvil: 'lawful_evil',
    NeutralEvil: 'neutral_evil',
    ChaoticEvil: 'chaotic_evil'
};

var Area = {
    Hommlet: "hommlet",
    Moathouse: "moathouse",
    Nulb: "nulb",
    Temple: "temple",
    EmridyMeadows: "emridy_meadows",
    ImerydsRun: "imeryds_run",
    TempleSecretExit: "temple_secret_exit",
    MoathouseSecretExit: "moathouse_secret_exit",
    OgreCave: "ogre_cave",
    DekloGrove: "deklo_grove",
    TempleRuinedHouse: "temple_ruined_house",
    TempleTower: "temple_tower"
};

var QuestState = {
    Unknown: 'unknown',
    Mentioned: 'mentioned',
    Accepted: 'accepted',
    Completed: 'completed',
    Botched: 'botched'
};

var RestingStatus = {
    Safe: 'safe',
    PassTimeOnly: 'pass_time_only',
    Impossible: 'impossible',
    Dangerous: 'dangerous'
};

/**
 * Defines constants for the ability scores.
 */
var Abilities = {
    Strength: 'strength',
    Dexterity: 'dexterity',
    Constitution: 'constitution',
    Intelligence: 'intelligence',
    Wisdom: 'wisdom',
    Charisma: 'charisma'
};

var Animations = {
    GetHitFront: 'fhit',
    GetHitBack: 'bhit',
    GetHitLeft: 'lhit',
    GetHitRight: 'rhit',
    Death: 'death',
    UseObject: 'magichands',
    AttackRight: 'rattack'
};

/**
 * The filenames for various cursor types.
 */
var Cursors = {
    Default: 'art/interface/cursors/MainCursor.png',
    Sword: 'art/interface/cursors/Sword.png',
    Hand:  'art/interface/cursors/Map_GrabHand_Open.png'
};

/**
 * Gives result codes for performing an action during combat.
 */
var CombatActionResult = {
    /**
     * Combat is currently not running, which is why the action wasn't performed.
     */
    OutOfCombat: 'notActive',

    /**
     * This particular action cannot be performed during combat. I.e. spells with a cast time
     * of more than 1 round.
     */
    NotDuringCombat: 'notDuringCombat',

    /**
     * The participant doesn't have enough time left to perform the action itself.
     */
    NotEnoughTime: 'notEnoughTime',

    /**
     * The action cannot be performed because the target is too far away to reach it in time.
     */
    TargetTooFarAway: 'targetTooFarAway',

    /**
     * The participant cannot find a way to get in range to perform the action. This is the result
     * if no path to the target is found.
     */
    TargetUnreachable: 'targetUnreachable',

    /**
     * The action is being performed. This code is not an indication that the action has already
     * been performed, since it may take time to complete. Instead, it is set as the current goal
     * of the character.
     */
    Success: 'success'

};

/**
 * Every round provides a swift, move and standard action.
 *
 * These enumeration literals describe the cost of an action.
 */
var ActionCost = {
    /**
     * The action has no associated cost.
     */
    None: 'none',
    /**
     * Immediate actions are swift actions that can be taken during another participants turn.
     */
    Immediate: 'immediate',
    /**
     * A single swift action can be taken per round.
     */
    Swift: 'swift',
    /**
     * Free actions can be taken whenever it's the character's turn. There is no limit on the number of free
     * actions that a character can taken during his or her turn.
     */
    Free: 'free',
    /**
     * A single move action can be taken per turn, although a second move action can be taken by consuming the
     * standard action of that turn.
     */
    Move: 'move',
    /**
     * A single standard action can be taken per turn.
     */
    Standard: 'standard',
    /**
     * A full round action consumes both the move and standard action of that turn, while free and swift actions
     * may still be taken. If no movement is involved in the full round action, a 5-foot step may also be made.
     */
    FullRound: 'full'
};
