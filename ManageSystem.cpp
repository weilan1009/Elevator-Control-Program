class ElevatorSystem {
private:
    vector<Elevator> elevators;
    
public:
    ElevatorSystem(int numElevators, int totalFloors) {
        for (int i = 0; i < numElevators; ++i) {
            elevators.emplace_back(totalFloors);
        }
    }
    
    void assignRequest(int floor, Direction direction) {
        // 简单调度算法：选择最近的空闲电梯
        int bestElevator = -1;
        int minDistance = INT_MAX;
        
        for (int i = 0; i < elevators.size(); ++i) {
            // 获取电梯状态（需要在Elevator类中添加接口）
            int elevatorFloor = elevators[i].getCurrentFloor();
            Direction elevatorDir = elevators[i].getDirection();
            
            // 计算距离并选择最优电梯
            int distance = abs(elevatorFloor - floor);
            if (distance < minDistance && 
                (elevatorDir == Direction::IDLE || elevatorDir == direction)) {
                minDistance = distance;
                bestElevator = i;
            }
        }
        
        if (bestElevator != -1) {
            if (direction == Direction::UP) {
                elevators[bestElevator].requestUp(floor);
            } else {
                elevators[bestElevator].requestDown(floor);
            }
        }
    }
};
