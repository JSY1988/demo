#include "IPLocator.hpp"
using namespace std;
extern string GetTime()
{
    time_t Timep;
    time(&Timep);
    char Tmp[64];
    strftime(Tmp, sizeof(Tmp), "%Y-%m-%d %H:%M:%S", localtime(&Timep));
    return Tmp;
}
int main()
{
    IPLocator ipl("QQWry.dat");
    PMIB_TCPTABLE_OWNER_PID pTcpTable;
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;
    char szLocalAddr[128];
    char szRemoteAddr[128];
    struct in_addr IpAddr;
    int i = 0;
    pTcpTable = (MIB_TCPTABLE_OWNER_PID *)MALLOC(sizeof(MIB_TCPTABLE_OWNER_PID));
    if (pTcpTable == NULL)
    {
        return 1;
    }
    dwSize = sizeof(MIB_TCPTABLE_OWNER_PID);
    if ((dwRetVal = GetExtendedTcpTable(pTcpTable, &dwSize, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0)) == ERROR_INSUFFICIENT_BUFFER)
    {
        FREE(pTcpTable);
        pTcpTable = (MIB_TCPTABLE_OWNER_PID *)MALLOC(dwSize);
        if (pTcpTable == NULL)
        {
            return 1;
        }
    }
    if ((dwRetVal = GetExtendedTcpTable(pTcpTable, &dwSize, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0)) == NO_ERROR)
    {

        for (i = 0; i < (int)pTcpTable->dwNumEntries; i++)
        {
            HANDLE Handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pTcpTable->table[i].dwOwningPid);
            if (Handle)
            {
                TCHAR Buffer[MAX_PATH];
                if (GetModuleFileNameEx(Handle, 0, Buffer, MAX_PATH))
                {
                    IpAddr.S_un.S_addr = (u_long)pTcpTable->table[i].dwLocalAddr;
                    strcpy_s(szLocalAddr, sizeof(szLocalAddr), inet_ntoa(IpAddr));
                    IpAddr.S_un.S_addr = (u_long)pTcpTable->table[i].dwRemoteAddr;
                    strcpy_s(szRemoteAddr, sizeof(szRemoteAddr), inet_ntoa(IpAddr));
                    string time = GetTime();
                    cout << "ʱ��    : " << time << endl;
                    cout << "���� PID: " << pTcpTable->table[i].dwOwningPid << endl;
                    printf("��������: %s\n", Buffer);
                    printf("����  IP: %s\n", szLocalAddr);
                    printf("���ض˿�: %d\n", ntohs((u_short)pTcpTable->table[i].dwLocalPort));
                    printf("Զ��  IP: %s\n", szRemoteAddr);
                    printf("Զ�̶˿�: %d\n", ntohs((u_short)pTcpTable->table[i].dwRemotePort));
                    cout << "Զ�̵�ַ: " << ipl.getIpAddr(szRemoteAddr) << std::endl;
                    printf("\n");
                }
            }
        }
    }
    else
    {
        FREE(pTcpTable);
        return 1;
    }
    system("pause");
    return 0;
}