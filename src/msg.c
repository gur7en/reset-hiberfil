#include <string.h>
#include <wchar.h>

#include <windows.h>

/*============================================================================*/

const LPWSTR gc_title = L"Reset Hiberfil";
const LPWSTR gc_msg_fmt = L"Windows на диске %s находится в гибернации.\n"
                          L"Возможна потеря данных при работе с этим диском.\n"
                          L"Сбросить состояние гибернации?";
const LPWSTR gc_msg_fail = L"Не удалось сбросить состояние гибернации.";

/*============================================================================*/

int ask_user(const LPWSTR drive);
void fail_mess();

/*============================================================================*/

int ask_user(const LPWSTR drive)
{

    int mess_len = _scwprintf(gc_msg_fmt, drive);
    LPWSTR message = calloc(mess_len + 1, sizeof(WCHAR));
    swprintf(message, mess_len, gc_msg_fmt, drive);

    int result;
    result = MessageBoxW(NULL, message, gc_title,
        MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON1);

    free(message);

    return (result == IDYES);
};

void fail_mess()
{
    MessageBoxW(NULL, gc_msg_fail, gc_title, MB_OK);
};
