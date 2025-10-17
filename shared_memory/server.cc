// 以 writer.cpp 为例，reader.cpp 同理增加检查
#include <iostream>
#include <cstring>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <errno.h>  // 用于打印错误码

const int SHM_SIZE = 1024;
const key_t SHM_KEY = 0x1234;  // 可尝试修改为其他值（如 0x5678）避免冲突

int main() {
    int shmid;
    char* shm_addr;

    // 1. 创建共享内存，增加错误码打印
    shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        std::cerr << "shmget 失败！错误码：" << errno 
                  << "（可能原因：权限不足/键值冲突）" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 2. 映射共享内存，检查返回值
    shm_addr = static_cast<char*>(shmat(shmid, nullptr, 0));
    if (shm_addr == reinterpret_cast<char*>(-1)) {
        std::cerr << "shmat 失败！错误码：" << errno 
                  << "（可能原因：内存映射失败）" << std::endl;
        // 失败时先删除已创建的共享内存，避免残留
        shmctl(shmid, IPC_RMID, nullptr);
        exit(EXIT_FAILURE);
    }

    // 3. 写入数据（确保不越界）
    const char* msg = "i am process A";
    if (strlen(msg) >= SHM_SIZE) {  // 检查数据长度，避免越界
        std::cerr << "数据过长，超过共享内存大小！" << std::endl;
        shmdt(shm_addr);
        shmctl(shmid, IPC_RMID, nullptr);
        exit(EXIT_FAILURE);
    }
    std::strcpy(shm_addr, msg);
    std::cout << "进程A：已写入数据" << std::endl;

    // 4. 解除映射
    if (shmdt(shm_addr) == -1) {
        std::cerr << "shmdt 失败！错误码：" << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    return 0;
}
