import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import CustomControls 1.0

Window {
    width: 1200
    height: 800
    visible: true
    title: "Data Visualizer"
    color: "#05050A"

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#070514" } 
            GradientStop { position: 0.3; color: "#191030" } 
            GradientStop { position: 0.6; color: "#2B1A4A" } 
            GradientStop { position: 0.85; color: "#141C3A" } 
            GradientStop { position: 1.0; color: "#050814" } 
        }
    }

    DropArea {
        anchors.fill: parent
        keys: ["text/uri-list"]
        onDropped: (drop) => {
            if (drop.hasUrls) {
                for (let i = 0; i < drop.urls.length; ++i) {
                    let url = drop.urls[i].toString();
                    if (url.endsWith(".txt")) {
                        let selectedColor = colorModel.get(colorSelector.currentIndex).code;
                        plotItem.loadData(url, selectedColor);
                        break;
                    }
                }
            }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 25
            spacing: 20

            Rectangle {
                Layout.fillWidth: true
                Layout.minimumHeight: 180
                color: "#1AFFFFFF" 
                radius: 16
                border.color: "#30FFFFFF"
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 15

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 15

                        Label {
                            text: "Data Visualizer"
                            color: "#FFFFFF"
                            font.pixelSize: 28
                            font.bold: true
                            font.family: "Segoe UI, Helvetica, sans-serif"
                            Layout.fillWidth: true
                        }

                        ComboBox {
                            id: colorSelector
                            font.pixelSize: 15
                            implicitWidth: 200
                            implicitHeight: 40

                            model: ListModel {
                                id: colorModel
                                ListElement { name: "cyan"; code: "#00ffff" }
                                ListElement { name: "blue"; code: "#0000ff" }
                                ListElement { name: "red"; code: "#ff0000" }
                                ListElement { name: "green"; code: "#00ff00" }
                                ListElement { name: "black"; code: "#000000" }
                                ListElement { name: "darkCyan"; code: "#008080" }
                                ListElement { name: "darkRed"; code: "#800000" }
                                ListElement { name: "magenta"; code: "#ff00ff" }
                                ListElement { name: "darkMagenta"; code: "#800080" }
                                ListElement { name: "darkGreen"; code: "#008000" }
                                ListElement { name: "yellow"; code: "#ffff00" }
                                ListElement { name: "darkYellow"; code: "#808000" }
                                ListElement { name: "darkBlue"; code: "#000080" }
                                ListElement { name: "gray"; code: "#a0a0a4" }
                                ListElement { name: "darkGray"; code: "#808080" }
                                ListElement { name: "lightGray"; code: "#c0c0c0" }
                            }
                            textRole: "name"

                            popup: Popup {
                                y: colorSelector.height + 5
                                width: colorSelector.width
                                implicitHeight: contentItem.implicitHeight
                                padding: 5

                                contentItem: ListView {
                                    clip: true
                                    implicitHeight: contentHeight
                                    model: colorSelector.delegateModel
                                    currentIndex: colorSelector.highlightedIndex
                                    ScrollIndicator.vertical: ScrollIndicator { }
                                }

                                background: Rectangle {
                                    color: "#1A1525" 
                                    border.color: "#40FFFFFF"
                                    border.width: 1
                                    radius: 10
                                }
                            }

                            delegate: ItemDelegate {
                                width: colorSelector.width
                                background: Rectangle {
                                    color: parent.highlighted ? "#3300E5FF" : "transparent"
                                    radius: 6 
                                }
                                contentItem: Row {
                                    spacing: 10
                                    Rectangle {
                                        width: 16; height: 16; radius: 8
                                        color: model.code
                                        border.color: "#808080"; border.width: 1
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                    Text {
                                        text: model.name
                                        font.pixelSize: 15
                                        color: "#FFFFFF"
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                }
                            }

                            contentItem: Item {
                                anchors.fill: parent
                                Row {
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.left: parent.left; anchors.leftMargin: 15
                                    spacing: 10
                                    Rectangle {
                                        width: 16; height: 16; radius: 8
                                        color: colorSelector.currentIndex >= 0 ? colorModel.get(colorSelector.currentIndex).code : "transparent"
                                        border.color: "#808080"; border.width: 1
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                    Text {
                                        text: colorSelector.currentText
                                        font.pixelSize: 15
                                        color: "#FFFFFF"
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                }
                            }

                            background: Rectangle {
                                color: "#1AFFFFFF"
                                border.color: colorSelector.pressed ? "#00A2FF" : (colorSelector.hovered ? "#60FFFFFF" : "#40FFFFFF")
                                border.width: 1
                                radius: 8
                                Behavior on border.color { ColorAnimation { duration: 150 } }
                            }

                            indicator: Canvas {
                                x: colorSelector.width - width - 15
                                y: colorSelector.topPadding + (colorSelector.availableHeight - height) / 2
                                width: 12
                                height: 8
                                contextType: "2d"
                                Connections {
                                    target: colorSelector
                                    function onPressedChanged() { indicator.requestPaint(); }
                                    function onHoveredChanged() { indicator.requestPaint(); }
                                }
                                onPaint: {
                                    var context = getContext("2d");
                                    context.reset();
                                    context.moveTo(0, 0);
                                    context.lineTo(width, 0);
                                    context.lineTo(width / 2, height);
                                    context.closePath();
                                    context.fillStyle = colorSelector.pressed ? "#00A2FF" : (colorSelector.hovered ? "#FFFFFF" : "#A0AAB5");
                                    context.fill();
                                }
                            }
                        }

                        Button {
                            text: "🧹 Clear"
                            font.pixelSize: 15
                            flat: true 
                            background: Rectangle { 
                                implicitWidth: 110; implicitHeight: 40; 
                                color: parent.down ? "#4DFF4D4D" : (parent.hovered ? "#26FF4D4D" : "#1AFF4D4D")
                                border.color: parent.hovered ? "#80FF4D4D" : "#40FF4D4D"
                                border.width: 1
                                radius: 8 
                                Behavior on color { ColorAnimation { duration: 150 } }
                                Behavior on border.color { ColorAnimation { duration: 150 } }
                            }
                            contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font.pixelSize: 15; font.bold: true }
                            onClicked: plotItem.clearPlot()
                        }

                        Button {
                            text: "🔍 Reset Scale"
                            font.pixelSize: 15
                            flat: true
                            background: Rectangle { 
                                implicitWidth: 140; implicitHeight: 40; 
                                color: parent.down ? "#4DFFA64D" : (parent.hovered ? "#26FFA64D" : "#1AFFA64D")
                                border.color: parent.hovered ? "#80FFA64D" : "#40FFA64D"
                                border.width: 1
                                radius: 8 
                                Behavior on color { ColorAnimation { duration: 150 } }
                                Behavior on border.color { ColorAnimation { duration: 150 } }
                            }
                            contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font.pixelSize: 15; font.bold: true }
                            onClicked: plotItem.resetScale()
                        }

                        Button {
                            text: "📁 Load .txt"
                            font.pixelSize: 15
                            flat: true
                            background: Rectangle { 
                                implicitWidth: 140; implicitHeight: 40; 
                                color: parent.down ? "#4D4D94FF" : (parent.hovered ? "#264D94FF" : "#1A4D94FF")
                                border.color: parent.hovered ? "#804D94FF" : "#404D94FF"
                                border.width: 1
                                radius: 8 
                                Behavior on color { ColorAnimation { duration: 150 } }
                                Behavior on border.color { ColorAnimation { duration: 150 } }
                            }
                            contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font.pixelSize: 15; font.bold: true }
                            onClicked: fileDialog.open()
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 15

                        Label {
                            text: "f(x) ="
                            color: "#00E5FF" 
                            font.pixelSize: 20
                            font.bold: true
                            font.family: "Consolas, monospace"
                        }

                        ComboBox {
                            id: formulaInput
                            Layout.fillWidth: true
                            editable: true 
                            font.pixelSize: 16
                            rightPadding: 40
                            
                            model: ListModel {
                                id: formulaModel
                                // 1. Формула для IG-L: использует z и w из C++, а также делит итоговое значение на scale
                                ListElement { text: "(abs(shape) < 1e-9) ? (0.5 * exp(-z) / scale) : ((w / (2 * sqrt(1 + 2*shape*z))) * exp((w/shape) * (1 - sqrt(1 + 2*shape*z))) / scale)" }
                                // 2. Равномерное: использует shift как центр отрезка, scale - как полуширину
                                ListElement { text: "(x >= (shift - scale) && x <= (shift + scale)) ? (1 / (2 * scale)) : 0.0" }
                                // 3. Нормальное: shift - мат. ожидание, scale - среднекв. отклонение
                                ListElement { text: "(1 / (scale * sqrt(2 * PI))) * exp(-pow(x - shift, 2) / (2 * scale * scale))" }
                            }
                            textRole: "text"

                            popup: Popup {
                                y: formulaInput.height + 5
                                width: formulaInput.width
                                implicitHeight: contentItem.implicitHeight
                                padding: 5

                                contentItem: ListView {
                                    clip: true
                                    implicitHeight: contentHeight
                                    model: formulaInput.delegateModel
                                    currentIndex: formulaInput.highlightedIndex
                                    ScrollIndicator.vertical: ScrollIndicator { }
                                }

                                background: Rectangle {
                                    color: "#1A1525" 
                                    border.color: "#40FFFFFF"
                                    border.width: 1
                                    radius: 10
                                }
                            }

                            delegate: ItemDelegate {
                                width: formulaInput.width
                                background: Rectangle {
                                    color: parent.highlighted ? "#3300E5FF" : "transparent"
                                    radius: 6
                                }
                                contentItem: Text {
                                    text: model.text
                                    color: "#FFFFFF"
                                    font.pixelSize: 15
                                    font.family: "Consolas, monospace"
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }

                            contentItem: TextField {
                                text: formulaInput.editText
                                font.pixelSize: 16
                                font.family: "Consolas, monospace"
                                color: "#FFFFFF"
                                selectionColor: "#00A2FF"
                                verticalAlignment: Text.AlignVCenter
                                background: Item {} 
                                onAccepted: formulaInput.accepted()
                            }

                            background: Rectangle { 
                                color: "#1AFFFFFF"
                                border.color: formulaInput.activeFocus ? "#00E5FF" : (formulaInput.hovered ? "#60FFFFFF" : "#40FFFFFF")
                                border.width: 1
                                radius: 8 
                                Behavior on border.color { ColorAnimation { duration: 150 } }
                            }

                            indicator: Canvas {
                                x: formulaInput.width - width - 15
                                y: formulaInput.topPadding + (formulaInput.availableHeight - height) / 2
                                width: 12
                                height: 8
                                contextType: "2d"
                                Connections {
                                    target: formulaInput
                                    function onPressedChanged() { indicator.requestPaint(); }
                                    function onHoveredChanged() { indicator.requestPaint(); }
                                }
                                onPaint: {
                                    var context = getContext("2d");
                                    context.reset();
                                    context.moveTo(0, 0);
                                    context.lineTo(width, 0);
                                    context.lineTo(width / 2, height);
                                    context.closePath();
                                    context.fillStyle = formulaInput.pressed ? "#00A2FF" : (formulaInput.hovered ? "#FFFFFF" : "#A0AAB5");
                                    context.fill();
                                }
                            }
                        }

                        Button {
                            text: "📈 Plot"
                            flat: true
                            background: Rectangle { 
                                implicitWidth: 110; implicitHeight: 40;
                                color: parent.down ? "#4D00E676" : (parent.hovered ? "#2600E676" : "#1A00E676")
                                border.color: parent.hovered ? "#8000E676" : "#4000E676"
                                border.width: 1
                                radius: 8 
                                Behavior on color { ColorAnimation { duration: 150 } }
                                Behavior on border.color { ColorAnimation { duration: 150 } }
                            }
                            contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font.pixelSize: 16; font.bold: true }
                            onClicked: {
                                // ОБНОВЛЕНО: Считываем унифицированные параметры
                                let shiftVal = parseFloat(shiftInput.text.replace(',', '.'));
                                let scaleVal = parseFloat(scaleInput.text.replace(',', '.'));
                                let shapeVal = parseFloat(shapeInput.text.replace(',', '.'));

                                let xMinVal = parseFloat(xMinInput.text.replace(',', '.'));
                                let xMaxVal = parseFloat(xMaxInput.text.replace(',', '.'));
                                if (isNaN(xMinVal)) xMinVal = -5.0;
                                if (isNaN(xMaxVal)) xMaxVal = 5.0;

                                let currentFormula = formulaInput.editText.trim(); 
                                let selectedColor = colorModel.get(colorSelector.currentIndex).code;
                                
                                if (currentFormula !== "") {
                                    let foundIndex = formulaInput.find(currentFormula);
                                    if (foundIndex === -1) {
                                        formulaModel.append({ text: currentFormula });
                                        foundIndex = formulaModel.count - 1;
                                    }
                                    formulaInput.currentIndex = foundIndex;
                                    
                                    plotItem.plotCustomFunction(currentFormula, shiftVal, scaleVal, shapeVal, selectedColor, xMinVal, xMaxVal);
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 15

                        component StyledTextField : TextField {
                            color: "#FFFFFF"
                            font.pixelSize: 15
                            implicitWidth: 65
                            horizontalAlignment: TextInput.AlignHCenter
                            selectionColor: "#00A2FF"
                            background: Rectangle { 
                                color: parent.hovered ? "#25FFFFFF" : "#1AFFFFFF"
                                border.color: parent.activeFocus ? "#00E5FF" : "#40FFFFFF"
                                border.width: 1
                                radius: 6 
                                Behavior on color { ColorAnimation { duration: 150 } }
                                Behavior on border.color { ColorAnimation { duration: 150 } }
                            }
                        }

                        RowLayout {
                            spacing: 8
                            Label { text: "Xmin:"; color: "#A0AAB5"; font.pixelSize: 16; font.italic: true }
                            StyledTextField { id: xMinInput; text: "-5.0" }
                        }

                        RowLayout {
                            spacing: 8
                            Label { text: "Xmax:"; color: "#A0AAB5"; font.pixelSize: 16; font.italic: true }
                            StyledTextField { id: xMaxInput; text: "5.0" }
                        }
                        
                        Rectangle { width: 1; height: 25; color: "#40FFFFFF"; radius: 1; Layout.leftMargin: 5; Layout.rightMargin: 5 }

                        // ОБНОВЛЕНО: Три унифицированных поля ввода параметров
                        RowLayout {
                            spacing: 8
                            Label { text: "Shift:"; color: "#A0AAB5"; font.pixelSize: 16; font.italic: true }
                            StyledTextField { id: shiftInput; text: "0.0" }
                        }

                        RowLayout {
                            spacing: 8
                            Label { text: "Scale:"; color: "#A0AAB5"; font.pixelSize: 16; font.italic: true }
                            StyledTextField { id: scaleInput; text: "1.0" }
                        }

                        RowLayout {
                            spacing: 8
                            Label { text: "Shape:"; color: "#A0AAB5"; font.pixelSize: 16; font.italic: true }
                            StyledTextField { id: shapeInput; text: "2.0" }
                        }
                        
                        Item { Layout.fillWidth: true } // Spacer
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#FFFFFF" 
                radius: 16
                border.color: "#40FFFFFF" 
                border.width: 1
                clip: true

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: -1
                    color: "transparent"
                    border.color: "#0B0B1A"
                    border.width: 4
                    opacity: 0.1
                    radius: 16
                    z: 1 
                }

                CustomPlot {
                    id: plotItem
                    anchors.fill: parent
                    anchors.margins: 10 
                }
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "Select .txt file with points"
        nameFilters: ["Text files (*.txt)"]
        onAccepted: {
            let selectedColor = colorModel.get(colorSelector.currentIndex).code;
            plotItem.loadData(selectedFile, selectedColor);
        }
    }
}