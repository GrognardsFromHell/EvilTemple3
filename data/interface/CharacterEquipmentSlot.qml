import Qt 4.7

Item {
    id: root
    width: 44
    height: 44

    property string slot

    property string itemIcon

    property string itemId

    property string itemTooltip

    signal equipCheck(string itemId, string slot, variant result)

    signal equipItem(string itemId)

    Component {
        id: dragDelegate
        Item {
            width:  42
            height: 42
            z: 1000

            Rectangle {
                anchors.fill:  parent
                color: "#000000"
                opacity: 0.5
                border.color: '#FFFFFF'
                radius: 5
            }

            Image {
                anchors.fill:  parent
                source: dragData.icon
            }
        }
    }

    /*
      This rectangle covers the "slot icon" with a black square, when an item is equipped in the slot.
      Otherwise the transparent icon of the item overlays the slot icon leading to visual confusion.
     */
    Rectangle {
        x: 1
        y: 1
        width: 40
        height: 40

        id: bgRectangle
        color: '#000000'
        opacity: 0

        states: [
            State {
                when: root.itemId != ''
                PropertyChanges {
                    target: bgRectangle
                    opacity: 1
                }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation { property: "opacity"; duration: 100 }
            }

        ]
    }

    /*
      This rectangle displays a green border around an inventory slot when an item is being draged.
      It indicates whether this slot is valid for the dragged item.
      */
    Rectangle {
        id: feedbackRectangle
        color: 'transparent'
        border.color: "#7f3d942b"
        border.width: 2
        opacity: 0

        z: 1

        // The width/height here are misleading since they have to account for the border
        width: 41
        height: 41

        transitions: [
            Transition {
                NumberAnimation { property: "opacity"; duration: 100 }
            }
        ]
    }

    /*
      This rectangle displays a white border around an inventory slot when an item is hovered above it.
      */
    Rectangle {
        id: hoverRectangle
        color: 'transparent'
        border.color: '#ffffff'
        opacity: 0

        // The width/height here are misleading since they have to account for the border
        width: 41
        height: 41

        z: 2

        states: [
            State {
                name: "hovering"
                PropertyChanges {
                    target: hoverRectangle
                    opacity: 1
                }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation { property: "opacity"; duration: 100 }
            }

        ]
    }

    /*
      Used to display item icons in inventory slots.
      */
    Image {
        source: itemIcon
        smooth: true
        anchors.fill: parent
    }

    DropArea {
        anchors.fill: parent
        onDropAccept: {
            root.equipCheck(event.data.id, slot, resultObject);
            if (resultObject.result)
                event.accepted = true;
        }
        onDropEnter: hoverRectangle.state = 'hovering'
        onDropLeave: hoverRectangle.state = ''
        onDrop: {
            hoverRectangle.state = '';
            equipItem(event.data.id);
        }
        onDragStart: {
            root.equipCheck(event.data.id, slot, resultObject);
            if (resultObject.result)
                feedbackRectangle.opacity = 1;
            else
                feedbackRectangle.opacity = 0;
        }
        onDragSuccess: feedbackRectangle.opacity = 0
        onDragFailure: feedbackRectangle.opacity = 0
    }

    DragArea {
        delegate: dragDelegate
        anchors.fill: parent
        delegateX: -20
        delegateY: -20
        enabled: itemId != ''
        data : ({
            type: 'Item',
            id: itemId,
            icon: itemIcon
            })
    }

    states: [
        State {
            name: "dropOkay"
            PropertyChanges {
                target: rectangle
                border.color: 'green'
            }
        }
    ]

    ResultObject {
        id: resultObject
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        enabled: itemTooltip != ''
        hoverEnabled: true
        acceptedButtons: Qt.NoButton
    }

    Tooltip {
        id: tooltip
        shown: mouseArea.containsMouse
        anchors.horizontalCenter: root.horizontalCenter
        anchors.top: root.bottom
        text: itemTooltip
    }

}
