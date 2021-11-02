/*@file log.h
@brief This class is log module
@author sylar.yin
@date 2021/8/23
*/
#ifndef __SYLAR_LOG_H__
#define __SYLAR_LOG_H__

#include<sstream>
#include<string.h>
#include<stdint.h>
#include<memory>
#include<list>
#include<fstream>
#include<vector>
#include<stdarg.h>
#include<map>
/*#include"util.h"
#include"singleton.h"
#include"thread.h"*/

//The following definitions consider using inline functions instead

//Log level logs are written to logger in stream mode
#define SYLaR_LOG_LEVEL(logger , level) \
    if(logger->getLevel() <= level) \
        sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger, level, \
                __FILE__, __LINE__, 0, sylar::GetThreadId(),\
        sylar::GetFiberId(), time(0), sylar::Thread::GetName()))).getSS()

//Log debug logs are written to logger in stream mode
#define SYLAR_LOG_DEBUG(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::DEBUG)


//Log info logs are written to logger in stream mode
#define SYLAR_LOG_INFO(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::INFO)

//Log warn logs are written to logger in stream mode
#define SYLAR_LOG_WARN(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::WARN)

//Log error logs are written to logger in stream mode
#define SYLAR_LOG_ERROR(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::ERROR)

//Log fatal logs are written to logger in stream mode
#define SYLAR_LOG_FATAL(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::FATAL)


//Write log level logs to logger using formatting
#define SYLAR_LOG_FMT_LEVEL(logger, level, fmt, ...) \
    if(logger->getLevel() <= level) \
        sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger, level, \
                        __FILE__, __LINE__, 0, sylar::GetThreadId(),\
                sylar::GetFiberId(), time(0), sylar::Thread::GetName()))).getEvent()->format(fmt, __VA_ARGS__)

//Write log debug logs to logger using formatting
#define SYLAR_LOG_FMT_DEBUG(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::DEBUG, fmt, __VA_ARGS__)

//Write log info logs to logger using formatting
#define SYLAR_LOG_FMT_INFO(logger, fmt, ...)  SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::INFO, fmt, __VA_ARGS__)

//Write log warn logs to logger using formatting
#define SYLAR_LOG_FMT_WARN(logger, fmt, ...)  SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::WARN, fmt, __VA_ARGS__)

//Write log error logs to logger using formatting
#define SYLAR_LOG_FMT_ERROR(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::ERROR, fmt, __VA_ARGS__)

//Write log fatal logs to logger using formatting
#define SYLAR_LOG_FMT_FATAL(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::FATAL, fmt, __VA_ARGS__)

//Get the primary logger
#define SYLAR_LOG_ROOT() sylar::LoggerMgr::GetInstance()->getRoot()

//The logger that gets the name
#define SYLAR_LOG_NAME(name) sylar::LoggerMgr::GetInstance()->getLogger(name)

namespace sylar {



class logger;
class LoggerManager;
// log level
class LogLevel{
public:

    enum Level{
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3, //warning
        ERROR = 4,
        FATAL = 5
    };

    //convert log level to text output
    static const  char* ToString(LogLevel::Level level);

    //convert text to log level.
    //str,log level txt
    static LogLevel::Level FromString(const std::string &str);

};


//log  class日志类
class Logger : public std::enable_shared_from_this<Logger>{
friend class LoggerManager;
public:
    typedef std::shared_ptr<Logger>  ptr;
    typedef Spinlock MutexType;  

    /*
        constructor function
        @param name,logger name
    */
    Logger(const std::string &name="root");

    //destructor
    virtual ~Logger(){}
    
    
    //Write log
    //param event,log event.
    void log(LogLevel::Level level, const LogEvent &event);

    //Write debug level log.
    void debug(LogEvent::ptr event);

    //Write info level log .
    void info(LogEvent::ptr event);

    //Write warning level log.
    void warn(LogEvent::ptr event);

    //Write error level logs.
    void error(LogEvent::ptr event);

    //Write fatal level log .
    void fatal(LogEvent::ptr event);

    //Adding a log target.
    void addAppender(LogAppender::ptr appender);

    //Delete log target
    //@param appender,log target.
    void delAppender(LogAppender::ptr appender);

    //Clean log target
    void clearAppenders();

    //Return log level
    LogLevel::Level getLevel()const {
        return m_level;
    }


    //set log level
    void setLevel(LogLevel::Level val){
        m_level = val;
    }


    //Return log name 
    const std::string getName() const{
        return m_name;
    }

    //set formatter of log
    void setFormatter(LogFormatter::ptr val);

    //set the log format template
    void setFormatter(const std::string& val);

    //get log formatter
    LogFormatter::ptr getFormatter();

    //Convert the logger configuration to YAML String
    std::string toYamlString();
private:
    std::string m_name;//log name 
    LogLevel::Level m_level; //log level
    std::list<LogAppender::ptr>m_appenders;   //Appender gather

    //log formatter
    LogFormatter::ptr m_formatter;

    //主日志器
    Logger::ptr m_root;
};

//Output to the console appender
class StadoutLogAppender  : public LogAppender{
public:
    typedef std::shared_ptr<StadoutLogAppender>  ptr;
    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event);
    std::string toYamlString() override;
};

//Output to the file's Appender
class FileLogAppender : public LogAppender{
public:
    typedef std::shared_ptr<FileLogAppender>  ptr;
    FileLogAppender(const std::string& filename);
    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event);
    std::string toYamlString() override;

    /*Re-open the log file,Return ture on success*/
    bool reopen();

private:
    //The file path
    std::string m_filename;

    //file stream
    std::ofstream m_filestream;

    //The time since the last file was reopened
    uint64_t m_lastTime = 0;
};

//logger mannager
class LoggerManager{
public:
    typedef Spinlock MutexType;

    //constructor function
    LoggerManager();

    //get logger,name is logger name
    Logger::ptr getLogger(const std::string& name);

    //initialize
    void init();

    //returns the primary logger
    Logger::ptr getRoot()const {
        return m_root;
    }

    std::string toYamlString();

private:
    //Mutex
    MutexType m_mutex;

    //logger container 
    std::map<std::string, Logger::ptr > m_loggers;

    //primary logger
    Logger::ptr m_root;
};

//Logger management class singleton mode
typedef sylar::Singleton<LoggerManager>  LoggerMgr;

//log output日志输出类
class LogAppender{
friend class Logger;
public:
    typedef std::shared_ptr<LogAppender>  ptr;
    typedef Spinlock MutexType;


    LogAppender();

    //Destructor function,empty  implementation 
    virtual ~LogAppender(){}

    /*
    @brief writting log
    @param logger,Record log
    @param level,log level
    @param event,log event
    */
   
    void log(LogLevel::Level level, LogEvent::ptr event); 

    //The configuration of the log output target is converted to YamlString
    virtual std::string toYamlString() = 0;

    //Change the log format function 
    void setFormatter(LogFormatter::ptr val);

    //Get the log format function
    LogFormatter::ptr getFormatter();

    //set log level
    LogLevel::Level getLevel() const {
        return m_level;
    }

    void setLevel(LogLevel::Level val){
        m_level = val;
    }
protected:
    LogLevel::Level m_level = LogLevel::DEBUG;

    //Whether you have your own format configurator
    bool m_hasFormatter = false;
    
    //Mutex
    MutexType m_mutex;

    //log format configuration
    LogFormatter::ptr m_formatter;
};

//Log format
class LogFormatter{
public:
   
    typedef std::shared_ptr<LogFormatter>  ptr;
     /*Constructor function
    %m massege
    %m 
     *  %p Log level
     *  %r Elapsed time, in milliseconds
     *  %c Log name
     *  %t Thread Id
     *  %n \n
     *  %d Time
     *  %f File name 
     *  %l Line number
     *  %T Tabs
     *  %F Coroutine id
     *  %N Thread name 
    
    
    */
    LogFormatter(const std::string &pattern);
    virtual ~LogFormatter();

    /*
    Return formatted log text.
    @param logger,Record log
    @param level,log level
    @param event,log event
    */
    std::string format(std::shared_ptr<sylar::Logger> logger,LogLevel::Level level,LogEvent::ptr event);
    std::ostream format(std::ostream &ofs, std::shared_ptr<Logger> logger, LogLevel::Level level,LogEvent::ptr event);

    //Initialize and parse the log template
    void init();

    //Determine if there is an error
    bool isError()const {
        return m_error;
    }

    //Return log template
    const std::string getPattern() const {
        return m_pattern;
    }
private:
    //Log format Template
    std::string m_pattern;

    //Format of the log after parsing
    std::vector<FormatItem::ptr> m_items;

    //Determine if there is an error
    bool m_error = false;

};

//在原文件里，这个类是在public 方式的LogFormatter类里的In the original file, this class is in the public LogFormatter class
class FormatItem{
public:
    typedef std::shared_ptr<FormatItem> ptr;

    virtual ~FormatItem();

    /*
        format log to the stream
    */
    virtual void format(std::ostream &os, std::shared_ptr<Logger> Logger, LogLevel::Level level, LogEvent::ptr event) = 0;


};
/*log event 日志事件
*   Processes can have multipes threads ,and threads can have multipes coroutines.
*   m_elapse : Record the time elapsed since the program was started, in ms.
*/
class LogEvent{
public:
    //destructor Function
    ~LogEvent();

    /* Constructor function 
    @param logger,record log
    @param level,log level
    @param file,file name
    @param line,line number
    @param elapse,The time it takes for the program to start dependencies
    @param thread_id, thread if
    @param fiber id ,coroutine id 
    @param time, log event,in seconds
    @param thread_name,thread_name 
    */
    LogEvent(std::shared_ptr<Logger> Logger,LogLevel::Level level
    , const char* file, int32_t line, uint32_t elapse
    , uint32_t thread_id, uint32_t fiber_id, uint64_t time
    , const std::string &thread_name);

    typedef std::shared_ptr<LogEvent> ptr;

    //Return files name
    const char* getFile() const {
        return m_file;
    }

    //Return line number
    int32_t getline() const {
        return m_line;
    }

    //Return time comsuming
    uint32_t getElapse() const {
        return m_elapse;
    }

    //Return thread id
    uint32_t getThreadId() const {
        return m_threadId;
    }
    //Return coroutine id
    uint32_t getFiberId() const{
        return m_fiberId;
    }

    //Return current time 
    uint64_t getTime() const {
        return m_time;
    }
    //Return thread name
    const std::string& getThreadName() const {
        return m_threadName;
    }

    //Return log text content
    std::string getContent() const{
        return m_ss.str();
    }
    
    //Return log level
    std::shared_ptr<Logger> getLogger() const {
        return m_logger;
    }

    //Return log level
    LogLevel::Level getLevel()const {
        return m_level;
    }

    //Return string stream of log content
    std::stringstream& getSS(){
        return m_ss;
    }

    //Format what is written to the log
    void format(const char* fmt,...);

    //Format what is written to the log
    void format(const char* fmt, va_list al);
private:
    const char* m_file = nullptr; //Files name
    int32_t m_line = 0;     //Files line number

    int32_t m_threadId = 0; //Thread ID
    uint32_t  m_fiberId = 0; //Coroutine ID
    std::string m_content;        

    uint64_t m_time = 0;    //Time Stamp
    uint32_t m_elapse; 
    std::string m_threadName; // Thread name

    std::stringstream m_ss; //log content stream 
    std::shared_ptr<Logger> m_logger; //Log recorder
    LogLevel::Level m_level;  //Log level of feedback
};

//log event wrapper
class LogEventWrap{
public:
    //Destructor function
    ~LogEventWrap();

    /*Constructor function 
    param e is Logevent
    */
    LogEventWrap(LogEvent::ptr e);

    //Get log event
    LogEvent::ptr getEvent() const {
        return m_event;
    }

    //get log content stream
    std::stringstream& getSS();

private:
    LogEvent::ptr m_event;
};

class StoutLogAppender : public LogAppender{
public:
    typedef std::shared_ptr<StadoutLogAppender>  ptr;
    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event);
    std::string toYamlString();//可加override
};
}
#endif
