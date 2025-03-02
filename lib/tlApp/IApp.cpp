// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlApp/IApp.h>

#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <iostream>

namespace tl
{
    namespace app
    {
        struct IApp::Private
        {
            std::vector<std::string> cmdLine;
            std::string cmdLineName;
            std::string cmdLineSummary;
            std::vector<std::shared_ptr<ICmdLineArg> > cmdLineArgs;
            std::vector<std::shared_ptr<ICmdLineOption> > cmdLineOptions;
            std::shared_ptr<observer::ListObserver<log::Item> > logObserver;
        };

        void IApp::_init(
            int argc,
            char* argv[],
            const std::shared_ptr<system::Context>& context,
            const std::string& cmdLineName,
            const std::string& cmdLineSummary,
            const std::vector<std::shared_ptr<ICmdLineArg> >& args,
            const std::vector<std::shared_ptr<ICmdLineOption> >& options)
        {
            TLRENDER_P();

            _context = context;

            // Parse the command line.
            for (int i = 1; i < argc; ++i)
            {
                p.cmdLine.push_back(argv[i]);
            }
            p.cmdLineName = cmdLineName;
            p.cmdLineSummary = cmdLineSummary;
            p.cmdLineArgs = args;
            p.cmdLineOptions = options;
            p.cmdLineOptions.push_back(CmdLineFlagOption::create(
                _options.log,
                { "-log" },
                "Print the log to the console."));
            p.cmdLineOptions.push_back(CmdLineFlagOption::create(
                _options.help,
                { "-help", "-h", "--help", "--h" },
                "Show this message."));
            _exit = _parseCmdLine();

            // Setup the log.
            if (_options.log)
            {
                p.logObserver = observer::ListObserver<log::Item>::create(
                    context->getSystem<log::System>()->observeLog(),
                    [this](const std::vector<log::Item>& value)
                    {
                        const size_t options =
                            static_cast<size_t>(log::StringConvert::Time) |
                            static_cast<size_t>(log::StringConvert::Prefix);
                        for (const auto& i : value)
                        {
                            _print("[LOG] " + toString(i, options));
                        }
                    },
                    observer::CallbackAction::Suppress);
            }
        }
        
        IApp::IApp() :
            _p(new Private)
        {}

        IApp::~IApp()
        {}

        const std::shared_ptr<system::Context>& IApp::getContext() const
        {
            return _context;
        }

        int IApp::getExit() const
        {
            return _exit;
        }

        void IApp::_log(const std::string& value, log::Type type)
        {
            _context->log(_p->cmdLineName, value, type);
        }

        void IApp::_print(const std::string& value)
        {
            std::cout << value << std::endl;
        }

        void IApp::_printNewline()
        {
            std::cout << std::endl;
        }

        void IApp::_printError(const std::string& value)
        {
            std::cerr << "ERROR: " << value << std::endl;
        }

        int IApp::_parseCmdLine()
        {
            TLRENDER_P();
            for (const auto& i : p.cmdLineOptions)
            {
                try
                {
                    i->parse(p.cmdLine);
                }
                catch (const std::exception& e)
                {
                    throw std::runtime_error(string::Format("Cannot parse option \"{0}\": {1}").
                        arg(i->getMatchedName()).
                        arg(e.what()));
                }
            }
            size_t requiredArgs = 0;
            size_t optionalArgs = 0;
            for (const auto& i : p.cmdLineArgs)
            {
                if (!i->isOptional())
                {
                    ++requiredArgs;
                }
                else
                {
                    ++optionalArgs;
                }
            }
            if (p.cmdLine.size() < requiredArgs ||
                p.cmdLine.size() > requiredArgs + optionalArgs ||
                _options.help)
            {
                  _printCmdLineHelp();
                return 1;
            }
            for (const auto& i : p.cmdLineArgs)
            {
                try
                {
                    if (!(p.cmdLine.empty() && i->isOptional()))
                    {
                        i->parse(p.cmdLine);
                    }
                }
                catch (const std::exception& e)
                {
                    throw std::runtime_error(string::Format("Cannot parse argument \"{0}\": {1}").
                        arg(i->getName()).
                        arg(e.what()));
                }
            }
            return 0;
        }

        void IApp::_printCmdLineHelp()
        {
            TLRENDER_P();
            _print("\n" + p.cmdLineName + "\n");
            _print("    " + p.cmdLineSummary + "\n");
            _print("Usage:\n");
            {
                std::stringstream ss;
                ss << "    " + p.cmdLineName;
                if (p.cmdLineArgs.size())
                {
                    std::vector<std::string> args;
                    for (const auto& i : p.cmdLineArgs)
                    {
                        const bool optional = i->isOptional();
                        args.push_back(
                            (optional ? "[" : "(") +
                            string::toLower(i->getName()) +
                            (optional ? "]" : ")"));
                    }
                    ss << " " << string::join(args, " ");
                }
                if (p.cmdLineOptions.size())
                {
                    ss << " [option],...";
                }
                _print(ss.str());
                _printNewline();
            }
            _print("Arguments:\n");
            for (const auto& i : p.cmdLineArgs)
            {
                _print("    " + i->getName());
                _print("        " + i->getHelp());
                _printNewline();
            }
            _print("Options:\n");
            for (const auto& i : p.cmdLineOptions)
            {
                bool first = true;
                for (const auto& j : i->getHelpText())
                {
                    if (first)
                    {
                        first = false;
                        _print("    " + j);
                    }
                    else
                    {
                        _print("        " + j);
                    }
                }
                _printNewline();
            }
        }
    }
}
