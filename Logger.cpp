lass Logger {
private:
    ofstream logFile;
    
public:
    Logger(const string& filename) {
        logFile.open(filename, ios::app);
        if (!logFile.is_open()) {
            cerr << "无法打开日志文件" << endl;
        }
    }
    
    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }
    
    void log(const string& message) {
        time_t now = time(0);
        string timestamp = ctime(&now);
        timestamp.pop_back();  // 移除换行符
        
        if (logFile.is_open()) {
            logFile << "[" << timestamp << "] " << message << endl;
        }
        
        // 同时输出到控制台
        cout << "[" << timestamp << "] " << message << endl;
    }
};
