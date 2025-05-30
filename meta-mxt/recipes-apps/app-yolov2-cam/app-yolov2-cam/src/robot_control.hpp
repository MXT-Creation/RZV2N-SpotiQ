#ifndef ROBOT_CONTROL_HPP
#define ROBOT_CONTROL_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <cstdint> // for uint8_t, uint16_t
//Define single instance of robot control in a singleton way

#include "singleton.hpp"
#include "define.h"
#define sRobotControl  RobotControl::GetInstance()

/************************************************
 * MotorCommand 
 * 
 * Holds motor ID and a desired angle in degrees
 ************************************************/
struct MotorCommand {
    uint8_t motor_id;   // e.g., 0x11, 0x12, etc.
    double  angle_deg;  // store angle in degrees
};

/************************************************
 * RobotControl class
 * 
 * - Manages a worker thread that sends commands via I2C
 * - Provides functions to enqueue commands (angle-based)
 * - Provides a function to split larger moves into steps
 ************************************************/
class RobotControl: public Singleton<RobotControl> {
friend class Singleton<RobotControl>;
public:
    // Constructor / Destructor
    RobotControl();
    ~RobotControl();
	
	void closeUART();
	void clearQueue();
    // Initializes I2C bus if needed (like your Init_I2C() function)
    bool initUART();

    // Start the worker thread that processes the command queue
    void startWorker();

    // Stop the worker thread gracefully
    void stopWorker();

    // Enqueue a single motor command by angle (degrees)
    void enqueueMotorAngle(uint8_t motor_id, double angle_deg);

    // Split a move into small increments and enqueue them
    void moveMotorInSteps(
        uint8_t motor_id,
        double targetAngle
    );
	bool openUART();
private:
    // Helper to push a command into the queue
    void pushCommand(const MotorCommand &cmd);

    // Worker thread function
    void uartWorkerThread();

    // Actually sends a command over I2C (like Send_I2C_Comands)
    bool sendUartCommand(uint8_t motor_id, double angleDegrees);

    // Convert angle (degrees) to a 12-bit servo position or ADC value
    int  degreesToADC(double angle_deg);

	//semd I2c commands
	bool Send_UART_Commands(unsigned char motor_id, position_type position);

    // The queue itself + synchronization
    std::queue<MotorCommand> commandQueue_;
    std::mutex queueMutex_;
	std::mutex UARTBusMutex;
    std::condition_variable queueCV_;
    bool stop_{false};             // signals the worker to exit
    std::atomic<bool> running_{false};

    // The actual worker thread
    std::thread worker_;
};


#endif // ROBOT_CONTROL_HPP