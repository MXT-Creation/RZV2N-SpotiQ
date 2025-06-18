#include "robot_control.hpp"
#include <iostream>
#include <cmath>       // for round, etc.
 #include <fcntl.h>   // if needed for open()
 #include <unistd.h> // if needed
 #include <sys/ioctl.h>  // if needed
 #include <linux/i2c-dev.h>  // if needed
 #include <i2c/smbus.h> // if needed
#include <termios.h>



// If you have separate includes or definitions for motor IDs (MOTOR1=0x11, etc.),
// place them here or in a shared header like "define.h" or "robot_defines.h".

int uart_fd = -1;

/************************************************
 * Constructor / Destructor
 ************************************************/
RobotControl::RobotControl()
{
    // Optionally do more setup here
}

RobotControl::~RobotControl()
{
    stopWorker();  // ensure the worker is stopped before destruct
}

bool RobotControl::openUART()
{
	    // 1) Open the UART device
	uart_fd = open("/dev/ttySC1", O_RDWR | O_NOCTTY);
    if (uart_fd < 0) {
        std::cerr << "Failed to open UART\n";
        return false;
    }
    // 2) Configure the UART if needed (like 'stty -F /dev/ttySC1 115200 cs8 -cstopb -parenb')
    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    if (tcgetattr(uart_fd, &tty) != 0) {
        std::cerr << "Failed to get attr\n";
        close(uart_fd);
        return false;
    }
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);
    tty.c_cflag &= (~CREAD);
    tty.c_cflag |= (CLOCAL);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB; // no parity
    tty.c_cflag &= ~CSTOPB; // 1 stop bit
    tty.c_cflag &= ~CRTSCTS; // no hw flow control
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_oflag &= ~OPOST;
    tcsetattr(uart_fd, TCSANOW, &tty);
return true;
}

void RobotControl::closeUART() {
    // Close and reopen UART port by sending a dummy command or re-initializing
    std::lock_guard<std::mutex> lock(UARTBusMutex);
        tcflush(uart_fd, TCIOFLUSH);  // Clear buffers
		uart_fd =-1;
        close(uart_fd);
}

void RobotControl::clearQueue()
{
    std::lock_guard<std::mutex> lock(queueMutex_);
    std::queue<MotorCommand> empty;
    std::swap(commandQueue_, empty);
}
/************************************************
 * initI2C()
 * 
 * Similar to your existing Init_I2C() function
 ************************************************/
bool RobotControl::initUART()
{
std::this_thread::sleep_for(std::chrono::milliseconds(200));
robot_is_moving.store(true);
//init uart motor positions to default values
enqueueMotorAngle(0x01,92);
std::this_thread::sleep_for(std::chrono::milliseconds(200));
enqueueMotorAngle(0x02,50);
std::this_thread::sleep_for(std::chrono::milliseconds(200));
enqueueMotorAngle(0x03,150);
std::this_thread::sleep_for(std::chrono::milliseconds(200));
enqueueMotorAngle(0x05,48);
std::this_thread::sleep_for(std::chrono::milliseconds(200));
enqueueMotorAngle(0x06,20);
std::this_thread::sleep_for(std::chrono::milliseconds(200));
enqueueMotorAngle(0x04,215);
std::this_thread::sleep_for(std::chrono::seconds(2));
	robot_is_moving.store(false);
	return true;
}

void RobotControl::pushCommand(const MotorCommand &cmd)
{
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        commandQueue_.push(cmd);
    }
    queueCV_.notify_one();
}

/************************************************
 * enqueueMotorAngle(motor_id, angle_deg)
 * 
 * Simple function: creates a MotorCommand and pushes it
 ************************************************/
void RobotControl::enqueueMotorAngle(uint8_t motor_id, double angle_deg)
{
    MotorCommand cmd;
    cmd.motor_id  = motor_id;
    cmd.angle_deg = angle_deg;
    pushCommand(cmd);
}

/************************************************
 * moveMotorInSteps()
 * 
 * Splits a move from currentAngle -> targetAngle
 * into small increments, enqueues them all
 ************************************************/
void RobotControl::moveMotorInSteps(
    uint8_t motor_id,
    double targetAngle
)
{
//    double diff = targetAngle - currentAngle;
//    int steps   = static_cast<int>(std::ceil(std::fabs(diff) / stepSizeDeg));
//    double sign = (diff >= 0.0) ? 1.0 : -1.0;
//    double angle = currentAngle;

//    for (int i = 0; i < steps; i++) {
//        angle += (sign * stepSizeDeg);
//        // clamp overshoot
//        if ((sign > 0.0 && angle > targetAngle) ||
//            (sign < 0.0 && angle < targetAngle))
//        {
//            angle = targetAngle;
//        }
//        enqueueMotorAngle(motor_id, angle);
//    }
	enqueueMotorAngle(motor_id, targetAngle);

}
/************************************************
 * startWorker()
 * 
 * Spawns the worker thread that processes commands
 ************************************************/
void RobotControl::startWorker()
{
    if (!running_) {
        running_ = true;
        worker_ = std::thread(&RobotControl::uartWorkerThread, this);
    }
}

/************************************************
 * stopWorker()
 * 
 * Instructs the worker to stop and joins the thread
 ************************************************/
void RobotControl::stopWorker()
{
    if (running_) {
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            stop_ = true;
        }
        queueCV_.notify_all();  // wake the thread
        running_ = false;

        if (worker_.joinable()) {
            worker_.join();
        }
    }
}

/************************************************
 * uartWorkerThread()
 * 
 * The background loop that pops commands and
 * calls sendUartCommand()
 ************************************************/
void RobotControl::uartWorkerThread()
{
    while (true) {
        MotorCommand cmd;
        {
            // Wait for a command or stop
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCV_.wait(lock, [this] {
                return !commandQueue_.empty() || stop_;
            });

            if (stop_ && commandQueue_.empty()) {
                break; // no more commands, time to exit
            }

            // Get the next command
            cmd = commandQueue_.front();
            commandQueue_.pop();
        }

        // Process the command
        sendUartCommand(cmd.motor_id, cmd.angle_deg);
        // small sleep between commands:
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::cout << "[RobotControl] uartWorkerThread exiting." << std::endl;
}

/************************************************
 * degreesToADC(angle_deg)
 * 
 * Example function for mapping angle -> servo ADC
 ************************************************/
int RobotControl::degreesToADC(double angle_deg)
{
    // Example formula from your code:
    //    (2200.0 / 180.0) * angle + 900.0
    // plus clamping 0..4095 if 12-bit.
    double slope = 2200.0 / 180.0;
    double val   = slope * angle_deg + 900.0;

    if (val < 0) val = 0;
    if (val > 4095) val = 4095;

    return static_cast<int>(std::round(val));
}

bool RobotControl::sendUartCommand(uint8_t motor_id, double angleDegrees)
{
    // 1) Convert degrees to ADC, then to position_type
    int adcVal = degreesToADC(angleDegrees);

    position_type pos;
    pos.positon_16Bits = static_cast<uint16_t>(adcVal);

	switch(motor_id)
{
case 0x11:
{
	posDegres_11 = angleDegrees;
break;
}
case 0x12:
{
	posDegres_12 = angleDegrees;
break;
}
case 0x13:
{
	posDegres_13 = angleDegrees;
break;
}
case 0x14:
{
	posDegres_14 = angleDegrees;
break;
}
case 0x15:
{
	posDegres_15 = angleDegrees;
break;
}
case 0x16:
{
	posDegres_16 = angleDegrees;
break;
}
default:
//do nothing
break;
}
// Convert ADC value for Motor 1 into 2 bytes in pos
pos.position_8Bits[1] = static_cast<uint8_t>(adcVal & 0x00FF);   // LSB
pos.position_8Bits[0] = static_cast<uint8_t>((adcVal >> 8) & 0xFF); // MSB

    // 2) Insert your existing code that does I2C writes here:
    bool success = Send_UART_Commands(motor_id, pos);
     if (!success) {
std::cerr << "[ERROR] UART write failed for motor 0x"
          << std::hex << (int)motor_id << std::dec
          << ", angle=" << angleDegrees << std::endl;
         return false;
    }
	//wait 500 miliseconds
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return true;
}

/*****************************************
*Function Name: Send Send_UART_Commands Comands
 * Description   : Function to send I2C commands to the robotic arm
 * Arguments     : motor_id (0x11,0x12,0x13,0x14,0x15,0x16) The id of the motor from the robotic arm 
						  position . The value of the 
 * Return value  : 1 if succeeded
 *                       0 otherwise
*****************************************/
bool RobotControl::Send_UART_Commands(unsigned char motor_id, position_type position)
{
// Acquire the lock here; it is automatically released when the function returns
    std::lock_guard<std::mutex> lock(UARTBusMutex);

    uint8_t speedLSB;
    uint8_t speedMSB;
	if(motor_id == 0x06)
	{
	speedLSB = 0x03;   // speed
    speedMSB = 0xE8;   // speed
	}
else
	{
    speedLSB = 0x07;   // speed
    speedMSB = 0xD0;   // speed
	}

    uint8_t posMSB = position.position_8Bits[1];   // angle
    uint8_t posLSB = position.position_8Bits[0];   // angle

    std::vector<uint8_t> outbuf;
    outbuf.reserve(11); // number of bytes
    outbuf.push_back(0xFF);
    outbuf.push_back(0xFF);
    outbuf.push_back(motor_id);
    outbuf.push_back(0x07);  // HARDCODDED
    outbuf.push_back(0x03);  // HARDCODDED
    outbuf.push_back(0x2A);  // HARDCODDED
    outbuf.push_back(posLSB); // angle
    outbuf.push_back(posMSB);  // angle
    outbuf.push_back(speedLSB);  // speed
    outbuf.push_back(speedMSB);  // speed

// 5) Compute checksum: sum from motor_id onward, then invert lower 8 bits
uint16_t sum16 = 0;

// We'll start from index 2 because 0 & 1 are 0xFF 0xFF, 
// which (in your protocol) are not included in the sum
for (size_t i = 2; i < outbuf.size(); i++) {
    sum16 += outbuf[i];
}


// Lower 8 bits of sum16
uint8_t sum8 = static_cast<uint8_t>(sum16 & 0xFF);


// Invert that byte (bitwise NOT)
uint8_t crc = static_cast<uint8_t>(~sum8);


// Append that CRC byte to the vector
outbuf.push_back(crc);
//check if uart port is open
if(uart_fd<0)
{
return false;
}
	//write the data
    ssize_t written = write(uart_fd, outbuf.data(), outbuf.size());

  if (written != (ssize_t)outbuf.size()) {
    std::cerr << "Incomplete UART write\n";
    //close(uart_fd);
    return false;
}
	tcdrain(uart_fd);
    return true;
}