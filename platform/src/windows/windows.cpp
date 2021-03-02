#include "platform/fs.hpp"
#include "common/log.hpp"
#include "platform/platform.hpp"

#include "stb_ds.h"

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>

#include "common/bytes.hpp"


namespace lucid::platform
{
    static struct
    {
        char* key = ""; //DirectoryPath
        HANDLE value; // Windows change handle
    }* HandleByDirectoryPath = NULL;

    static struct
    {
        HANDLE key; // Directory changed handle
        DirectoryChangedListener* value; // Listeners
    }* ListenersByHandle = NULL;

    HANDLE* DirectoryChangedHandles;
    
    i8 AddDirectoryListener(const String& InDirectoryPath, DirectoryChangedListener InListener)
    {
        // There is already an entry for this directory, let's just add a listener to it
        if(shgeti(HandleByDirectoryPath, *InDirectoryPath) != -1)
        {
            const HANDLE Handle = shget(HandleByDirectoryPath, *InDirectoryPath);
            DirectoryChangedListener* Listeners = hmget(ListenersByHandle, Handle);
            arrput(Listeners, InListener);
            return 0;
        }

        //@TODO Unicode support
        // If there is no listener yet for this directory, then let's add one
        const HANDLE ChangeHandle = FindFirstChangeNotificationA(*InDirectoryPath, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);
 
        if (ChangeHandle == INVALID_HANDLE_VALUE || ChangeHandle == NULL) 
        {
            LUCID_LOG(LogLevel::WARN, "[platform-windows] FindFirstChangeNotification function failed with error code %d", GetLastError());
            return -1;
        }

        // Add path - handle mapping
        shput(HandleByDirectoryPath, *InDirectoryPath, ChangeHandle);

        // Add handle - listeners mapping
        hmput(ListenersByHandle, ChangeHandle, NULL);

        // Add the initial listener
        arrput(hmget(ListenersByHandle, ChangeHandle), InListener);

        // Add the handle to handles array
        arrput(DirectoryChangedHandles, ChangeHandle);

        return 0;
    }

    void RemoveDirectoryListener(const String& InDirectoryPath, DirectoryChangedListener InListener)
    {
        if(shgeti(HandleByDirectoryPath, *InDirectoryPath) == -1)
        {
            return;
        }

        HANDLE Handle = shget(HandleByDirectoryPath, *InDirectoryPath);

        DirectoryChangedListener* Listeners =  hmget(ListenersByHandle, Handle);

        if (arrlen(Listeners) == 1)
        {
            hmdel(ListenersByHandle, Handle);
            shdel(HandleByDirectoryPath, *InDirectoryPath);
            return;
        }

        for (int i = 0; i < arrlen(Listeners); ++i)
        {
            if (Listeners[i] == InListener)
            {
                arrdel(Listeners, i);
                return;
            }
        }
    }

    void _UpdateSystem()
    {
        // Check for changes in directory that are being listened
        for (int i = 0; i < arrlen(DirectoryChangedHandles); ++i)
        {
            const HANDLE Handle = DirectoryChangedHandles[i];
            const auto WaitResult = WaitForSingleObject(Handle, 0);
            if (WaitResult == WAIT_FAILED)
            {
                LUCID_LOG(LogLevel::WARN, "[platform-windows] WaitForMultipleObjects function failed with error code %d", GetLastError());
                return;
            }

            // The directory changed, notify the listeners
            if (WaitResult == WAIT_OBJECT_0)
            {
                DirectoryChangedListener* Listeners = hmget(ListenersByHandle, Handle);
                for (int j = 0; j < arrlen(Listeners); ++j)
                {
                    Listeners[j]();
                }
            }

            // Listen for next changes
            if(FindNextChangeNotification(DirectoryChangedHandles[i]) == FALSE)
            {
                LUCID_LOG(LogLevel::WARN, "[platform-windows] FindNextChangeNotification function failed with error code %d", GetLastError());
                return;
            }
        }
    }

    i8 ExecuteCommand(const String& InCommand, const StaticArray<String>& Args)
    {
        DString CommandToExecute = InCommand.ToDString();

        for (int i = 0; i < Args.Length; ++i)
        {
            CommandToExecute.Append(" ");
            CommandToExecute.Append(*Args[i]);
        }
       
        if(system(*CommandToExecute) == 0)
        {
            CommandToExecute.Free();
            return 0;
        }

        CommandToExecute.Free();
        LUCID_LOG(LogLevel::WARN, "[platform-windows] system function failed with error code %d", GetLastError());
        return -1;
    }
    
}
