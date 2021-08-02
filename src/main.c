#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include <fileapi.h>
#include <windows.h>

#include "drv.c"
#include "msg.c"
#include "priv.c"

/*============================================================================*/

enum error_code {
    success,
    set_privileges_fail,
    get_drives_list_fail
};

enum open_file_mode {
    read,
    write
};

/*============================================================================*/

const char gc_hiber_sign[] = { 'H', 'I', 'B', 'R' };
const char gc_null_sign[] = { 0, 0, 0, 0 };
const LPWSTR gc_hiber_file_name = L"hiberfil.sys";

/*============================================================================*/

int check_hiberfil(const LPWSTR drive);
int reset_hiberfil(const LPWSTR drive);
LPWSTR
open_hiber_file(const LPWSTR drive, enum open_file_mode mode);

/*============================================================================*/

int WINAPI wWinMain(HINSTANCE CurI, HINSTANCE PrevI, LPWSTR Cmd, int ShowCmd)
{
    LPWSTR sys_drv, drives, cur_drv;

    if (!get_backup_privileges(enable))
        return set_privileges_fail;

    sys_drv = get_sys_drive();

    drives = get_drives();
    if (!drives)
        return get_drives_list_fail;

    cur_drv = drives;
    for (;;) {
        cur_drv = next_drive(cur_drv, sys_drv);
        if (!cur_drv)
            break;
        if (check_hiberfil(cur_drv) && ask_user(cur_drv))
            if (!reset_hiberfil(cur_drv))
                fail_mess();
    };

    free(drives);
    return success;
}

int check_hiberfil(const LPWSTR drive)
{
    char buffer[] = { 0, 0, 0, 0 };
    HANDLE hiber_file_handle;
    DWORD readed;
    int i;

    hiber_file_handle = open_hiber_file(drive, read);

    if (hiber_file_handle == INVALID_HANDLE_VALUE)
        return 0;

    ReadFile(hiber_file_handle, &buffer, sizeof(buffer), &readed, NULL);
    CloseHandle(hiber_file_handle);

    for (i = 0; i < sizeof(buffer); i++)
        if (buffer[i] != gc_hiber_sign[i])
            return 0;

    return 1;
}

int reset_hiberfil(const LPWSTR drive)
{
    DWORD written;
    HANDLE hiber_file_handle;

    hiber_file_handle = open_hiber_file(drive, write);

    if (hiber_file_handle == INVALID_HANDLE_VALUE)
        return 0;

    WriteFile(hiber_file_handle, &gc_null_sign,
        sizeof(gc_null_sign), &written, NULL);
    CloseHandle(hiber_file_handle);

    return written;
}

LPWSTR
open_hiber_file(const LPWSTR drive, enum open_file_mode mode)
{
    LPWSTR hiber_file_path;
    int hiber_file_path_len;
    HANDLE hiber_file_handle;

    DWORD desired_access, share_mode;
    switch (mode) {
    case read:
        desired_access = GENERIC_READ;
        share_mode = FILE_SHARE_READ;
        break;
    case write:
        desired_access = GENERIC_WRITE;
        share_mode = FILE_SHARE_WRITE;
        break;
    default:
        return INVALID_HANDLE_VALUE;
    }

    hiber_file_path_len = _scwprintf(L"%s%s", drive, gc_hiber_file_name);

    hiber_file_path = calloc(hiber_file_path_len + 1, sizeof(WCHAR));
    swprintf(hiber_file_path, hiber_file_path_len + 1,
        L"%s%s", drive, gc_hiber_file_name);

    hiber_file_handle = CreateFileW(hiber_file_path, desired_access, share_mode,
        NULL, OPEN_EXISTING, 0, NULL);

    if (hiber_file_handle == INVALID_HANDLE_VALUE)
        hiber_file_handle = CreateFileW(
            hiber_file_path, desired_access, share_mode, NULL,
            OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

    free(hiber_file_path);
    return hiber_file_handle;
}
