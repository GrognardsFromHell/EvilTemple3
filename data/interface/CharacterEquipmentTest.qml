import Qt 4.7

Rectangle {
    width: 640
    height: 480

    CharacterEquipment {
        id: charEquipment
        anchors.centerIn: parent

        onEquipCheck: {
            console.debug("EquipCheck: " + itemId + ' on slot ' + slot);
            result.result = true;
        }

        onEquipItem: {
            console.log('Equipping item ' + itemId + ' on slot ' + slot);
        }
    }

    Component.onCompleted: {
        charEquipment.equipment = {
            amulet: {
                id: 'blah',
                icon: '../art/interface/inventory/Black_TZGY_Scarab.png',
                tooltip: 'Black Scarab'
            },
            bracers: {
                id: 'yada',
                icon: '../art/interface/inventory/Bracers_of_Archery+1.png',
                tooltip: 'Bracers of Archery +1'
            }
        };
    }
}
