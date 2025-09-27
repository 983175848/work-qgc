/****************************************************************************
 *
 * (c) 2009-2022 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls




import QGroundControl.FactControls

RowLayout {
    id:         control
    spacing:    0

    property bool   showIndicator:          true
    property var    expandedPageComponent
    property bool   waitForParameters:      false

    property real fontPointSize:    ScreenTools.largeFontPointSize
    property var  activeVehicle:    QGroundControl.multiVehicleManager.activeVehicle
    property bool allowEditMode:    true
    property bool editMode:         false

    function flightModeDisplayName(mode) {
            switch(mode) {
                case "Manual": return qsTr("Manual");
                case "Acro": return qsTr("Acro");
                case "Learning": return qsTr("Learning");
                case "Steering": return qsTr("Steering");
                case "Hold": return qsTr("Hold");
                case "Loiter": return qsTr("Loiter");
                case "Follow": return qsTr("Follow");
                case "Simple": return qsTr("Simple");
                case "Dock": return qsTr("Dock");
                case "Circle": return qsTr("Circle");
                case "Auto": return qsTr("Auto");
                case "RTL": return qsTr("RTL");
                case "Smart RTL": return qsTr("Smart RTL");
                case "Guided": return qsTr("Guided");
                case "lnitializing": return qsTr("lnitializing");
                //在case处继续补充所有需要的模式
                default: return mode;
            }
        }

    RowLayout {
        Layout.fillWidth: true

        QGCColoredImage {
            id:         flightModeIcon
            width:      ScreenTools.defaultFontPixelWidth * 3
            height:     ScreenTools.defaultFontPixelHeight
            fillMode:   Image.PreserveAspectFit
            mipmap:     true
            color:      qgcPal.text
            source:     "/qmlimages/FlightModesComponentIcon.png"
        }
        // 当前飞行模式文本
        QGCLabel {
            //text:               activeVehicle ? activeVehicle.flightMode : qsTr("N/A", "No data to display")
            // 原有的飞行模式来自c++返回的字符串，无法通过qstr进行翻译
            text:               activeVehicle ? flightModeDisplayName(activeVehicle.flightMode) : qsTr("N/A", "No data to display")
            font.pointSize:     fontPointSize
            Layout.alignment:   Qt.AlignCenter
            // 点击可展开模式选择抽屉
            MouseArea {
                anchors.fill:   parent
                onClicked:      mainWindow.showIndicatorDrawer(drawerComponent, control)
            }
        }
    }

    Component {
        id: drawerComponent

        ToolIndicatorPage {
            showExpand:         true
            waitForParameters:  control.waitForParameters

            contentComponent:    flightModeContentComponent
            expandedComponent:   flightModeExpandedComponent

            onExpandedChanged: {
                if (!expanded) {
                    editMode = false
                }
            }
        }
    }

    Component {
        id: flightModeContentComponent

        ColumnLayout {
            id:         modeColumn
            spacing:    ScreenTools.defaultFontPixelWidth / 2

            property var    activeVehicle:            QGroundControl.multiVehicleManager.activeVehicle
            property var    flightModeSettings:       QGroundControl.settingsManager.flightModeSettings
            property var    hiddenFlightModesFact:    null
            property var    hiddenFlightModesList:    [] 

            Component.onCompleted: {
                // Hidden flight modes are classified by firmware and vehicle class
                var hiddenFlightModesPropPrefix
                if (activeVehicle.px4Firmware) {
                    hiddenFlightModesPropPrefix = "px4HiddenFlightModes"
                } else if (activeVehicle.apmFirmware) {
                    hiddenFlightModesPropPrefix = "apmHiddenFlightModes"
                } else {
                    control.allowEditMode = false
                }
                if (control.allowEditMode) {
                    var hiddenFlightModesProp = hiddenFlightModesPropPrefix + activeVehicle.vehicleClassInternalName()
                    if (flightModeSettings.hasOwnProperty(hiddenFlightModesProp)) {
                        hiddenFlightModesFact = flightModeSettings[hiddenFlightModesProp]
                        // Split string into list of flight modes
                        if (hiddenFlightModesFact && hiddenFlightModesFact.value !== "") {
                            hiddenFlightModesList = hiddenFlightModesFact.value.split(",")
                        }
                    } else {
                        control.allowEditMode = false
                    }
                }
                hiddenModesLabel.calcVisible()
            }

            Connections {
                target: control
                function onEditModeChanged() {
                    if (editMode) {
                        for (var i=0; i<modeRepeater.count; i++) {
                            var button      = modeRepeater.itemAt(i).children[0]
                            var checkBox    = modeRepeater.itemAt(i).children[1]

                            checkBox.checked = !hiddenFlightModesList.find(item => { return item === button.text } )
                        }
                    }
                }
            }

            QGCLabel {
                text:               qsTr("Hold to confirm")
                font.pointSize:     ScreenTools.smallFontPointSize
                Layout.fillWidth:   true
                horizontalAlignment:Text.AlignHCenter
                visible:            flightModeSettings.requireModeChangeConfirmation.rawValue
            }

            Repeater {
                id:     modeRepeater
                model:  activeVehicle ? activeVehicle.flightModes : []

                RowLayout {
                    spacing: ScreenTools.defaultFontPixelWidth
                    visible: editMode || !hiddenFlightModesList.find(item => { return item === modelData } )
                    // 模式按钮
                    QGCDelayButton {
                        id:                 modeButton
                        text:               flightModeDisplayName(modelData)
                        //text:               modelData
                        delay:              flightModeSettings.requireModeChangeConfirmation.rawValue ? 500 : 0
                        Layout.fillWidth:   true

                        onActivated: {
                            if (editMode) {
                                parent.children[1].toggle()
                                parent.children[1].clicked()
                            } else {
                                //var controller = globals.guidedControllerFlyView
                                //controller.confirmAction(controller.actionSetFlightMode, modelData)
                                activeVehicle.flightMode = modelData
                                mainWindow.closeIndicatorDrawer()
                            }
                        }
                    }

                    QGCCheckBoxSlider {
                        visible: editMode

                        onClicked: {
                            hiddenFlightModesList = []
                            for (var i=0; i<modeRepeater.count; i++) {
                                var checkBox = modeRepeater.itemAt(i).children[1]
                                if (!checkBox.checked) {
                                    hiddenFlightModesList.push(modeRepeater.model[i])
                                }
                            }
                            hiddenFlightModesFact.value = hiddenFlightModesList.join(",")
                            hiddenModesLabel.calcVisible()
                        }
                    }
                }
            }

            QGCLabel {
                id:                     hiddenModesLabel
                text:                   qsTr("Some Modes Hidden")
                Layout.fillWidth:       true
                font.pointSize:         ScreenTools.smallFontPointSize
                horizontalAlignment:    Text.AlignHCenter
                visible:                false

                function calcVisible() {
                    hiddenModesLabel.visible = hiddenFlightModesList.length > 0
                }
            }
        }
    }

    Component {
        id: flightModeExpandedComponent

        ColumnLayout {
            Layout.preferredWidth:  ScreenTools.defaultFontPixelWidth * 60
            spacing:                margins / 2

            property var  qgcPal:               QGroundControl.globalPalette
            property real margins:              ScreenTools.defaultFontPixelHeight
            property var  flightModeSettings:   QGroundControl.settingsManager.flightModeSettings

            Loader {
                sourceComponent: expandedPageComponent
            }

            SettingsGroupLayout {
                Layout.fillWidth:  true

                FactCheckBoxSlider {
                    Layout.fillWidth:   true
                    text:               qsTr("Click and Hold to Confirm Mode Change")
                    fact:               flightModeSettings.requireModeChangeConfirmation
                }

                RowLayout {
                    Layout.fillWidth:   true
                    enabled:            control.allowEditMode

                    QGCLabel {
                        Layout.fillWidth:   true
                        text:               qsTr("Edit Displayed Flight Modes")
                    }

                    QGCCheckBoxSlider {
                        onClicked: control.editMode = checked
                    }
                }

                LabelledButton {
                    Layout.fillWidth:   true
                    label:              qsTr("Flight Modes")
                    buttonText:         qsTr("Configure")
                    visible:            _activeVehicle.autopilotPlugin.knownVehicleComponentAvailable(AutoPilotPlugin.KnownFlightModesVehicleComponent) &&
                                            QGroundControl.corePlugin.showAdvancedUI

                    onClicked: {
                        mainWindow.showKnownVehicleComponentConfigPage(AutoPilotPlugin.KnownFlightModesVehicleComponent)
                        mainWindow.closeIndicatorDrawer()
                    }
                }
            }
        }
    }
}
