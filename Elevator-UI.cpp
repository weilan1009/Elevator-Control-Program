void runInteractiveMode(Elevator& elevator) {
    cout << "===== 电梯控制系统 =====" << endl;
    cout << "可用命令:" << endl;
    cout << "U [楼层] - 上行请求" << endl;
    cout << "D [楼层] - 下行请求" << endl;
    cout << "F [楼层] - 内部请求" << endl;
    cout << "S - 查看状态" << endl;
    cout << "Q - 退出" << endl;
    
    string command;
    while (true) {
        cout << "\n> ";
        cin >> command;
        
        if (command == "Q" || command == "q") break;
        
        try {
            if (command == "U" || command == "u") {
                int floor;
                cin >> floor;
                elevator.requestUp(floor);
            } else if (command == "D" || command == "d") {
                int floor;
                cin >> floor;
                elevator.requestDown(floor);
            } else if (command == "F" || command == "f") {
                int floor;
                cin >> floor;
                elevator.requestFloor(floor);
            } else if (command == "S" || command == "s") {
                elevator.getStatus();
            } else {
                cout << "未知命令，请重试" << endl;
            }
        } catch (const exception& e) {
            cout << "错误: " << e.what() << endl;
        }
    }
}
