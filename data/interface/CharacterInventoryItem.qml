import Qt 4.7

Rectangle {
    id: root
    width: 320
    height: 48

    property string itemId
    property string itemIcon
    property string itemName
    property string itemTooltip
    property string itemWeight
    property variant tooltip

    property bool lastItem : false

    signal rightClicked

    color: 'black'

    // Item icon
    Image {
        id: icon
        source: itemIcon
        width: 42
        height: 42
        smooth: true
    }

    StandardText {
        id: itemNameWidget
        anchors.leftMargin: 5
        anchors.left: icon.right
        anchors.right: parent.right
        anchors.topMargin: 5
        anchors.top:  parent.top
        font.bold: true
        text: itemName
        horizontalAlignment: "AlignLeft"
    }

    StandardText {
        id: itemWeightWidget
        anchors.left: itemNameWidget.left
        anchors.bottomMargin: 5
        anchors.bottom:  parent.bottom
        text: itemWeight
        horizontalAlignment: "AlignLeft"
    }

    Component {
        id: itemDragDelegate
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

    DragArea {
        anchors.fill: parent
        data: ({
            id: itemId,
            icon: itemIcon,
            type: 'Item'
        })
        delegate: itemDragDelegate
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.RightButton
        onClicked: root.rightClicked()
        onEntered: {
            if (tooltip) {
                tooltip.text = itemTooltip;
                tooltip.shown = true;
            }
        }
        onMousePositionChanged: {
            if (tooltip) {
                var pos = root.mapToItem(tooltip.parent, mouse.x, mouse.y);
                tooltip.x = pos.x + 10;
                tooltip.y = pos.y;
            }
        }
        onExited: if (tooltip) tooltip.shown = false;
    }

    // Lower edge border
    Rectangle {
        visible: !lastItem
        color: 'white'
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        height: 1
    }
}
