import QtQuick
import QtQuick.Controls
import QGroundControl
import QGroundControl.Controls

Item {
    id: root

    // 绘制圆形背景
    Canvas {
        id: circleCanvas
        width: Math.max(parent.width, 100)  // 设置最小宽度为 100px
        height: Math.max(parent.height, 100)  // 设置最小高度为 100px
        anchors.fill: parent  // 画布大小与父级控件相同
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)  // 清空画布
            var centerX = width / 2  // 圆心 X 坐标
            var centerY = height / 2  // 圆心 Y 坐标
            // 设置最小圆形半径
            var radius = Math.min(width, height) / 2 // 使用 Math.min 保证半径适应父容器的最小边
            ctx.beginPath()
            ctx.arc(centerX, centerY, radius, 0, 2 * Math.PI)
            ctx.fillStyle = Qt.rgba(0, 0, 0, 0.3)  // 背景色：半透明黑色
            ctx.fill()
        }
    }

    // =======================
    // 上方向
    // =======================
    MouseArea {
        id: upArea
        anchors.centerIn: parent  // 按钮中心锚定到父级
        width: parent.width * 0.4
        height: parent.height * 0.2
        anchors.verticalCenterOffset: -parent.height * 0.35  // 上移，靠近顶部

        hoverEnabled: true  // 启用鼠标悬停效果

        onPressed: {
            console.log("[PTZ] ↑ 上仰开始")
            QGroundControl.androidHCNetSDK.hcNetSDKPTZControlOther(QGroundControl.androidHCNetSDK.userIDCtrl, QGroundControl.androidHCNetSDK.channelCtrl, 21, 0)
        }
        onReleased: {
            console.log("[PTZ] ↑ 上仰停止")
            QGroundControl.androidHCNetSDK.hcNetSDKPTZControlOther(QGroundControl.androidHCNetSDK.userIDCtrl, QGroundControl.androidHCNetSDK.channelCtrl, 21, 1)
        }

        Rectangle {
            anchors.fill: parent  // 按钮背景填满 MouseArea
            radius: width / 2  // 圆形按钮
            color: upArea.pressed ? Qt.rgba(255,255,255,0.6) : Qt.rgba(255,255,255,0.2)  // 按钮按下时变色
        }
    }

    // =======================
    // 下方向
    // =======================
    MouseArea {
        id: downArea
        anchors.centerIn: parent  // 按钮中心锚定到父级
        width: parent.width * 0.4
        height: parent.height * 0.2
        anchors.verticalCenterOffset: parent.height * 0.35  // 下移，靠近底部

        hoverEnabled: true  // 启用鼠标悬停效果

        onPressed: {
            console.log("[PTZ] ↓ 下俯开始")
            QGroundControl.androidHCNetSDK.hcNetSDKPTZControlOther(QGroundControl.androidHCNetSDK.userIDCtrl, QGroundControl.androidHCNetSDK.channelCtrl, 22, 0)
        }
        onReleased: {
            console.log("[PTZ] ↓ 下俯停止")
            QGroundControl.androidHCNetSDK.hcNetSDKPTZControlOther(QGroundControl.androidHCNetSDK.userIDCtrl, QGroundControl.androidHCNetSDK.channelCtrl, 22, 1)
        }

        Rectangle {
            anchors.fill: parent  // 按钮背景填满 MouseArea
            radius: width / 2  // 圆形按钮
            color: downArea.pressed ? Qt.rgba(255,255,255,0.6) : Qt.rgba(255,255,255,0.2)  // 按钮按下时变色
        }
    }

    // =======================
    // 左方向
    // =======================
    MouseArea {
        id: leftArea
        anchors.centerIn: parent  // 按钮中心锚定到父级
        width: parent.width * 0.2
        height: parent.height * 0.4
        anchors.horizontalCenterOffset: -parent.width * 0.35  // 左移，靠近左侧

        hoverEnabled: true  // 启用鼠标悬停效果

        onPressed: {
            console.log("[PTZ] ← 左转开始")
            QGroundControl.androidHCNetSDK.hcNetSDKPTZControlOther(QGroundControl.androidHCNetSDK.userIDCtrl, QGroundControl.androidHCNetSDK.channelCtrl, 23, 0)
        }
        onReleased: {
            console.log("[PTZ] ← 左转停止")
            QGroundControl.androidHCNetSDK.hcNetSDKPTZControlOther(QGroundControl.androidHCNetSDK.userIDCtrl, QGroundControl.androidHCNetSDK.channelCtrl, 23, 1)
        }

        Rectangle {
            anchors.fill: parent  // 按钮背景填满 MouseArea
            radius: width / 2  // 圆形按钮
            color: leftArea.pressed ? Qt.rgba(255,255,255,0.6) : Qt.rgba(255,255,255,0.2)  // 按钮按下时变色
        }
    }

    // =======================
    // 右方向
    // =======================
    MouseArea {
        id: rightArea
        anchors.centerIn: parent  // 按钮中心锚定到父级
        width: parent.width * 0.2
        height: parent.height * 0.4
        anchors.horizontalCenterOffset: parent.width * 0.35  // 右移，靠近右侧

        hoverEnabled: true  // 启用鼠标悬停效果

        onPressed: {
            console.log("[PTZ] → 右转开始")
            QGroundControl.androidHCNetSDK.hcNetSDKPTZControlOther(QGroundControl.androidHCNetSDK.userIDCtrl, QGroundControl.androidHCNetSDK.channelCtrl, 24, 0)
        }
        onReleased: {
            console.log("[PTZ] → 右转停止")
            QGroundControl.androidHCNetSDK.hcNetSDKPTZControlOther(QGroundControl.androidHCNetSDK.userIDCtrl, QGroundControl.androidHCNetSDK.channelCtrl, 24, 1)
        }

        Rectangle {
            anchors.fill: parent  // 按钮背景填满 MouseArea
            radius: width / 2  // 圆形按钮
            color: rightArea.pressed ? Qt.rgba(255,255,255,0.6) : Qt.rgba(255,255,255,0.2)  // 按钮按下时变色
        }
    }

    // =======================
    // 中心按钮（复位）
    // =======================
    MouseArea {
        id: centerButton
        anchors.centerIn: parent  // 按钮中心锚定到父级
        width: Math.min(parent.width * 0.2, parent.height * 0.2)  // 按钮的宽度为背景圆形的 20%
        height: width  // 高度与宽度相同，保持圆形

        onClicked: {
            console.log("[PTZ] 复位云台")
            // 可根据需要定义复位逻辑
        }

        Rectangle {
            anchors.fill: parent  // 按钮背景填满 MouseArea
            radius: width / 2  // 圆形按钮
            color: centerButton.pressed ? Qt.rgba(255,255,255,0.6) : Qt.rgba(255,255,255,0.2)  // 按钮按下时变色
        }
        Text {
            anchors.centerIn: parent
            text: "●"  // 圆形中间显示的符号
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
                    QGroundControl.androidHCNetSDK.hcNetSDKPTZControlOther(QGroundControl.androidHCNetSDK.userIDCtrl,QGroundControl.androidHCNetSDK.channelCtrl, 11, 0)
                }
                onReleased: {
                    console.log("[PTZ] 放大停止")
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
