#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

int main(int argc, char **argv)
{
    // 在大多数场合，我们仅用到成员bdaddr，它标识了设备的蓝牙地址。
    // 有些场合我们也会用到成员dev_class， 它标识了被检测到的蓝牙设备的一些信息
    // (例如，识别这个设备是打印设备，电话，个人电脑等)，详细地对应关系可以参见蓝牙设备分配号
    inquiry_info *ii = NULL;
    int max_rsp, num_rsp;
    int dev_id, sock, len, flags;
    int i;
    char addr[19] = { 0 };
    char name[248] = { 0 };
    // 通常的话系统只有一个蓝牙适配器，把参数NULL传给hci_get_route可以获得第一个有效的蓝牙适配器识别号
    dev_id = hci_get_route(NULL);
    // hci_open_dev打开的套接字建立了一条和本地蓝牙适配器控制器的连接，而不是和远端蓝牙设备的连接。
    // 使用这个套接口发送命令到蓝牙控制器可以实现底层的蓝牙操作
    sock = hci_open_dev( dev_id );
    printf("%d - %d \n", dev_id, sock);
    if (dev_id < 0 || sock < 0) {
        perror("opening socket");
        exit(1);
    }

    len = 8;
    max_rsp = 255;
    // 如果标志位flag设置为IREQ_CACHE_FLUSH，那么在进行查询操作时会把先前一次查询记录的cache刷新，
    // 否则flag设置为0的话，即便先前查询的设备已经不处于有效范围内，先前查询的记录也将被返回
    flags = IREQ_CACHE_FLUSH;
    ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));

    // 选择好本地蓝牙适配器并进行系统资源分配后，程序就可以开始扫描周边的蓝牙设备了，
    // 在这个例程中，hci_inquiry函数完成对蓝牙设备的搜寻，并将返回的设备信息数据记录在变量ii中
    num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
    if( num_rsp < 0 ) perror("hci_inquiry");

    for (i = 0; i < num_rsp; i++) {
        ba2str(&(ii+i)->bdaddr, addr);
        memset(name, 0, sizeof(name));
        // hci_read_remote_name函数在规定的超时时间内使用套接口通过蓝牙地址ba去获取蓝牙设备的名称，
        // 成功返回0，并将获取的蓝牙设备名称存入name中；失败时返回-1并设置相应的errno
        if (hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name),
                                 name, 0) < 0)
            strcpy(name, "[unknown]");
        printf("%s %s \n", addr, name);
    }

    free( ii );
    close( sock );
    return 0;
}
