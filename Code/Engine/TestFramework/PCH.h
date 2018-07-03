#pragma once

#include <Foundation/PCH.h>
#include <algorithm>
#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>

// <StaticLinkUtil::StartHere>
// all include's before this will be left alone and not replaced by the StaticLinkUtil
// all include's AFTER this will be removed by the StaticLinkUtil and updated by what is actually used throughout the library



#include <Foundation/Basics.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/FileSystem/DataDirTypeFolder.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/JSONWriter.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Image/Image.h>
#include <Foundation/Image/ImageConversion.h>
#include <Foundation/Image/ImageUtils.h>
#include <Foundation/Logging/ConsoleWriter.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Math/Math.h>
#include <Foundation/Memory/MemoryTracker.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/System/SystemInformation.h>
#include <Foundation/Time/Time.h>
#include <Foundation/Time/Timestamp.h>
#include <Foundation/Utilities/CommandLineUtils.h>
#include <Foundation/Utilities/EnumerableClass.h>

#ifdef EZ_USE_QT
#include <QWidget>
#include <QtWidgets>
#endif
