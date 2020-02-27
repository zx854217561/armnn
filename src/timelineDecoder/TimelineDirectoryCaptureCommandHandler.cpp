//
// Copyright © 2019 Arm Ltd. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "TimelineDirectoryCaptureCommandHandler.hpp"
#include "TimelineCaptureCommandHandler.hpp"

#include <iostream>
#include <string>

using namespace armnn::profiling;

namespace armnn
{

namespace timelinedecoder
{

void TimelineDirectoryCaptureCommandHandler::ParseData(const armnn::profiling::Packet& packet)
{
    uint32_t offset = 0;

    if (packet.GetLength() < 8)
    {
        return;
    }

    const unsigned char* data = packet.GetData();

    m_SwTraceHeader.m_StreamVersion = ReadUint8(data, offset);
    offset += uint8_t_size;
    m_SwTraceHeader.m_PointerBytes = ReadUint8(data, offset);
    offset += uint8_t_size;
    m_SwTraceHeader.m_ThreadIdBytes = ReadUint8(data, offset);
    offset += uint8_t_size;

    uint32_t numberOfDeclarations = profiling::ReadUint32(data, offset);
    offset += uint32_t_size;

    for (uint32_t declaration = 0; declaration < numberOfDeclarations; ++declaration)
    {
        m_SwTraceMessages.push_back(profiling::ReadSwTraceMessage(data, offset));
    }

    m_TimelineCaptureCommandHandler.SetThreadIdSize(m_SwTraceHeader.m_ThreadIdBytes);
}

void TimelineDirectoryCaptureCommandHandler::Print()
{
    std::string header;

    header.append(profiling::CentreAlignFormatting("decl_id", 12));
    header.append(" | ");
    header.append(profiling::CentreAlignFormatting("decl_name", 20));
    header.append(" | ");
    header.append(profiling::CentreAlignFormatting("ui_name", 20));
    header.append(" | ");
    header.append(profiling::CentreAlignFormatting("arg_types", 16));
    header.append(" | ");
    header.append(profiling::CentreAlignFormatting("arg_names", 80));
    header.append("\n");

    std::cout << "\n" << "\n";
    std::cout << profiling::CentreAlignFormatting("SW DIRECTORY", static_cast<int>(header.size()));
    std::cout << "\n";
    std::cout << std::string(header.size(), '=') << "\n";

    std::cout << header;

    for (const auto& swTraceMessage : m_SwTraceMessages)
    {
        std::string body;

        body.append(profiling::CentreAlignFormatting(std::to_string(swTraceMessage.m_Id), 12));
        body.append(" | ");
        body.append(profiling::CentreAlignFormatting(swTraceMessage.m_Name, 20));
        body.append(" | ");
        body.append(profiling::CentreAlignFormatting(swTraceMessage.m_UiName, 20));
        body.append(" | ");

        std::string argTypes;
        for (auto argType: swTraceMessage.m_ArgTypes)
        {
            argTypes += argType;
            argTypes += " ";
        }
        body.append(profiling::CentreAlignFormatting(argTypes, 16));
        body.append(" | ");

        std::string argNames;
        for (auto argName: swTraceMessage.m_ArgNames)
        {
            argNames += argName + " ";
        }
        body.append(profiling::CentreAlignFormatting(argNames, 80));

        body.append("\n");

        std::cout << std::string(body.size(), '-') << "\n";

        std::cout << body;
    }
}

void TimelineDirectoryCaptureCommandHandler::operator()(const profiling::Packet& packet)
{
    ParseData(packet);

    if (!m_QuietOperation)
    {
        Print();
    }
}

} //namespace gatordmock

} //namespace armnn
