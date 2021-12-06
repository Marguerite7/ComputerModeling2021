#include "Logger.h"
#include "Utils.h"

#include <fstream>
#include <string>
#include <cstdlib>
#include <stdio.h>
#include <iomanip>
#include <climits>
#include <mutex>

/**
 *  This file is the cpp file for the Logger class.
 *  @file Logger.cpp
 *  @brief cpp file for Engine-Logger
 *  @author Seonghyeon Park
 *  @date 2020-03-31
 */


/**
 * @fn Logger::Logger()
 * @brief the function of basic constructor of Logger
 * @author Seonghyeon Park
 * @date 2020-04-01
 * @details 
 *  - None
 * @param none
 * @return none
 * @bug none
 * @warning none
 * @todo none
 */
Logger::Logger()
{

}

/**
 * @fn Logger::~Logger()
 * @brief the function of basic destructor of Logger
 * @author Seonghyeon Park
 * @date 2020-04-01
 * @details 
 *  - None
 * @param none
 * @return none
 * @bug none
 * @warning none
 * @todo none
 */
Logger::~Logger()
{

}

/**
 * @fn void start_logging()
 * @brief this function starts the logging of simulation events
 * @author Seonghyeon Park
 * @date 2020-04-01
 * @details 
 *  - None
 * @param none
 * @return none
 * @bug none
 * @warning none
 * @todo none
 */

void Logger::set_schedule_log_info(std::vector<std::shared_ptr<Task>>& task_vector)
{
    std::ofstream scheduling_log;
    scheduling_log.open(utils::cpsim_path + "/Log/scheduling.log", std::ios::out);     
    std::string contents = "";
    for(int idx = 0; idx < task_vector.size(); idx++)
    {
        contents += "ECU" + std::to_string(task_vector.at(idx)->get_ECU()->get_ECU_id())+ ": " + task_vector.at(idx)->get_task_name();
        if(idx == task_vector.size() - 1)
            contents += "\n";
        else
        {
            contents += ", ";
        }
    }
    scheduling_log.write(contents.c_str(), contents.size());
    scheduling_log.close();
}

void Logger::start_logging()
{
    std::ofstream scheduling_log;
    while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - utils::simulator_start_time).count()  < utils::simulation_termination_time)
    {
        scheduling_log.open(utils::cpsim_path + "/Log/scheduling.log", std::ios::app);    
        utils::mtx_data_log.lock();
        if(global_object::schedule_data.size() > 10)
        {
            int min_idx = 0;
            std::shared_ptr<ScheduleData> current_data = global_object::schedule_data.front();
            for (int idx = 0; idx < global_object::schedule_data.size(); idx ++)
            {
                if(current_data->get_time() > global_object::schedule_data.at(idx)->get_time())
                {
                    current_data = global_object::schedule_data.at(idx);
                    min_idx = idx;
                }
            }
            
            global_object::schedule_data.erase(global_object::schedule_data.begin() + min_idx);
            scheduling_log.write(current_data->get_data().c_str(), current_data->get_data().size());
        }
        scheduling_log.close();
        utils::mtx_data_log.unlock();    
    }    
}

std::string Logger::_2019_13914_print_tagged_data_log(std::string task_name, std::shared_ptr<TaggedData> current_data, int size){
    std::stringstream data;
    data << std::hex << "0x" << current_data -> data_read1 << " ";
    data << std::hex << "0x" << current_data -> data_read2 << " ";
    data << std::hex << "0x" << current_data -> data_read3 << " ";
    data << std::hex << "0x" << current_data -> data_read4 << " ";
    data << std::hex << "0x" << current_data -> data_read5 << " ";
    data << std::hex << "0x" << current_data -> data_read6 << "\n";
    
    std::stringstream log;
    log << " ";
    log << std::left << std::setw(14) << task_name;
    log << std::left << std::setw(9) << std::to_string(current_data -> data_time);
    log << std::left << std::setw(15) << "READ";
    log << std::left << std::setw(16) << std::to_string(size);

    return log.str() + data.str();
}

std::string Logger::_2019_13914_print_delayed_data_log(std::string task_name, std::shared_ptr<DelayedData> delayed_data, int size){
    std::stringstream data;
    data << std::hex << "0x" << delayed_data -> data_write1 << " ";
    data << std::hex << "0x" << delayed_data -> data_write2 << " ";
    data << std::hex << "0x" << delayed_data -> data_write3 << " ";
    data << std::hex << "0x" << delayed_data -> data_write4 << "\n";
    
    std::stringstream log;
    log << " ";
    log << std::left << std::setw(14) << task_name;
    log << std::left << std::setw(9) << std::to_string(delayed_data -> data_time);
    log << std::left << std::setw(15) << "READ";
    log << std::left << std::setw(16) << std::to_string(size);

    return log.str() + data.str();
}

void Logger::_2019_13914_task_read_write_logger(std::string task_name){
    std::ofstream log;

    static bool init = false;
    if(!init){
        log.open(utils::cpsim_path + "/Log/2019-13914_read_write.log", std::ios::app|std::ofstream::trunc);
        log << "[ TASK NAME ] [ TIME ] [ READ/WRITE ] [ DATA LENGTH ] [ RAW DATA ]\n";
        log.close();
        init = true;
    }
    log.open(utils::cpsim_path + "/Log/2019-13914_read_write.log", std::ios::app);
    log << task_name;
    log.close();
    return;
}

void Logger::_2019_13914_real_cyber_event_logger(long long time, int job_id, std::string event_type){
    std::ofstream event_log;

    static bool init = false;
    if(!init){
        event_log.open(utils::cpsim_path+"/Log/2019-13914_event.log",std::ios::app|std::ofstream::trunc);
        event_log << "[ TIME ] [ JOB ID ] [ EVENT TYPE ]\n";
        event_log.close();
        init = true;
    }

    std::stringstream log;
    log << " ";
    log << std::left << std::setw(9) << std::to_string(time);
    log << std::left << std::setw(11) << "J" + std::to_string(job_id);
    log << std::left << std::setw(15) << event_type;

    Event event;
    event.time = time;
    event.job_id = job_id;
    event.log = log.str();
    event_buffer.push(event);
    utils::mtx_data_log.lock();
    while(event_buffer.size() > 10){
        std::ofstream event_log;
        event_log.open(utils::cpsim_path+"/Log/2019-13914_event.log",std::ios::app);
        event_log << event_buffer.top().log << "\n";
        event_buffer.pop();
        event_log.close();
    }
    utils::mtx_data_log.unlock();
}