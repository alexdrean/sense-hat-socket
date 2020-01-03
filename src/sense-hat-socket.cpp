#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/file.h>
#include <sys/stat.h>

#include <signal.h>

#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include "RTIMULib.h"

#define SOCK_FOLDER "/run/sense-hat"
#define SOCK_PATH SOCK_FOLDER "/imu.sock"
#define SOCK_LOCK_PATH SOCK_FOLDER "/imu.sock.lock"

using namespace std;

int acquireLock() {
    // open lock file
    if (mkdir(SOCK_FOLDER, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
        if (errno != EEXIST) {
            cout << "cannot create directory: " << strerror(errno) << endl;
            throw runtime_error(strerror(errno));
        }
    }
    int lock_fd = open(SOCK_LOCK_PATH, O_RDONLY | O_CREAT, 0600);
    if (lock_fd == -1)
        return -1;

// try to acquire lock
    int ret = flock(lock_fd, LOCK_EX | LOCK_NB);
    if (ret != 0)
        return -2;   // the lock is held by another process
    return 0;
}

int check(int res, string errorMessage) {
    if (res < 0) {
        perror(errorMessage.c_str());
        exit(1);
    }
    return res;
}

int createSocket() {
    struct sockaddr_un name;
    int sock = check(socket(AF_UNIX, SOCK_STREAM, 0), "opening unix datagram socket");
    name.sun_family = AF_UNIX;
    strcpy(name.sun_path, SOCK_PATH);

    check(acquireLock(), "could not acquire lock");
    unlink(SOCK_PATH);

    check(bind(sock, (struct sockaddr *) &name, sizeof(struct sockaddr_un)), "binding unix datagram socket");

    check(listen(sock, 5), "listen");
    int flags = check(fcntl(sock, F_GETFL), "could not get flags");
    check(fcntl(sock, F_SETFL, flags | O_NONBLOCK), "could not set flags");
    signal(SIGPIPE, SIG_IGN);
    return sock;
}

RTIMU *initImu() {
    RTIMUSettings *settings = new RTIMUSettings("/etc", "RTIMULib");
    RTIMU *imu = RTIMU::createIMU(settings);
    if ((imu == NULL) || (imu->IMUType() == RTIMU_TYPE_NULL)) {
        printf("No IMU found\n");
        exit(1);
    }

    imu->IMUInit();

    imu->setSlerpPower(0.02);
    imu->setGyroEnable(true);
    imu->setAccelEnable(true);
    imu->setCompassEnable(true);
    return imu;
}

int main() {
    int sock = createSocket();
    RTIMU *imu = initImu();

    vector<int> clients;

    socklen_t socklen = sizeof(struct sockaddr_un);
    struct sockaddr_un* current_client = (sockaddr_un*) malloc(socklen);

    while (1) {
        int connfd;
        do {
            connfd = accept(sock, (struct sockaddr *) (void *) current_client, &socklen);
            if (connfd >= 0) {
                cout << "New client\n";
                current_client = (sockaddr_un *) malloc(socklen);
                clients.push_back(connfd);
            }
        } while (connfd >= 0);
        usleep(imu->IMUGetPollInterval() * 1000);
        while (imu->IMURead()) {
            RTIMU_DATA imuData = imu->getIMUData();
            string result = RTMath::displayDegrees("", imuData.fusionPose);
            result.append("\n");
            for (auto it = clients.begin(); it != clients.end();) {
                if (write(*it, result.c_str(), result.size()) < 0) {
                    it = clients.erase(it);
                    cout << "Lost client\n";
                } else {
                    it++;
                }
            }
        }
    }
}
