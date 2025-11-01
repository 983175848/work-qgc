/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import QtLocation
import QtPositioning
import QtQuick.Window
import QtQml.Models

import QGroundControl

import QGroundControl.Controls

import QGroundControl.FlightDisplay
import QGroundControl.FlightMap


import QGroundControl.UTMSP

import QGroundControl.Viewer3D

Item {
    id: _root

    // These should only be used by MainRootWindow
    property var planController:    _planController
    property var guidedController:  _guidedController

    // Properties of UTM adapter
    property bool utmspSendActTrigger: false

    PlanMasterController {
        id:                     _planController
        flyView:                true
        Component.onCompleted:  start()
    }

    property bool   _mainWindowIsMap:       mapControl.pipState.state === mapControl.pipState.fullState
    property bool   _isFullWindowItemDark:  _mainWindowIsMap ? mapControl.isSatelliteMap : true
    property var    _activeVehicle:         QGroundControl.multiVehicleManager.activeVehicle
    property var    _missionController:     _planController.missionController
    property var    _geoFenceController:    _planController.geoFenceController
    property var    _rallyPointController:  _planController.rallyPointController
    property real   _margins:               ScreenTools.defaultFontPixelWidth / 2
    property var    _guidedController:      guidedActionsController
    property var    _guidedValueSlider:     guidedValueSlider
    property var    _widgetLayer:           widgetLayer
    property real   _toolsMargin:           ScreenTools.defaultFontPixelWidth * 0.75
    property rect   _centerViewport:        Qt.rect(0, 0, width, height)
    property real   _rightPanelWidth:       ScreenTools.defaultFontPixelWidth * 30
    property var    _mapControl:            mapControl

    property real   _fullItemZorder:    0
    property real   _pipItemZorder:     QGroundControl.zOrderWidgets

    function _calcCenterViewPort() {
        var newToolInset = Qt.rect(0, 0, width, height)
        toolstrip.adjustToolInset(newToolInset)
    }

    function dropMainStatusIndicatorTool() {
        toolbar.dropMainStatusIndicatorTool();
    }

    QGCToolInsets {
        id:                     _toolInsets
        leftEdgeBottomInset:    _pipView.leftEdgeBottomInset
        bottomEdgeLeftInset:    _pipView.bottomEdgeLeftInset
    }

    FlyViewToolBar {
        id:         toolbar
        visible:    !QGroundControl.videoManager.fullScreen
    }

    Item {
        id:                 mapHolder
        anchors.top:        toolbar.bottom
        anchors.bottom:     parent.bottom
        anchors.left:       parent.left
        anchors.right:      parent.right

        FlyViewMap {
            id:                     mapControl
            planMasterController:   _planController
            rightPanelWidth:        ScreenTools.defaultFontPixelHeight * 9
            pipView:                _pipView
            pipMode:                !_mainWindowIsMap
            toolInsets:             customOverlay.totalToolInsets
            mapName:                "FlightDisplayView"
            enabled:                !viewer3DWindow.isOpen
        }

        FlyViewVideo {
            id:         videoControl
            pipView:    _pipView
        }

        PipView {
            id:                     _pipView
            anchors.left:           parent.left
            anchors.bottom:         parent.bottom
            anchors.margins:        _toolsMargin
            item1IsFullSettingsKey: "MainFlyWindowIsMap"
            item1:                  mapControl
            item2:                  QGroundControl.videoManager.hasVideo ? videoControl : null
            show:                   QGroundControl.videoManager.hasVideo && !QGroundControl.videoManager.fullScreen &&
                                        (videoControl.pipState.state === videoControl.pipState.pipState || mapControl.pipState.state === mapControl.pipState.pipState)
            z:                      QGroundControl.zOrderWidgets

            property real leftEdgeBottomInset: visible ? width + anchors.margins : 0
            property real bottomEdgeLeftInset: visible ? height + anchors.margins : 0
        }

        FlyViewWidgetLayer {
            id:                     widgetLayer
            anchors.top:            parent.top
            anchors.bottom:         parent.bottom
            anchors.left:           parent.left
            anchors.right:          guidedValueSlider.visible ? guidedValueSlider.left : parent.right
            z:                      _fullItemZorder + 2 // we need to add one extra layer for map 3d viewer (normally was 1)
            parentToolInsets:       _toolInsets
            mapControl:             _mapControl
            visible:                !QGroundControl.videoManager.fullScreen
            utmspActTrigger:        utmspSendActTrigger
            isViewer3DOpen:         viewer3DWindow.isOpen
            // 云台控制
            Item {
                id: gimbalControl
                width: 140
                height: 140
                anchors {
                    right: parent.right
                    bottom: parent.bottom
                    bottomMargin: 100
                    rightMargin: 24
                }
                z: 999
                visible: true

                // 绘制圆形背景
                Canvas {
                    id: circleCanvas
                    anchors.fill: parent
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)
                        var centerX = width / 2
                        var centerY = height / 2
                        var radius = width / 2

                        // 半透明圆形背景
                        ctx.beginPath()
                        ctx.arc(centerX, centerY, radius, 0, 2 * Math.PI)
                        ctx.fillStyle = Qt.rgba(0, 0, 0, 0.3)
                        ctx.fill()
                    }
                }

                // =======================
                // 上方向
                // =======================
                MouseArea {
                    id: upArea
                    anchors.centerIn: parent
                    width: parent.width * 0.4
                    height: parent.height * 0.2
                    anchors.verticalCenterOffset: -parent.height * 0.35
                    hoverEnabled: true

                    onPressed: {
                        console.log("[PTZ] ↑ 上仰开始")
                        QGroundControl.androidHCNetSDK.hcNetSDKPTZControlOther(QGroundControl.androidHCNetSDK.userIDCtrl,QGroundControl.androidHCNetSDK.channelCtrl, 21, 0)
                    }
                    onReleased: {
                        console.log("[PTZ] ↑ 上仰停止")
                        QGroundControl.androidHCNetSDK.hcNetSDKPTZControlOther(QGroundControl.androidHCNetSDK.userIDCtrl,QGroundControl.androidHCNetSDK.channelCtrl, 21, 1)
                    }

                    Rectangle {
                        anchors.fill: parent
                        radius: width / 2
                        color: upArea.pressed ? Qt.rgba(255,255,255,0.6) : Qt.rgba(255,255,255,0.2)
                    }
                }

                // =======================
                // 下方向
                // =======================
                MouseArea {
                    id: downArea
                    anchors.centerIn: parent
                    width: parent.width * 0.4
                    height: parent.height * 0.2
                    anchors.verticalCenterOffset: parent.height * 0.35
                    hoverEnabled: true

                    onPressed: {
                        console.log("[PTZ] ↓ 下俯开始")
                        QGroundControl.androidHCNetSDK.hcNetSDKPTZControlOther(QGroundControl.androidHCNetSDK.userIDCtrl,QGroundControl.androidHCNetSDK.channelCtrl, 22, 0)
                    }
                    onReleased: {
                        console.log("[PTZ] ↓ 下俯停止")
                        QGroundControl.androidHCNetSDK.hcNetSDKPTZControlOther(QGroundControl.androidHCNetSDK.userIDCtrl,QGroundControl.androidHCNetSDK.channelCtrl, 22, 1)
                    }

                    Rectangle {
                        anchors.fill: parent
                        radius: width / 2
                        color: downArea.pressed ? Qt.rgba(255,255,255,0.6) : Qt.rgba(255,255,255,0.2)
                    }
                }

                // =======================
                // 左方向
                // =======================
                MouseArea {
                    id: leftArea
                    anchors.centerIn: parent
                    width: parent.width * 0.2
                    height: parent.height * 0.4
                    anchors.horizontalCenterOffset: -parent.width * 0.35
                    hoverEnabled: true

                    onPressed: {
                        console.log("[PTZ] ← 左转开始")
                        QGroundControl.androidHCNetSDK.hcNetSDKPTZControlOther(QGroundControl.androidHCNetSDK.userIDCtrl,QGroundControl.androidHCNetSDK.channelCtrl, 23, 0)
                    }
                    onReleased: {
                        console.log("[PTZ] ← 左转停止")
                        QGroundControl.androidHCNetSDK.hcNetSDKPTZControlOther(QGroundControl.androidHCNetSDK.userIDCtrl,QGroundControl.androidHCNetSDK.channelCtrl, 23, 1)
                    }

                    Rectangle {
                        anchors.fill: parent
                        radius: width / 2
                        color: leftArea.pressed ? Qt.rgba(255,255,255,0.6) : Qt.rgba(255,255,255,0.2)
                    }
                }

                // =======================
                // 右方向
                // =======================
                MouseArea {
                    id: rightArea
                    anchors.centerIn: parent
                    width: parent.width * 0.2
                    height: parent.height * 0.4
                    anchors.horizontalCenterOffset: parent.width * 0.35
                    hoverEnabled: true

                    onPressed: {
                        console.log("[PTZ] → 右转开始")
                        QGroundControl.androidHCNetSDK.hcNetSDKPTZControlOther(QGroundControl.androidHCNetSDK.userIDCtrl,QGroundControl.androidHCNetSDK.channelCtrl, 24, 0)
                    }
                    onReleased: {
                        console.log("[PTZ] → 右转停止")
                        QGroundControl.androidHCNetSDK.hcNetSDKPTZControlOther(QGroundControl.androidHCNetSDK.userIDCtrl,QGroundControl.androidHCNetSDK.channelCtrl, 24, 1)
                    }

                    Rectangle {
                        anchors.fill: parent
                        radius: width / 2
                        color: rightArea.pressed ? Qt.rgba(255,255,255,0.6) : Qt.rgba(255,255,255,0.2)
                    }
                }

                // =======================
                // 中心按钮（复位）
                // =======================
                MouseArea {
                    id: centerButton
                    anchors.centerIn: parent
                    width: parent.width * 0.2
                    height: parent.height * 0.2

                    onClicked: {
                        console.log("[PTZ] 复位云台")
                        // 可根据需要定义复位逻辑
                    }

                    Rectangle {
                        anchors.fill: parent
                        radius: width / 2
                        color: centerButton.pressed ? Qt.rgba(255,255,255,0.6) : Qt.rgba(255,255,255,0.2)
                    }
                    Text {
                        anchors.centerIn: parent
                        text: "●"
                        color: "white"
                        font.pixelSize: 20
                    }
                }

                // =======================
                // 放大 / 缩小按钮
                // =======================
                Column {
                    anchors {
                        left: parent.left
                        verticalCenter: parent.verticalCenter
                        leftMargin: -60
                    }
                    spacing: 8

                    // ---------- 放大按钮 ----------
                    Rectangle {
                        id: zoomInButton
                        width: 44
                        height: 44
                        radius: 22
                        color: zoomInArea.pressed ? Qt.rgba(255,255,255,0.25) : Qt.rgba(0,0,0,0.3)
                        border.color: "white"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "+"
                            color: "white"
                            font.pixelSize: 22
                        }

                        MouseArea {
                            id: zoomInArea
                            anchors.fill: parent
                            onPressed: {
                                console.log("[PTZ] 放大开始")
                                 console.log("userIDCtrl,channelCtr", QGroundControl.androidHCNetSDK.userIDCtrl, QGroundControl.androidHCNetSDK.channelCtrl)
                                QGroundControl.androidHCNetSDK.hcNetSDKPTZControlOther(QGroundControl.androidHCNetSDK.userIDCtrl,QGroundControl.androidHCNetSDK.channelCtrl, 11, 0)
                            }
                            onReleased: {
                                console.log("[PTZ] 放大停止")
                                console.log("userIDCtrl,channelCtr", QGroundControl.androidHCNetSDK.userIDCtrl, QGroundControl.androidHCNetSDK.channelCtrl)

                                QGroundControl.androidHCNetSDK.hcNetSDKPTZControlOther(QGroundControl.androidHCNetSDK.userIDCtrl,QGroundControl.androidHCNetSDK.channelCtrl, 11, 1)
                            }
                        }
                    }

                    // ---------- 缩小按钮 ----------
                    Rectangle {
                        id: zoomOutButton
                        width: 44
                        height: 44
                        radius: 22
                        color: zoomOutArea.pressed ? Qt.rgba(255,255,255,0.25) : Qt.rgba(0,0,0,0.3)
                        border.color: "white"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "-"
                            color: "white"
                            font.pixelSize: 22
                        }

                        MouseArea {
                            id: zoomOutArea
                            anchors.fill: parent
                            onPressed: {
                                console.log("[PTZ] 缩小开始")
                                QGroundControl.androidHCNetSDK.hcNetSDKPTZControlOther(QGroundControl.androidHCNetSDK.userIDCtrl,QGroundControl.androidHCNetSDK.channelCtrl, 12, 0)
                            }
                            onReleased: {
                                console.log("[PTZ] 缩小停止")
                                QGroundControl.androidHCNetSDK.hcNetSDKPTZControlOther(QGroundControl.androidHCNetSDK.userIDCtrl,QGroundControl.androidHCNetSDK.channelCtrl, 12, 1)
                            }
                        }
                    }
                }
            }

        }

        FlyViewCustomLayer {
            id:                 customOverlay
            anchors.fill:       widgetLayer
            z:                  _fullItemZorder + 2
            parentToolInsets:   widgetLayer.totalToolInsets
            mapControl:         _mapControl
            visible:            !QGroundControl.videoManager.fullScreen
        }

        // Development tool for visualizing the insets for a paticular layer, show if needed
        FlyViewInsetViewer {
            id:                     widgetLayerInsetViewer
            anchors.top:            parent.top
            anchors.bottom:         parent.bottom
            anchors.left:           parent.left
            anchors.right:          guidedValueSlider.visible ? guidedValueSlider.left : parent.right
            z:                      widgetLayer.z + 1
            insetsToView:           widgetLayer.totalToolInsets
            visible:                false
        }

        GuidedActionsController {
            id:                 guidedActionsController
            missionController:  _missionController
            guidedValueSlider:     _guidedValueSlider
        }

        //-- Guided value slider (e.g. altitude)
        GuidedValueSlider {
            id:                 guidedValueSlider
            anchors.right:      parent.right
            anchors.top:        parent.top
            anchors.bottom:     parent.bottom
            z:                  QGroundControl.zOrderTopMost
            visible:            false
        }

        Viewer3D {
            id: viewer3DWindow
            anchors.fill: parent
        }
    }

    UTMSPActivationStatusBar {
        activationStartTimestamp:   UTMSPStateStorage.startTimeStamp
        activationApproval:         UTMSPStateStorage.showActivationTab && QGroundControl.utmspManager.utmspVehicle.vehicleActivation
        flightID:                   UTMSPStateStorage.flightID
        anchors.fill:               parent

        function onActivationTriggered(value) {
            _root.utmspSendActTrigger = value
        }
    }
}
