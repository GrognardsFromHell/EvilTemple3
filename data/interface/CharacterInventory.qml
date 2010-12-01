import Qt 4.7

/**
  Describes the character's inventory listing.
  */
Rectangle {
    id: rootWidget
    width: 640
    height: 480

    color: 'black'

    property variant items : []

    signal rightClicked(string itemId)

    signal moveItemToInventory(string itemId)

    Component {
        id: itemDelegate
        CharacterInventoryItem {
            anchors.left: parent.left
            anchors.right: parent.right
            lastItem: (index + 1) >= items.length

            itemId: modelData.id
            itemIcon: modelData.icon
            itemName: modelData.name
            itemTooltip: modelData.tooltip
            itemWeight: modelData.weight ? modelData.weight : ''
            tooltip: tooltipItem

            onRightClicked: rootWidget.rightClicked(modelData.id)
        }
    }

    ScrollView {
        id: itemsView
        model: items
        delegate: itemDelegate
        anchors.fill: parent
    }

    Rectangle {
        anchors.fill: itemsView
        id: feedbackRectangle
        color: 'transparent'
        border.color: 'green'
        border.width:  2
        opacity: 0
        transitions: [
            Transition {
                from: "*"
                to: "*"
                NumberAnimation { property: "opacity"; duration: 200 }
            }
        ]

    }

    DropArea {
        anchors.fill: itemsView
        onDropAccept: {
            if (event.data.type == 'Item')
                event.accepted = true;
        }
        onDropEnter: feedbackRectangle.opacity = 1
        onDropLeave: feedbackRectangle.opacity = 0.5
        onDrop: {
            feedbackRectangle.opacity = 0;
            rootWidget.moveItemToInventory(event.data.id);
        }
        onDragStart: {
            feedbackRectangle.opacity = 0.5;
        }
        onDragSuccess: feedbackRectangle.opacity = 0
        onDragFailure: feedbackRectangle.opacity = 0
    }

    Tooltip {
        id: tooltipItem
        maxWidth: 250
        z: 1000
    }

}
