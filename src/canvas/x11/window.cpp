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

#include "window.hpp"
#include "free_delete.hpp"

#include <xcb/xcb.h>
#include <iostream>
#include <chrono>

Window::Window(
        xcb_connection_t *connection,
        xcb_screen_t *screen,
        xcb_window_t parent,
        int x, int y, int max_width, int max_height
):
connection(connection),
screen(screen),
parent(parent),
width(max_width),
height(max_height)
{
    unsigned int value_mask = XCB_CW_BACK_PIXEL | XCB_CW_BORDER_PIXEL | XCB_CW_EVENT_MASK | XCB_CW_COLORMAP;
    unsigned int value_list[4] = {
        this->screen->black_pixel,
        this->screen->black_pixel,
        XCB_EVENT_MASK_EXPOSURE,
        this->screen->default_colormap
    };

    xcb_window_t wid = xcb_generate_id(this->connection);
    this->gc = xcb_generate_id(this->connection);
    xcb_create_window(this->connection,
            this->screen->root_depth,
            wid,
            this->parent,
            x, y,
            max_width, max_height,
            0,
            XCB_WINDOW_CLASS_INPUT_OUTPUT,
            this->screen->root_visual,
            value_mask,
            value_list);

    xcb_create_gc(connection, gc, wid, 0, nullptr);
    this->window = wid;
    xcb_map_window(this->connection, this->window);

    this->event_handler = std::make_unique<std::jthread>([this]() {
        this->handle_events();
    });

    xcb_flush(connection);
}

auto Window::draw(const Image& image) -> void
{
    auto ptr = malloc(image.size());
    xcb_image = xcb_image_create_native(connection,
            image.width(),
            image.height(),
            XCB_IMAGE_FORMAT_Z_PIXMAP,
            screen->root_depth,
            nullptr,
            image.size(),
            const_cast<unsigned char*>(image.data()));
    xcb_clear_area(connection, true, window, 0, 0, 0, 0);
    xcb_flush(connection);
}

Window::~Window()
{
    terminate_event_handler();
    xcb_image_destroy(xcb_image);
    xcb_free_gc(connection, gc);
    xcb_unmap_window(connection, window);
    xcb_destroy_window(connection, window);
}

auto Window::handle_events() -> void
{
    while (true) {
        std::unique_ptr<xcb_generic_event_t, free_delete> event {
            xcb_wait_for_event(this->connection)
        };
        auto response = event->response_type & ~0x80;
        switch (response) {
            case XCB_EXPOSE: {
                auto expose = reinterpret_cast<xcb_expose_event_t*>(event.get());
                if (expose->x == 69 && expose->y == 420) return;
                if (!xcb_image) continue;
                xcb_image_put(connection, window, gc, xcb_image, 0, 0, 0);
                break;
            }
            default: {
                break;
            }
        }
    }
}

auto Window::terminate_event_handler() -> void
{
    auto e = std::make_unique<xcb_expose_event_t>();
    e->response_type = XCB_EXPOSE;
    e->window = window;
    e->x = 69;
    e->y = 420;
    xcb_send_event(connection, false, window,
            XCB_EVENT_MASK_EXPOSURE, reinterpret_cast<char*>(e.get()));
    xcb_flush(this->connection);
}
