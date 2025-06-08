#include <QMap>
#include "JZGCodeParser.h"

//JZGCodeParser
JZGCodeParser::JZGCodeParser()
{

}

JZGCodeParser::~JZGCodeParser()
{
}

bool JZGCodeParser::parse(const QString &gcode, JZGCodeProgram & program)
{
    QMap<QString, JZGCodeInstance::Type> commandMap;
    commandMap["G0"] = JZGCodeInstance::G0;
    commandMap["G1"] = JZGCodeInstance::G1;
    commandMap["G2"] = JZGCodeInstance::G2;
    commandMap["G3"] = JZGCodeInstance::G3;
    commandMap["M4"] = JZGCodeInstance::M4;
    commandMap["M5"] = JZGCodeInstance::M5;

    auto parstInstParam = [](JZGCodeInstance& inst, QString param)->bool
    {
        QString upperTypeam = param[0];
        param = param.mid(1);

        bool ok;
        double value = param.toDouble(&ok);
        if (!ok)
            return false;

        // 根据参数类型设置值（直接访问JZGCodeInstance的公共成员变量）
        if (upperTypeam == "X") {
            inst.X = value;
        }
        else if (upperTypeam == "Y") {
            inst.Y = value;
        }
        else if (upperTypeam == "Z") {
            inst.Z = value;
        }
        else if (upperTypeam == "I") {
            inst.I = value;
        }
        else if (upperTypeam == "J") {
            inst.J = value;
        }
        else if (upperTypeam == "K") {
            inst.K = value;
        }
        else if (upperTypeam == "R") {
            inst.R = value;
        }
        else if (upperTypeam == "F") {
            inst.F = value;
        }
        else if (upperTypeam == "S") {
            inst.S = value;
        }
        else {
            // 未知参数类型
            return false;
        }

        return true;
    };

    QStringList lines = gcode.split("\n");
    for (const QString &line : lines) 
    {
        QString trimmedLine = line.simplified().trimmed();
        if (trimmedLine.isEmpty() || trimmedLine.startsWith(';')) {
            continue;
        }

        QStringList parts = trimmedLine.split(" ");
        QString command = parts[0];
        if (command.size() < 2)
            return false;

        if (command[1] == '0' && command.size() > 2)
            command.removeAt(1);

        if (!commandMap.contains(command))
            return false;
        
        JZGCodeInstance inst;
        inst.type = commandMap[command];
        for (int i = 1; i < parts.size(); i++)
        {
            if (!parstInstParam(inst,parts[i]))
                return false;
        }

        program.instList.append(inst);
    }
    return true;
}