import Qt 4.7

Rectangle {
    width: 800
    height: 600

    CharacterScreen {
        anchors.centerIn: parent

        inventoryItems: [
            {'id': 'abc', 'name': 'bac', 'icon': '../art/interface/inventory/Bracers_of_Archery+1.png',
            tooltip: 'Tooltip'}
        ]
    }

}
