import Qt 4.7
import 'CharacterEquipment.js' as CharacterEquipment

Item {
    id: root
    width: 185
    height: 182

    property variant equipment : []

    signal equipCheck(string itemId, string slot, variant result)

    signal equipItem(string itemId, string slot)

    function test() {
        CharacterEquipment.Test();
    }

    Item {
        id: xchange
    }

    onEquipmentChanged: CharacterEquipment.UpdateEquipment(equipment)

    Image {
        source: "C:/Users/Sebastian/Code/EvilTemple/data/art/interface/CHAR_UI/CHAR_PORTRAIT_UI/Equipped_Items.png"
        width: 185
        height: 182
        z: 0
    }

    Component {
        id: slotComponent

        CharacterEquipmentSlot {
            x: modelData.x
            y: modelData.y
            slot: modelData.id
            itemId: modelData.itemId ? modelData.itemId : ''
            itemIcon: modelData.itemIcon ? modelData.itemIcon : ''
            itemTooltip: modelData.itemTooltip ? modelData.itemTooltip : ''

            onEquipCheck: root.equipCheck(itemId, slot, result)

            onEquipItem: root.equipItem(itemId, slot)
        }
    }

    Repeater {
        id: slotRepeater
        delegate: slotComponent
    }

}
