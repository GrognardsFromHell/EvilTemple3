var CharScreenUi = {};

(function() {

    var dialogCritter;
    var dialog;

    /**
     * Callback that is called by the UI to test whether a given item may be equipped on a certain slot.
     *
     * @param itemId
     * @param slot
     * @param result Result object, whose result property must be set to either true or false.
     */
    function equipCheck(itemId, slot, result) {
        var item = Maps.findMobileById(itemId);

        if (!item) {
            result.result = false;
            return;
        }

        result.result = (item.equipmentSlots.indexOf(slot) !== -1);
    }

    /**
     * Callback function that is called, when the dialog requests an item to be equipped
     * on a certain slot.
     *
     * @param itemId
     * @param slot
     */
    function equipItem(itemId, slot) {
        print("Requesting " + itemId + " to be equipped on slot " + slot);

        var item = Maps.findMobileById(itemId);

        if (!item)
            return;

        dialogCritter.equip(item, slot);
    }

    function convertItem(item) {
        var tooltip = '<b>' + item.getName() + '</b><br>' + item.type + '<br>' + item.weight + ' lb';

        if (item.equipmentSlots.length > 0) {
            tooltip += '<br>Slots: ';
            for (var i = 0; i < item.equipmentSlots.length; ++i) {
                if (i > 0)
                    tooltip += ', ';
                tooltip += item.equipmentSlots[i];
            }
        }

        if (item.containedIn)
            tooltip += '<br>Contained in: ' + item.containedIn.id;

        if (item.equippedBy)
            tooltip += '<br>Equipped by: ' + item.equippedBy.id;

        return {
            id: item.id,
            icon: '../' + getInventoryIconPath(item.inventoryId),
            name: item.getName(),
            tooltip: tooltip,
            weight: item.weight ? (item.weight + ' lb') : ''
        };
    }

    function updateInventory(dialog, critter) {
        if (critter.content) {
            dialog.inventory = critter.content.map(convertItem);
        } else {
            dialog.inventory = [];
        }
    }

    function updateEquipment(dialog, critter) {
        if (critter.equipment) {
            var equipment = {};

            for (var slot in critter.equipment) {
                equipment[slot] = convertItem(critter.equipment[slot]);
            }

            dialog.equipment = equipment;
        } else {
            dialog.equipment = {};
        }
    }

    function moveItemToInventory(itemId) {
        print("Moving item to inventory: " + itemId);
        var item = Maps.findMobileById(itemId); // TODO: MobileRegistry or something similar

        if (item && item.containedIn !== dialogCritter) {
            dialogCritter.giveItem(item);
        }
    }

    function updateCharacterSheet(dialog, critter) {
        var sheet = {
            strength: critter.strength,
            dexterity: critter.dexterity,
            constitution: critter.constitution,
            intelligence: critter.intelligence,
            wisdom: critter.wisdom,
            charisma: critter.charisma,

            height: critter.height,
            weight: critter.weight
        };

        var level = critter.getEffectiveCharacterLevel();

        if (level > 0) {
            sheet.level = level;
            sheet.experience = critter.experiencePoints;
            sheet.fortitudeSave = critter.getFortitudeSave();
            sheet.willSave = critter.getWillSave();
            sheet.reflexSave = critter.getReflexSave();
            sheet.hitPoints = critter.hitPoints;

            var bab = critter.getBaseAttackBonus();
            sheet.meleeBonus = bab + getAbilityModifier(critter.getEffectiveStrength());
            sheet.rangedBonus = bab + getAbilityModifier(critter.getEffectiveDexterity());

            sheet.initiative = critter.getInitiativeBonus();
            sheet.speed = critter.getEffectiveLandSpeed();
        }

        dialog.characterSheet = sheet;
    }

    function updatePartyGold(dialog) {
        dialog.money = Party.money.getTotalCopper();
    }

    /**
     * Shows the character screen for the given critter and closes the current character screen.
     * @param critter The critter.
     */
    CharScreenUi.show = function(critter) {
        if (dialog)
            dialog.deleteLater();

        dialogCritter = critter; // TODO: Remove this hack

        dialog = gameView.addGuiItem('interface/CharacterScreen.qml');
        dialog.closeClicked.connect(CharScreenUi.close);
        dialog.equipCheck.connect(equipCheck);
        dialog.equipItem.connect(equipItem);
        dialog.moveItemToInventory.connect(moveItemToInventory);
        dialog.itemRightClicked.connect(function (itemId) {
            print("Right clicked on: " + itemId);
            var item = Maps.findMobileById(itemId);
            if (item)
                showMobileInfo(item, null);
        });

        // Set the inventory
        updateInventory(dialog, critter);
        updateEquipment(dialog, critter);
        updateCharacterSheet(dialog, critter);
        updatePartyGold(dialog);
    };

    CharScreenUi.close = function() {
        if (!dialog)
            return;

        dialog.deleteLater();
        dialog = null;
    };

    EventBus.addListener(EventTypes.EquippedItem, function(critter, item) {
        if (dialogCritter === critter)
            updateEquipment(dialog, critter);
    });

    EventBus.addListener(EventTypes.UnequippedItem, function(critter, item) {
        if (dialogCritter === critter)
            updateEquipment(dialog, critter);
    });

    EventBus.addListener(EventTypes.ItemAddedToInventory, function(critter, item) {
        if (dialogCritter === critter)
            updateInventory(dialog, critter);
    });

    EventBus.addListener(EventTypes.ItemRemovedFromInventory, function(critter, item) {
        if (dialogCritter === critter)
            updateInventory(dialog, critter);
    });

})();
