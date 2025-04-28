#include "JZModuleCommEditor.h"
#include "JZModuleComm.h"
#include "JZEditorGlobal.h"

void JZModuleCommEditorInit()
{
    auto inst = editorManager()->instance();

    inst->registLogicNode(Node_CommInit,"通信");
    inst->registLogicNode(Node_ModbusRead,"通信");
    inst->registLogicNode(Node_ModbusWrite,"通信");
    inst->registLogicNode(Node_TcpClientRead,"通信");
    inst->registLogicNode(Node_TcpClientWrite,"通信");
    inst->registLogicNode(Node_UdpRead,"通信");
    inst->registLogicNode(Node_UdpWrite,"通信");
    inst->registLogicNode(Node_SerialRead,"通信");
    inst->registLogicNode(Node_SerialWrite,"通信");
}