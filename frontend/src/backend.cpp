// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2022 E. Devlin and T. Youngs
#include "backend.h"
#include "args.h"
#include <QCommandLineParser>
#include <QDebug>

namespace
{
// This must match that defined in backend/config module
constexpr auto ENVIRON_NAME_PREFIX = "JV2_";

/**
 * Take a program argument name and convert to a backend environment variable name.
 * Replace '-' with '_' and add prefix
 */
QString argToEnvironName(QString argName) { return ENVIRON_NAME_PREFIX + argName.replace("-", "_"); }
} // namespace

/**
 * Configure process for start to be called
 */
Backend::Backend(const QCommandLineParser &args) : process_(), env_(QProcessEnvironment::systemEnvironment())
{
    configureProcessArgs();
    configureEnvironment(args);
    process_.setProcessEnvironment(env_);
}

/**
 * Start the backend process and hold the process open
 */
void Backend::start()
{
    qDebug() << "Starting backend process";
    process_.start();
    process_.waitForStarted();
    qDebug() << "Backend process started with pid " << process_.processId();
}

/**
 * Stop the process
 */
void Backend::stop()
{
    qDebug() << "Stopping backend process with pid " << process_.processId();
    process_.terminate();
    process_.waitForFinished();
}

// Private

void Backend::configureProcessArgs()
{
    process_.setProgram("gunicorn");
    QStringList args;
    // clang-format off
    args << "--bind" << Backend::bind_address()
         << "--graceful-timeout" << "120"
         << "--timeout" << "120"
         << "jv2backend.app:create_app";
    // clang-format on
    process_.setArguments(args);
    process_.setProcessChannelMode(QProcess::ForwardedChannels);
}

void Backend::configureEnvironment(const QCommandLineParser &args)
{
    if (args.isSet(Args::RunLocatorClass))
        env_.insert(argToEnvironName(Args::RunLocatorClass), args.value(Args::RunLocatorClass));
    if (args.isSet(Args::RunLocatorPrefix))
        env_.insert(argToEnvironName(Args::RunLocatorPrefix), args.value(Args::RunLocatorPrefix));
}
