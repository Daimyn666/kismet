/*
    This file is part of Kismet

    Kismet is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kismet is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Kismet; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "config.h"

#include "kis_pcapnglogfile.h"
#include "messagebus.h"

kis_pcapng_logfile::kis_pcapng_logfile(shared_log_builder in_builder) :
    kis_logfile(in_builder),
    buffer{4096, 1024} {
    pcapng = nullptr;
    pcapng_file = nullptr;
}

kis_pcapng_logfile::~kis_pcapng_logfile() {
    close_log();
}

bool kis_pcapng_logfile::open_log(std::string in_path) {
    local_locker lock(&log_mutex);

    set_int_log_path(in_path);

    pcapng_file = fopen(in_path.c_str(), "w");

    if (pcapng_file == nullptr) {
        _MSG_ERROR("Failed to open pcapng log '{}' - {}",
                in_path, kis_strerror_r(errno));
        return false;
    }

    pcapng = new pcapng_stream_packetchain(buffer, nullptr, nullptr, 16384);

    _MSG_INFO("Opened pcapng log file '{}'", in_path);

    set_int_log_open(true);

    auto thread_p = std::promise<void>();
    auto thread_f = thread_p.get_future();

    stream_t = std::thread([this, &thread_p]() {
            thread_p.set_value();

            while (buffer.running() || buffer.size() > 0) {
                buffer.wait();

                char *data;

                auto sz = buffer.get(&data);

                if (sz > 0) {
                    if (fwrite(data, sz, 1, pcapng_file) == 0) {
                        _MSG_ERROR("Error writing to pcapng log '{}' - {}", get_log_path(),
                                kis_strerror_r(errno));
                        close_log();
                        return;
                    }
                }

                buffer.consume(sz);
            }

        });

    thread_f.wait();
    pcapng->start_stream();

    return true;
}

void kis_pcapng_logfile::close_log() {
    local_locker lock(&log_mutex);

    set_int_log_open(false);

    buffer.cancel();

    if (stream_t.joinable())
        stream_t.join();

    if (pcapng_file)
        fclose(pcapng_file);

    pcapng_file = nullptr;
}

