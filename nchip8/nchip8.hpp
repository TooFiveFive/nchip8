//
// Created by ocanty on 17/09/18.
//

#ifndef CHIP8_NCURSES_NCHIP8_HPP
#define CHIP8_NCURSES_NCHIP8_HPP

#include <memory>
#include <vector>

#include <bits/stdc++.h>

#include "io.hpp"
#include "cpu_daemon.hpp"
#include "gui.hpp"

namespace nchip8
{

class nchip8_app
{
public:
    nchip8_app(const std::vector<std::string> &args);

    ~nchip8_app();

private:
    void process_arguments();

    std::unique_ptr<gui> m_gui;
    std::shared_ptr<cpu_daemon> m_cpu_daemon;
};

}

#endif //CHIP8_NCURSES_NCHIP8_HPP
