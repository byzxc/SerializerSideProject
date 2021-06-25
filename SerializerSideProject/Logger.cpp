/******************************************************************************/
/*!
\file       Logger.cpp
\author     Darren Lin (100% code contribution)
\copyright  Copyright (C) 2021 DigiPen Institute of Technology. Reproduction
            or disclosure of this file or its contents without the prior
            written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#ifndef _LOGGER_HPP_
#define _LOGGER_HPP_

#include "Logger.h"
#include <iostream>

namespace Logger
{
    //Discard everything before reading & writing to the file
    FileWriter::FileWriter(const char* file) :
        _fileName{ file }, _fileHandle{ _fileName, std::ios_base::trunc | std::ios_base::out }
    {
    }

    FileWriter::~FileWriter()
    {
        //_fileHandle.close();
    }

    std::filesystem::path FileWriter::getFileName() const
    {
        return _fileName;
    }

    void FileWriter::writeLine(const std::string& content)
    {
        //std::fstream file{ _fileName, std::ios_base::out };
        if (_fileHandle.is_open())
        {
            //_fileHandle.write(content.c_str(), content.size());
            _fileHandle << content.c_str();
            _fileHandle.close();
        }
    }

    const std::string FileWriter::readFile()
    {
        std::stringstream ss;
        if (_fileHandle.is_open())
        {
            //Copy everything from file to ss
            std::copy(std::istreambuf_iterator<char>(_fileHandle), std::istreambuf_iterator<char>(), std::ostreambuf_iterator<char>(ss));
        }
        else
        {
            _fileHandle.open(_fileName, std::ios_base::in);
            std::copy(std::istreambuf_iterator<char>(_fileHandle), std::istreambuf_iterator<char>(), std::ostreambuf_iterator<char>(ss));
            _fileHandle.close();
        }
        return ss.str();
    }

    Logger::Logger(const char* logFilename) : FileWriter(logFilename)
    {
    }

    const std::string Logger::logCurrentDateTime()
    {
        std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
        time_t time = std::chrono::system_clock::to_time_t(tp);
        char timeBuffer[40];
        ctime_s(timeBuffer, sizeof(timeBuffer), &time);
        return timeBuffer;
    };

    void Logger::readLogs(ErrorType type) const
    {
        //Only shows the most recent
        std::cout << _MapofLogs.at(type) << std::endl;
    }

    void Logger::logError(const std::string& message, ErrorType type)
    {
        std::string finalMsg;

        switch (type)
        {
        case ErrorType::MESSAGES:
            finalMsg += "[Message] ";
            break;
        case ErrorType::WARNING:
            finalMsg += "[Warning] ";
            break;
        case ErrorType::ERROR:
            finalMsg += "[Error] ";
            break;
        default:
            finalMsg += "[Undefined ErrorType] ";
            break;
        }

        finalMsg += message + " on " + logCurrentDateTime() + "\n";
        writeLine(finalMsg);
        _MapofLogs.insert(std::pair<ErrorType, std::string>(type, message));
    }
}

#endif