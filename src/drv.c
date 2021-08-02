#include <fileapi.h>
#include <windows.h>

/*============================================================================*/

LPWSTR get_drives();
LPWSTR get_sys_drive();
LPWSTR next_drive(LPWSTR drives, LPWSTR sys_drv);

/*============================================================================*/

LPWSTR get_drives()
{
    LPWSTR buffer;
    DWORD buf_size;
    buf_size = GetLogicalDriveStringsW(0, 0);
    if (buf_size == 0)
        return NULL;

    buf_size *= sizeof(WCHAR);

    buffer = malloc(buf_size + sizeof(WCHAR));
    if (buffer == NULL)
        return NULL;

    if (0 == GetLogicalDriveStringsW(buf_size, buffer))
        return NULL;

    return buffer;
}

LPWSTR get_sys_drive()
{
    LPWSTR sys_drv;
    size_t size;
    _wgetenv_s(&size, NULL, 0, L"SystemDrive");

    if (!size)
        return NULL;

    /* Reserve 1 char for ending "\" */
    sys_drv = calloc(size + 1, sizeof(WCHAR));

    if (sys_drv == NULL)
        return NULL;

    if (_wgetenv_s(&size, sys_drv, size, L"SystemDrive")) {
        free(sys_drv);
        return NULL;
    }

    sys_drv[size - 1] = L'\\';
    sys_drv[size] = L'\0';
    return sys_drv;
}

LPWSTR next_drive(LPWSTR drives, LPWSTR sys_drv)
{
    while (*drives) {
        LPWSTR cur_drv;
        cur_drv = drives;
        drives += wcslen(drives) + 1;

        /* Skip empty disks */
        if (!GetVolumeInformationW(cur_drv, 0, 0, 0, 0, 0, 0, 0))
            continue;

        if (0 == wcscmp(cur_drv, sys_drv))
            continue;

        return cur_drv;
    }

    return 0;
}