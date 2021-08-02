#include <fileapi.h>
#include <windows.h>

/*============================================================================*/

enum states_privileges {
    enable = SE_PRIVILEGE_ENABLED,
    disable = 0
};

/*============================================================================*/

int get_backup_privileges(enum states_privileges state);
int set_priv_data(TOKEN_PRIVILEGES* tknpriv, int index,
    LPCWSTR priv_name, enum states_privileges state);
int get_priv(TOKEN_PRIVILEGES* tknpriv, size_t tknpriv_size);

/*============================================================================*/

int get_backup_privileges(enum states_privileges state)
{
    int result = 0;
    TOKEN_PRIVILEGES* tknpriv;
    int tknpriv_size;

    /* For 2 privileges */
    tknpriv_size = sizeof(TOKEN_PRIVILEGES) + sizeof(LUID_AND_ATTRIBUTES);
    tknpriv = malloc(tknpriv_size);
    if (tknpriv == NULL)
        return 0;

    tknpriv->PrivilegeCount = 2;

    if (set_priv_data(tknpriv, 0, SE_BACKUP_NAME, enable) && set_priv_data(tknpriv, 1, SE_RESTORE_NAME, enable) && get_priv(tknpriv, tknpriv_size))
        result = 1;

    free(tknpriv);
    return result;
}

int set_priv_data(TOKEN_PRIVILEGES* tknpriv, int index,
    LPCWSTR priv_name, enum states_privileges state)
{
    LUID_AND_ATTRIBUTES* laa = &tknpriv->Privileges[index];
    if (!LookupPrivilegeValueW(NULL, priv_name, &laa->Luid))
        return 0;

    laa->Attributes = state;

    return 1;
}

int get_priv(TOKEN_PRIVILEGES* tknpriv, size_t tknpriv_size)
{
    int result = 0;
    HANDLE process;
    HANDLE tknproc;

    process = GetCurrentProcess();
    if (OpenProcessToken(process, TOKEN_ADJUST_PRIVILEGES, &tknproc)) {
        AdjustTokenPrivileges(tknproc, 0, tknpriv, tknpriv_size, NULL, NULL);
        result = (GetLastError() == ERROR_SUCCESS);
        CloseHandle(tknproc);
    };

    CloseHandle(process);
    return result;
}
