/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#pragma once 

#include <fstream>
#include <type_traits>
#include <stdexcept>
#include <memory>
#include <atomic>

namespace more {

    template <typename Tp>
    struct persistent
    {
        static_assert(std::is_pod<Tp>::value, "persistent Tp must be a pod");

        persistent(const char *name)
        : stream_(new std::fstream(name, std::ios::in|std::ios::out|std::ios::binary))
        , value_ (new std::atomic<Tp>())
        {
            if (!*stream_)
            {
                value_initialize_(name);
                
                stream_->open(name, std::ios::in|std::ios::out|std::ios::binary);
                if (!*stream_)
                    throw std::runtime_error("persistent::persistent");
            }
            else
            {
                Tp value;
                stream_->read(reinterpret_cast<char *>(&value), static_cast<std::streamsize>(sizeof(Tp)));
                if (!*stream_)
                    throw std::runtime_error("persistent::persistent");
                value_->store(value);
            }
        }

        persistent(std::string const &name)
        : persistent(name.c_str())
        {}

        persistent(const char *name, Tp const &value)
        : stream_(name, std::ios::trunc|std::ios::out|std::ios::binary) 
        , value_(new std::atomic<Tp>(value))
        {
            store_();
        }
        
        persistent(std::string const &name, Tp const &value)
        : persistent(name.c_str(), value)
        {}

        persistent(const persistent &) = delete;
        persistent& operator=(const persistent &) = delete;

        persistent(persistent &&other)
        : stream_(std::move(other.stream_))
        , value_(std::move(other.value_))
        {
        }

        persistent& operator=(persistent &&other)
        {
            if (this != &other)
            {
                stream_ = std::move(other.stream_);
                value_  = std::move(other.value_);
            }
            return *this;
        }

        Tp 
        get() const
        {
            return value_->load();
        }

        template <typename F>
        Tp update(F fun)
        {
            auto value = value_->load();
            fun(value);
            value_->store(value);
            store_();
            return value;
        }
    

    private:               

        void value_initialize_(const char *name)
        {
            stream_->open(name, std::ios::trunc|std::ios::out|std::ios::binary);
            value_->store(Tp());
            store_();
            stream_->close();
        }

        void store_()
        {
            stream_->seekg(0);
            auto value = value_->load();
            stream_->write(reinterpret_cast<const char *>(&value), static_cast<std::streamsize>(sizeof(Tp)));
            if (!*stream_)
                throw std::runtime_error("persistent::store");
        }

        std::unique_ptr<std::fstream> stream_;

        mutable std::unique_ptr<std::atomic<Tp>> value_;
    };


} // namespace more
