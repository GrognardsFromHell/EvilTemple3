import Qt 4.7

MovableWindow {
    id: root
    width: 350
    height: 480
    title: 'Inventory'

    property alias money : moneyDisplay.money
    property variant items : {
                             objects: [{
                                    iconPath: 'art/interface/inventory/Sword_2-Handed3_Icon.png',
                                    description: 'Some Sword',
                                    worth: 1000,
                                    weight: 10
                                    }]
                             }

    signal itemLeftClicked(string guid)
    signal itemRightClicked(string guid)
    signal itemDoubleClicked(string guid)

    function removeItem(guid) {
        for (var i = 0; i < listModel.count; ++i) {
            if (listModel.get(i).guid == guid) {
                listModel.remove(i);
                return;
            }
        }
    }

    onItemsChanged: {
        var objects = items.objects;
        listModel.clear();
        for (var i = 0; i < objects.length; ++i) {
            listModel.append(objects[i]);
        }
    }

    Component {
        id: delegate

        InventoryItem {
            iconPath: model.iconPath
            quantity: model.quantity
            location: model.location ? model.location : 0
            description: model.description
            magical: model.magical
            weight: model.weight
            worth: model.worth

            acceptedButtons: Qt.LeftButton|Qt.RightButton
            anchors.left: parent ? parent.left : undefined
            anchors.right: parent ? parent.right : undefined
            onClicked: if (mouse.button == Qt.LeftButton) {
                itemLeftClicked(guid)
            } else if (mouse.button == Qt.RightButton) {
                itemRightClicked(guid)
            }

            onDoubleClicked: if (mouse.button == Qt.LeftButton) {
                itemDoubleClicked(guid)
            }
        }
    }

    ListModel {
        id: listModel
    }

    ListView {
        id: listView
        anchors.fill: parent
        anchors.margins: 6
        anchors.topMargin: 45
        anchors.bottomMargin: 40
        model: listModel
        delegate: delegate
        clip: true
        spacing: 5
    }

    MoneyDisplay {
        id: moneyDisplay
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 6
    }

}
