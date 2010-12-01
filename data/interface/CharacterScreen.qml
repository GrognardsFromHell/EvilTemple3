import Qt 4.7

Item {

    property alias inventory : inventoryWidget.items

    property alias equipment : equipmentWidget.equipment

    property alias characterSheet : createcharacterpaperdoll1.sheet

    property alias money : moneyDisplay.money

    signal equipCheck(string itemId, string slot, variant result)

    signal equipItem(string itemId, string slot)

    signal itemRightClicked(string itemId)

    signal moveItemToInventory(string itemId)

    signal closeClicked

    id: rootWidget
    width: 671
    height: 464

    x: (gameView.viewportSize.width - width) / 2
    y: (gameView.viewportSize.height - height) / 2

    Image {
        id: background
        x: 0
        y: 0
        source: '../art/interface/CHAR_UI/main_window.png'
        width: 671
        height: 464

        CreateCharacterPaperdoll {
            id: createcharacterpaperdoll1
            x: 21
            y: 267
        }

        CharacterEquipment {
            id: equipmentWidget
            x: 21
            y: 42

            // Forward these events to the external interface
            onEquipCheck: rootWidget.equipCheck(itemId, slot, result)
            onEquipItem: rootWidget.equipItem(itemId, slot)
        }

        CharacterInventory {
            id: inventoryWidget
            x: 250
            y: 71
            width: 405
            height: 271

            onRightClicked: rootWidget.itemRightClicked(itemId)
            onMoveItemToInventory: rootWidget.moveItemToInventory(itemId)
        }

        Rectangle {
            id: rectangle1
            x: 18
            y: 245
            width: 194
            height: 16
            color: "#000000"
        }

        MoneyDisplay {
            id: moneyDisplay
            x: 250
            y: 347
            money: 99
        }
    }

    Button {
        x: 608
        y: 403
        text: ''
        normalImage: 'art/interface/CHAR_UI/main_exit_button_hover_off.png'
        disabledImage: 'art/interface/CHAR_UI/main_exit_button_disabled.png'
        hoverImage: 'art/interface/CHAR_UI/main_exit_button_hover_on.png'
        pressedImage: 'art/interface/CHAR_UI/main_exit_button_pressed.png'
        onClicked: closeClicked()
    }
}
