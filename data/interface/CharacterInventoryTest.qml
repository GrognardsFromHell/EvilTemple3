import Qt 4.7

Rectangle {
    width: 640
    height: 480

    CharacterInventory {
        anchors.centerIn: parent
        items: [
            {id: 'yada', icon: '../art/interface/inventory/Black_TZGY_Scarab.png', name: 'Black Scarab',
            tooltip: '<b style="color: yellow">Black Scarab</b><br>This artifact confers great power to its user.'},
            {id: 'blah', icon: '../art/interface/inventory/Bracers_of_Archery+1.png', name: 'Bracers of Archery +1',
            tooltip: '<b style="color: cyan">Bracers of Archery +1</b><br>This item gives you a +1 bonus to your attack rolls with ranged weapons.'}
        ]

        onRightClicked: console.log('Rightclicked')
    }

}
