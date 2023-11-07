// pragma once

#include <streambuf>
#include <iostream>

#include <windows.h>

enum class Mode { Client, Server };

struct np_buf : std::streambuf
{
    using int_type = std::streambuf::int_type;
    using traits_type = std::streambuf::traits_type;
    using char_type = std::streambuf::char_type;

    static size_t const buffer_size = 4096;

    np_buf(std::string const &pipe_name, Mode mode) {
        setg(read_buffer_, read_buffer_ + buffer_size, read_buffer_ + buffer_size);
        setp(write_buffer_, write_buffer_ + buffer_size);

        if (mode == Mode::Server) {
            pipe_ =CreateNamedPipeA(pipe_name.c_str(), PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                                 1, 1024 * 16, 1024 * 16, NMPWAIT_USE_DEFAULT_WAIT, NULL);

            if (pipe_ == INVALID_HANDLE_VALUE) {
                throw std::runtime_error("CreateNamedPipe failed ");
            }
            if (!ConnectNamedPipe(pipe_, NULL)) {
                throw std::runtime_error("ConnectNamedPipe failed");
            }
        } else { // Client mode:
            if (!WaitNamedPipeA(pipe_name.c_str(), NMPWAIT_WAIT_FOREVER)) {
                throw std::runtime_error("WaitNamedPipe failed");
			}

            pipe_ = CreateFileA(pipe_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
            if (pipe_ == INVALID_HANDLE_VALUE) {
                throw std::runtime_error("CreateFile failed");
			}
        }
    }

	~np_buf() {
		CloseHandle(pipe_);
    }

protected:
    int_type underflow() {
        DWORD read;

        if (::ReadFile(pipe_, eback(), buffer_size, &read, NULL)) {
            setg(eback(), eback(), eback() + read);
            return traits_type::to_int_type(*eback());
        } else {
            return traits_type::eof();
        }
    }

    int sync() {
        overflow(traits_type::eof());
        return 0;
    }

    int_type overflow(int_type c) {
        if (const size_t chars = pptr() - pbase()) {
            DWORD written;
            if (!WriteFile(pipe_, pbase(), chars, &written, NULL)) {
                return traits_type::eof();
            }
            setp(pbase(), epptr());
        }

        if (!traits_type::eq_int_type(c, traits_type::eof())) {
            char_type e = traits_type::to_char_type(c);
            DWORD written;

            if (!WriteFile(pipe_, &e, 1, &written, NULL)) {
                return traits_type::eof();
            }
        }

        return traits_type::not_eof(c);
    }

    HANDLE pipe_;
    char read_buffer_[buffer_size];
    char write_buffer_[buffer_size];
};

struct np_stream : std::iostream {
    np_stream(char const *name, Mode mode) : std::iostream(&buffer_), buffer_(name, mode) { }
private:
    np_buf buffer_;
};
