import Qt 4.7

/**
  This QML component serves as a drag and drop spot for items.
  */
Item {
    width: 44
    height: 44

    property string itemId

    property string itemIcon

    property string itemTooltip

    signal itemDroppedOn(string id)

    Rectangle {
        anchors.fill: parent
        radius: 5
        color: 'black'
    }

    DragArea {
        id: dragArea
    }

    DropArea {
        id: dropArea

        onDrop: {
            console.log('Dropped item');
        }
    }

    Image {
        id: itemIcon
    }
}
