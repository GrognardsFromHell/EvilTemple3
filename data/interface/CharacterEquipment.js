
var DragStartPosition;

var EquipmentChecker;

var Slots = {
    // First Row, Left-to-Right
    'gloves': {
        x: 4,
        y: 4
    },
    'helmet': {
        x: 49,
        y: 4
    },
    'cloak': {
        x: 94,
        y: 4
    },
    'robes': {
        x: 139,
        y: 4
    },

    // Second row left-to-right
    'primaryWeapon': {
        x: 4,
        y: 48
    },
    'armor': {
        x: 49,
        y: 48
    },
    'secondaryWeapon': {
        x: 94,
        y: 48
    },
    'bracers': {
        x: 139,
        y: 48
    },

    // Third row left-to-right
    'ammo': {
        x: 4,
        y: 92
    },
    'boots': {
        x: 49,
        y: 92
    },
    'shield': {
        x: 94,
        y: 92
    },
    'instrument': {
        x: 139,
        y: 92
    },

    // Fourth row left-to-right
    'ring1': {
        x: 4,
        y: 136
    },
    'amulet': {
        x: 49,
        y: 136
    },
    'ring2': {
        x: 94,
        y: 136
    },
    'lockpicks': {
        x: 139,
        y: 136
    }
};

function Startup() {
    InitSlotRepeater({});
}

function UpdateEquipment(equipment)
{
    InitSlotRepeater(equipment);
}

function InitSlotRepeater(equipment) {
    var itemCount = 0;
    var model = [];
    for (var k in Slots) {
        var slot = Slots[k];
        var slotModel = {
                   id: k,
                   x: slot.x,
                   y: slot.y
        };

        var equipped = equipment[k];
        if (equipped) {
            slotModel.itemId = equipped.id;
            slotModel.itemIcon = equipped.icon;
            slotModel.itemTooltip = equipped.tooltip;
            itemCount++;
        }

        model.push(slotModel);
    }
    console.debug("Initializing Slot Repeater with " + model.length + " slots and " + itemCount + " equipped items.");

    slotRepeater.model = model;
}

function StartDrag(obj) {
    DragStartPosition = [obj.x, obj.y];
}

function StopDrag(obj) {
    obj.x = DragStartPosition[0];
    obj.y = DragStartPosition[1];
}

function Test(checker) {
    var result = resultObject;
    result.result = false;

    equipCheck('itemId', 'slotId', result);
}
