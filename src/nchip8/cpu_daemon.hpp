//
// Created by ocanty on 17/09/18.
//

#ifndef CHIP8_NCURSES_CPU_DAEMON_HPP
#define CHIP8_NCURSES_CPU_DAEMON_HPP


#include <thread>
#include <vector>
#include <mutex>
#include <functional>
#include <queue>
#include <functional>
#include <condition_variable>

#include "cpu.hpp"
#include "cpu_message.hpp"

namespace nchip8
{

//! @brief  The cpu_daemon creates the cpu thread,
//!         passes messages to the cpu and controls it's state
//!         e.g. calling for an instruction to be executed or not
class cpu_daemon
{
public:
    //! @brief Constructor
    cpu_daemon();

    //! @brief Destructor
    virtual ~cpu_daemon();

    //! @brief          Send a message to the cpu thread
    //! @param message  The cpu_message structure
    void send_message(const cpu_message &);

    //! @brief      Register a message handler to be called in the cpu thread when it receives a message
    //! @param type Message type
    //! @param hdl  Message handler container
    void register_message_handler(const cpu_message_type &type, const cpu_message_handler &hdl);

    //! CPU state enumeration
    enum cpu_state
    {
        paused,
        running
    };

    //! @brief Get cpu_state
    //! @returns cpu_state
    cpu_state get_cpu_state() const;

    //! @brief Set cpu_state
    void set_cpu_state(const cpu_state &);

    void set_cpu_clockspeed(const size_t&);

    //! @brief Returns current screen mode
    //! @see cpu::screen_mode
    const cpu::screen_mode& get_screen_mode() const;

    //! @brief      Returns a reference to screen data
    //! @returns    Const vector reference that contains the screen data
    //!             (where true = pixel on, false = pixel off)
    //! @details    Screen array is ALWAYS the hires size, even if cpu is
    const std::array<bool, 128*64>& get_screen_framebuffer() const;

    //! @brief Get's the status of a pixel on the screen (on/off)
    bool get_screen_xy(const std::uint8_t&x , const std::uint8_t& y) const;

    void set_key_down(const std::uint8_t& key);
    void set_key_up(const std::uint8_t &key);

    //! @brief Returns a reference to the general purpose cpu registers (i.e V0-V15)
    const std::array<std::uint8_t, 16>& get_gpr() const;

    //! @brief Get I register
    const std::uint16_t get_i() const;

    //! @brief Get Stack Pointer
    const std::uint16_t get_sp() const;

    //! @brief Get Program Counter
    const std::uint16_t get_pc() const;

    //! @brief Get Delay Timer
    const std::uint8_t get_dt() const;

    //! @brief Get Sound Timer
    const std::uint8_t get_st() const;

    //! @brief Get stack
    const std::array<std::uint16_t, 16> get_stack() const;


    
private:
    //! The number of times a second we execute a CPU cycle
    std::size_t m_clock_speed = 500;

    //! CPU instance
    cpu m_cpu;

    //! Current cpu state, e.g. paused, running
    cpu_state m_cpu_state;

    //! Thread object for void cpu_thread()
    std::thread m_cpu_thread;

    //! Each instruction we execute using the cpu class is ran in here
    void cpu_thread();

    //! Locked when the message queue is being processed/operated on
    std::mutex m_cpu_thread_mutex;

    //! The list of messages that still need to be processed by the cpu thread
    std::queue<cpu_message> m_unhandled_messages;

    //! Message handlers, first indexed by type, and then by each handler for that type
    std::vector<std::vector<cpu_message_handler>> m_message_handlers;
};

}
#endif //CHIP8_NCURSES_CPU_DAEMON_HPP
