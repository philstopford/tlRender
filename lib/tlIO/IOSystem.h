// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/IO.h>

#include <tlCore/ISystem.h>

namespace tl
{
    namespace io
    {
        //! I/O system.
        class System : public system::ISystem
        {
            TLRENDER_NON_COPYABLE(System);

        protected:
            void _init(const std::shared_ptr<system::Context>&);
            
            System();

        public:
            virtual ~System();

            //! Create a new system.
            static std::shared_ptr<System> create(const std::shared_ptr<system::Context>&);

            //! Set the plugin options.
            void setOptions(const Options&);

            //! Get the list of plugins.
            const std::vector<std::shared_ptr<IPlugin> >& getPlugins() const;
            
            //! Add a plugin.
            void addPlugin(const std::shared_ptr<IPlugin>&);
            
            //! Remove a plugin.
            void removePlugin(const std::shared_ptr<IPlugin>&);

            //! Get a plugin.
            template<typename T>
            std::shared_ptr<T> getPlugin() const;

            //! Get a plugin for the given path.
            std::shared_ptr<IPlugin> getPlugin(const file::Path&) const;

            //! Get the supported file extensions.
            std::set<std::string> getExtensions(int types =
                static_cast<int>(FileType::Movie) |
                static_cast<int>(FileType::Sequence) |
                static_cast<int>(FileType::Audio)) const;

            //! Get the file type for the given extension.
            FileType getFileType(const std::string&) const;

            // Create a reader for the given path.
            std::shared_ptr<IRead> read(
                const file::Path&,
                const Options & = Options());

            // Create a reader for the given path and memory locations.
            std::shared_ptr<IRead> read(
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const Options & = Options());

            // Create a writer for the given path.
            std::shared_ptr<IWrite> write(
                const file::Path&,
                const Info&,
                const Options& = Options());

        private:
            std::vector<std::shared_ptr<IPlugin> > _plugins;
        };
    }
}

#include <tlIO/IOSystemInline.h>
