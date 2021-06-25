/******************************************************************************/
/*!
\file       Logger.h
\author     Darren Lin (100% code contribution)
\copyright  Copyright (C) 2021 DigiPen Institute of Technology. Reproduction
            or disclosure of this file or its contents without the prior
            written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <filesystem>
#include <fstream>
#include <unordered_map>

namespace Logger
{
    enum class ErrorType
    {
        MESSAGES = 0,
        WARNING = 1,
        ERROR = 2
    };

    class FileWriter
    {
    public:
        FileWriter(const char* file);
        ~FileWriter();

        std::filesystem::path getFileName() const;
        void writeLine(const std::string& content);
        const std::string readFile();

    private:
        //std::filesystem::path path;
        std::string _fileName;
        std::fstream _fileHandle;
    };

    class Logger : public FileWriter
    {
    public:
        Logger(const char* logFilename);

        const std::string logCurrentDateTime();
        void readLogs(ErrorType type) const;

        void logError(const std::string& message, ErrorType type);

    private:
        std::unordered_map<ErrorType, std::string> _MapofLogs;
    };
}

#endif
