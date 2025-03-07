// Display images inside a terminal
// Copyright (C) 2023  JustKidding
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef __CANVAS__
#define __CANVAS__

#include "image.hpp"
#include "terminal.hpp"
#include "dimensions.hpp"

#include <memory>
#include <spdlog/spdlog.h>

class Canvas
{
public:
    static auto create(const Terminal& terminal, Flags& flags,
            spdlog::logger& logger, std::mutex& img_lock) -> std::unique_ptr<Canvas>;
    virtual ~Canvas() = default;

    virtual auto init(const Dimensions& dimensions,
            std::shared_ptr<Image> image) -> void = 0;
    virtual void draw() = 0;
    virtual void clear() = 0;

    virtual void show() {}
    virtual void hide() {}
    virtual void toggle() {}
};

#endif

