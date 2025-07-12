#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

using namespace std;

// 电梯方向枚举
enum class Direction {
    UP, DOWN, IDLE
};

// 电梯类 - 核心控制逻辑
class Elevator {
private:
    int currentFloor;                // 当前楼层
    Direction direction;             // 当前方向
    vector<bool> upRequests;         // 上行请求表
    vector<bool> downRequests;       // 下行请求表
    vector<bool> internalRequests;   // 内部按钮请求
    mutex mtx;                       // 互斥锁
    condition_variable cv;           // 条件变量
    bool running;                    // 运行状态

    // 处理单个请求
    void processRequest() {
        while (running) {
            unique_lock<mutex> lock(mtx);
            
            // 如果没有请求，电梯待机
            if (!hasRequests()) {
                direction = Direction::IDLE;
                cv.wait(lock, [this] { return hasRequests() || !running; });
            }
            
            if (!running) break;
            
            // 根据当前方向处理请求
            if (direction == Direction::UP) {
                handleUpRequests();
            } else if (direction == Direction::DOWN) {
                handleDownRequests();
            } else {
                // 待机状态，决定下一个方向
                decideNextDirection();
            }
            
            lock.unlock();
            // 模拟电梯移动时间
            this_thread::sleep_for(chrono::seconds(1));
        }
    }

    // 检查是否有未处理的请求
    bool hasRequests() const {
        for (bool req : upRequests) if (req) return true;
        for (bool req : downRequests) if (req) return true;
        for (bool req : internalRequests) if (req) return true;
        return false;
    }

    // 处理上行请求
    void handleUpRequests() {
        // 检查内部请求
        for (int i = currentFloor + 1; i < internalRequests.size(); ++i) {
            if (internalRequests[i]) {
                moveToFloor(i);
                internalRequests[i] = false;
                return;
            }
        }
        
        // 检查上行外部请求
        for (int i = currentFloor + 1; i < upRequests.size(); ++i) {
            if (upRequests[i]) {
                moveToFloor(i);
                upRequests[i] = false;
                return;
            }
        }
        
        // 检查下行外部请求（折返点）
        for (int i = upRequests.size() - 1; i > currentFloor; --i) {
            if (downRequests[i]) {
                direction = Direction::DOWN;
                moveToFloor(i);
                downRequests[i] = false;
                return;
            }
        }
        
        // 如果没有更高楼层的请求，改变方向
        direction = Direction::DOWN;
    }

    // 处理下行请求
    void handleDownRequests() {
        // 检查内部请求
        for (int i = currentFloor - 1; i >= 0; --i) {
            if (internalRequests[i]) {
                moveToFloor(i);
                internalRequests[i] = false;
                return;
            }
        }
        
        // 检查下行外部请求
        for (int i = currentFloor - 1; i >= 0; --i) {
            if (downRequests[i]) {
                moveToFloor(i);
                downRequests[i] = false;
                return;
            }
        }
        
        // 检查上行外部请求（折返点）
        for (int i = 0; i < currentFloor; ++i) {
            if (upRequests[i]) {
                direction = Direction::UP;
                moveToFloor(i);
                upRequests[i] = false;
                return;
            }
        }
        
        // 如果没有更低楼层的请求，改变方向
        direction = Direction::UP;
    }

    // 决定下一个方向
    void decideNextDirection() {
        // 查找最近的上行请求
        int closestUp = -1;
        for (int i = currentFloor; i < upRequests.size(); ++i) {
            if (upRequests[i] || internalRequests[i]) {
                closestUp = i;
                break;
            }
        }
        
        // 查找最近的下行请求
        int closestDown = -1;
        for (int i = currentFloor; i >= 0; --i) {
            if (downRequests[i] || internalRequests[i]) {
                closestDown = i;
                break;
            }
        }
        
        // 选择更近的请求方向
        if (closestUp != -1 && closestDown != -1) {
            if ((closestUp - currentFloor) <= (currentFloor - closestDown)) {
                direction = Direction::UP;
                moveToFloor(closestUp);
                if (upRequests[closestUp]) upRequests[closestUp] = false;
                if (internalRequests[closestUp]) internalRequests[closestUp] = false;
            } else {
                direction = Direction::DOWN;
                moveToFloor(closestDown);
                if (downRequests[closestDown]) downRequests[closestDown] = false;
                if (internalRequests[closestDown]) internalRequests[closestDown] = false;
            }
        } else if (closestUp != -1) {
            direction = Direction::UP;
            moveToFloor(closestUp);
            if (upRequests[closestUp]) upRequests[closestUp] = false;
            if (internalRequests[closestUp]) internalRequests[closestUp] = false;
        } else if (closestDown != -1) {
            direction = Direction::DOWN;
            moveToFloor(closestDown);
            if (downRequests[closestDown]) downRequests[closestDown] = false;
            if (internalRequests[closestDown]) internalRequests[closestDown] = false;
        }
    }

    // 移动电梯到指定楼层
    void moveToFloor(int floor) {
        cout << "电梯从 " << currentFloor << " 层移动到 " << floor << " 层" << endl;
        currentFloor = floor;
        cout << "电梯已到达 " << currentFloor << " 层" << endl;
    }

public:
    // 构造函数 - 指定楼层数
    Elevator(int totalFloors) : 
        currentFloor(1), 
        direction(Direction::IDLE),
        upRequests(totalFloors + 1, false),
        downRequests(totalFloors + 1, false),
        internalRequests(totalFloors + 1, false),
        running(true) {
        // 启动电梯线程
        thread(&Elevator::processRequest, this).detach();
    }

    // 析构函数
    ~Elevator() {
        {
            lock_guard<mutex> lock(mtx);
            running = false;
        }
        cv.notify_one();
    }

    // 外部请求电梯（上行）
    void requestUp(int floor) {
        if (floor < 1 || floor >= upRequests.size()) {
            cout << "错误：无效的楼层号 " << floor << endl;
            return;
        }
        
        {
            lock_guard<mutex> lock(mtx);
            upRequests[floor] = true;
            cout << "收到上行请求：" << floor << " 层" << endl;
        }
        cv.notify_one();
    }

    // 外部请求电梯（下行）
    void requestDown(int floor) {
        if (floor < 1 || floor >= downRequests.size()) {
            cout << "错误：无效的楼层号 " << floor << endl;
            return;
        }
        
        {
            lock_guard<mutex> lock(mtx);
            downRequests[floor] = true;
            cout << "收到下行请求：" << floor << " 层" << endl;
        }
        cv.notify_one();
    }

    // 内部按钮请求
    void requestFloor(int floor) {
        if (floor < 1 || floor >= internalRequests.size()) {
            cout << "错误：无效的楼层号 " << floor << endl;
            return;
        }
        
        {
            lock_guard<mutex> lock(mtx);
            internalRequests[floor] = true;
            cout << "收到内部请求：前往 " << floor << " 层" << endl;
        }
        cv.notify_one();
    }

    // 获取当前电梯状态
    void getStatus() const {
        lock_guard<mutex> lock(mtx);
        cout << "电梯当前状态：" << currentFloor << " 层，方向：";
        switch(direction) {
            case Direction::UP: cout << "上行" << endl; break;
            case Direction::DOWN: cout << "下行" << endl; break;
            case Direction::IDLE: cout << "待机" << endl; break;
        }
    }
};

// 示例使用
int main() {
    Elevator elevator(10);  // 创建一个10层楼的电梯
    
    // 模拟请求
    elevator.requestUp(3);    // 3楼有人按上行按钮
    this_thread::sleep_for(chrono::seconds(2));
    
    elevator.requestDown(7);  // 7楼有人按下行按钮
    this_thread::sleep_for(chrono::seconds(2));
    
    elevator.requestFloor(5); // 电梯内有人按下5楼按钮
    this_thread::sleep_for(chrono::seconds(2));
    
    elevator.getStatus();     // 查看电梯当前状态
    
    // 程序会在所有请求处理完后继续运行，直到手动终止
    return 0;
}
