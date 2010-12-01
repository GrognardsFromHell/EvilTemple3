import Qt 4.7

Rectangle {
    width: 640
    height: 480

    CharacterInventoryItem {
        anchors.centerIn: parent

        itemId: 'testId'
        itemIcon: '../art/interface/inventory/Bracers_of_Archery+1.png'
        itemName: 'Bracers of Archery +1'
        itemTooltip: '<b style="color: cyan">Bracers of Archery +1</b><br>This item gives you a +1 bonus to your attack rolls with ranged weapons.'
        itemWeight: '10 lb'
        tooltip: tooltip

        onRightClicked: console.log('Rightclicked!');
    }

    Tooltip {
        id: tooltip
        maxWidth: 250
    }

}
