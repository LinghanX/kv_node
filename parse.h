//
// Created by Linghan Xing on 11/25/18.
//

#ifndef KEY_VALUE_PARSE_H
#define KEY_VALUE_PARSE_H

#include <vector>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <fstream>
#include <iostream>
#include "include/spdlog/spdlog.h"
#include "include/spdlog/sinks/stdout_color_sinks.h"

// returns a tuple of string with coordinator address and port
std::tuple<std::string, std::string> handle_input(int argc, char ** argv);

#endif //KEY_VALUE_PARSE_H
