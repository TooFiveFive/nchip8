//
// Created by ocanty on 26/09/18.
//

#include "io.hpp"
#include "gui.hpp"

#include <curses.h>
#include <clocale>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>

namespace nchip8
{

gui::gui()
{
    this->rebuild_windows();
    this->loop();
}

gui::~gui()
{

}

void gui::rebuild_windows()
{
    nchip8::log << "[gui] rebuilt windows" << std::endl;

    m_window = std::shared_ptr<::WINDOW>(::initscr(), ::wdelch);
    ::setlocale(LC_ALL, " ");   // set locale
    ::cbreak();
    ::noecho();
    ::nonl();
    ::intrflush(stdscr, FALSE);
    ::keypad(stdscr, TRUE);

    ::wborder(m_window.get(), 0, 0, 0, 0, 0, 0, 0, 0);
    getmaxyx(m_window.get(), m_window_h, m_window_w);

    m_screen_window = std::shared_ptr<::WINDOW>(::newwin(16 + 2, 64 + 2, 0, 1), ::wdelch);
    ::wborder(m_screen_window.get(), 0, 0, 0, 0, 0, 0, 0, 0);
    ::wrefresh(m_screen_window.get());


    m_log_window = std::shared_ptr<::WINDOW>(::newwin(m_window_h - 18, 64 + 2, 18, 1), ::wdelch);
    ::wborder(m_log_window.get(), 0, 0, 0, 0, 0, 0, 0, 0);
    ::wrefresh(m_log_window.get());
    this->update_log_window();
}

void gui::update_windows_on_resize()
{
    // get current terminal size
    int new_term_w = 0;
    int new_term_h = 0;
    getmaxyx(m_window.get(), new_term_h, new_term_w);

    // resize the window if it's changed within the past update
    if (new_term_w != m_window_w || new_term_h != m_window_h)
    {
        m_window_w = new_term_w;
        m_window_h = new_term_h;
        this->rebuild_windows();
    }
}

void gui::loop()
{
    bool die = false;
    while (!die)
    {
        update_windows_on_resize();
        update_log_on_global_log_change();
        // dont eat cpu
        std::this_thread::sleep_for(std::chrono::milliseconds(1000/60));
    }
}

void gui::update_log_on_global_log_change()
{
    if (!nchip8::log.str().empty())
    {
        static std::string line;

        while (nchip8::log.good() && !nchip8::log.eof())
        {
            std::getline(nchip8::log, line);
            m_gui_log.push_back(line);
            line.clear();
        }


        this->update_log_window();
    }

}

void gui::update_log_window()
{
    if (m_log_window == nullptr) return;

    // get height of log window and get its height minus its borders
    int log_window_w, log_window_h = 0;
    getmaxyx(m_log_window.get(), log_window_h, log_window_w);

    int height = log_window_h - 1; // height without borders
    int y = height;// initial y coordinate (from bottom) (skipping bottom border)

    // find out how many lines we need to draw, its either the max window height if it fits fully
    // or the size that doesnt
    auto draw_size = (m_gui_log.size() >= height ? height : m_gui_log.size());
    for(auto it = m_gui_log.rbegin(); it != (m_gui_log.rbegin()+draw_size); it++)
    {
        ::mvwprintw(m_log_window.get(), y, 1, (*it).c_str());
        y--;
    }

    ::wborder(m_log_window.get(), 0, 0, 0, 0, 0, 0, 0, 0);
    ::wrefresh(m_log_window.get());
}

void gui::update_screen()
{

}

void gui::set_cpu_target(const std::shared_ptr<cpu_daemon>& cpu)
{
    m_cpu_daemon = cpu;
}

}