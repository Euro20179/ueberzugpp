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

#include "opencv.hpp"
#include "util.hpp"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/ocl.hpp>

OpencvImage::OpencvImage(const Terminal& terminal, const Dimensions& dimensions, const Flags& flags,
            const std::string& filename, bool in_cache):
terminal(terminal),
flags(flags),
dimensions(dimensions),
path(filename),
max_width(dimensions.max_wpixels()),
max_height(dimensions.max_hpixels()),
in_cache(in_cache)
{
    image = cv::imread(filename, cv::IMREAD_UNCHANGED);

    auto opencl_ctx = cv::ocl::Context::getDefault();
    opencl_available = opencl_ctx.ptr() != nullptr;

    process_image();
}

OpencvImage::~OpencvImage()
{
    image.release();
}

auto OpencvImage::filename() const -> std::string
{
    return path.string();
}

auto OpencvImage::width() const-> int
{
    return image.cols;
}

auto OpencvImage::height() const -> int
{
    return image.rows;
}

auto OpencvImage::size() const -> unsigned long
{
    return _size;
}

auto OpencvImage::data() const -> const unsigned char*
{
    return image.data;
}

auto OpencvImage::channels() const -> int
{
    return image.channels();
}

// only use opencl if required
auto OpencvImage::resize_image() -> void
{
    if (in_cache) return;
    auto [new_width, new_height] = get_new_sizes(max_width, max_height, dimensions.scaler);
    if (new_width == 0 && new_height == 0) return;

    if (opencl_available) {
        uimage = image.getUMat(cv::ACCESS_RW);
        cv::resize(uimage, uimage, cv::Size(new_width, new_height), 0, 0, cv::INTER_AREA);
        image = uimage.getMat(cv::ACCESS_RW);
    } else {
        cv::resize(image, image, cv::Size(new_width, new_height), 0, 0, cv::INTER_AREA);
    }

    if (flags.no_cache) return;
    auto save_location = util::get_cache_file_save_location(path);
    try {
        cv::imwrite(save_location, image);
    } catch (const cv::Exception& ex) {}
}

void OpencvImage::process_image()
{
    resize_image();

    if (flags.output == "kitty") {
        if (image.channels() == 4) {
            cv::cvtColor(image, image, cv::COLOR_BGRA2RGBA);
        } else {
            cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
        }
    } else if (flags.output == "sixel") {
        if (image.channels() == 4) {
            cv::cvtColor(image, image, cv::COLOR_BGRA2RGB);
        } else {
            cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
        }
    } else if (flags.output == "x11") {
        cv::cvtColor(image, image, cv::COLOR_BGR2BGRA);
    }

    _size = image.total() * image.elemSize();
}
