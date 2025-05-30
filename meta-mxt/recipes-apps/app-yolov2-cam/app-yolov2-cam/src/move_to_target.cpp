#include "move_to_target.hpp"
#include "robot_control.hpp" // so we can call sRobotControl->moveMotorInSteps(...)
#include <iostream>
#include <cmath>   // for std::ceil, etc.

MoveToTarget::MoveToTarget()
{
    // constructor body if needed
}

MoveToTarget::~MoveToTarget()
{
    stopWorker();
}

void MoveToTarget::startWorker()
{
    if (!running_) {
        running_ = true;
        worker_ = std::thread(&MoveToTarget::workerThread, this);
    }
}

void MoveToTarget::stopWorker()
{
    if (running_) {
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            stop_ = true;
        }
        queueCV_.notify_all();
        running_ = false;

        if (worker_.joinable()) {
            worker_.join();
        }
    }
}

void MoveToTarget::enqueueTarget(double x, double y, int direction)
{
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        requestQueue_.push({ x, y,direction });
    }
    queueCV_.notify_one();
}

void MoveToTarget::workerThread()
{
    while (true) {
        TargetRequest req;
        {
            // Wait until we have a request or we're stopping
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCV_.wait(lock, [this] {
                return !requestQueue_.empty() || stop_;
            });

            if (stop_ && requestQueue_.empty()) {
                break;
            }

            req = requestQueue_.front();
            requestQueue_.pop();
        }

        // Now handle the request
        handleRequest(req);
    }

    std::cout << "[MoveToTarget] workerThread exiting." << std::endl;
}
bool MoveToTarget::initUART()
{
if(!sRobotControl->openUART())
{
return false;
}
std::this_thread::sleep_for(std::chrono::milliseconds(200));
robot_is_moving.store(true);
//init uart motor positions to default values
sRobotControl->moveMotorInSteps(0x01,92);
std::this_thread::sleep_for(std::chrono::milliseconds(200));
sRobotControl->moveMotorInSteps(0x02,50);
std::this_thread::sleep_for(std::chrono::milliseconds(200));
sRobotControl->moveMotorInSteps(0x03,150);
std::this_thread::sleep_for(std::chrono::milliseconds(200));
sRobotControl->moveMotorInSteps(0x05,48);
std::this_thread::sleep_for(std::chrono::milliseconds(200));
sRobotControl->moveMotorInSteps(0x06,20);
std::this_thread::sleep_for(std::chrono::milliseconds(200));
sRobotControl->moveMotorInSteps(0x04,215);
std::this_thread::sleep_for(std::chrono::seconds(2));
	robot_is_moving.store(false);
	return true;
}
double MoveToTarget::calculateAngleTheta2Corection(double x) {
    // Known data points
    //double height1 = 500, d1 = 20.0;  // distance = 20cm
    //double height2 = 370, d2 = 27.0;  // distance = 30cm
    double d1 = 20, corectedd1 = 15.0;  // distance = 20cm
    double d2 = 27, correctedd2 = 0;  // distance = 30cm

    // Linear interpolation formula for height
    double newAngle = corectedd1 + ((correctedd2 - corectedd1) / (d2 - d1)) * (x - d1);
return newAngle;
}
double MoveToTarget::calculateAngleTheta3Corection(double x) {
    // Known data points
    //double height1 = 500, d1 = 20.0;  // distance = 20cm
    //double height2 = 370, d2 = 27.0;  // distance = 30cm
    double d1 = 20, corectedd1 = 0;  // distance = 20cm
    double d2 = 27, correctedd2 = 5;  // distance = 30cm

    // Linear interpolation formula for height
    double newAngle = corectedd1 + ((correctedd2 - corectedd1) / (d2 - d1)) * (x - d1);
return newAngle;
}
/*****************************************
* Function Name: dotProduct
* Description   : Function to calculate the dot product between two vectors.
* Arguments     : Ax, Ay - x and y components of the first vector.
*                 Bx, By - x and y components of the second vector.
* Return value  : The scalar (dot) product of the vectors.
*****************************************/
double MoveToTarget::dotProduct(double Ax, double Ay, double Bx, double By) {
    return Ax * Bx + Ay * By;
}

/*****************************************
* Function Name: magnitude
* Description   : Function to calculate the magnitude (length) of a vector.
* Arguments     : x, y - the x and y components of the vector.
* Return value  : The magnitude (length) of the vector.
*****************************************/
double MoveToTarget::magnitude(double x, double y) {
    return std::sqrt(x * x + y * y);
}


bool MoveToTarget::moveGreen()
{
    // drop angles for each motor the drop the cube
    double dropAngle12 = 145; // drop angle the leave the cube Motor 2
    double dropAngle13 = 125; //drop angle the leave the cube Motor 3
    double dropAngle14 = 140; // drop angle the leave the cube Motor 4
	//rotate to right
	sRobotControl->moveMotorInSteps(0x1, 65);
    sRobotControl->moveMotorInSteps(0x4, dropAngle14);
    sRobotControl->moveMotorInSteps(0x3, dropAngle13);
    sRobotControl->moveMotorInSteps(0x2, dropAngle12);
	return true;
}
bool MoveToTarget::moveYellow()
{
    // drop angles for each motor the drop the cube
    double dropAngle12 = 105; // drop angle the leave the cube Motor 2
    double dropAngle13 = 180; //drop angle the leave the cube Motor 3
    double dropAngle14 = 145; // drop angle the leave the cube Motor 4
//rotate to right
	sRobotControl->moveMotorInSteps(0x1, 50);
    sRobotControl->moveMotorInSteps(0x4, dropAngle14);
    sRobotControl->moveMotorInSteps(0x3, dropAngle13);
    sRobotControl->moveMotorInSteps(0x2, dropAngle12);
	return true;
}
bool MoveToTarget::moveBlue()
{
    // drop angles for each motor the drop the cube
    double dropAngle12 = 145; // drop angle the leave the cube Motor 2
    double dropAngle13 = 125; //drop angle the leave the cube Motor 3
    double dropAngle14 = 140; // drop angle the leave the cube Motor 4
//rotate to left
	sRobotControl->moveMotorInSteps(0x1, 120);
    sRobotControl->moveMotorInSteps(0x4, dropAngle14);
    sRobotControl->moveMotorInSteps(0x3, dropAngle13);
    sRobotControl->moveMotorInSteps(0x2, dropAngle12);
	return true;
}
bool MoveToTarget::moveRed()
{
    // drop angles for each motor the drop the cube
    double dropAngle12 = 105; // drop angle the leave the cube Motor 2
    double dropAngle13 = 180; //drop angle the leave the cube Motor 3
    double dropAngle14 = 145; // drop angle the leave the cube Motor 4
	//rotate to left
	sRobotControl->moveMotorInSteps(0x1, 136);
    sRobotControl->moveMotorInSteps(0x4, dropAngle14);
    sRobotControl->moveMotorInSteps(0x3, dropAngle13);
    sRobotControl->moveMotorInSteps(0x2, dropAngle12);
return true;
}


/*****************************************
* Function Name: calculateAngle
* Description   : Function to calculate the angle between two segments defined by three points.
* Arguments     : x1, y1 - coordinates of the first point of the first segment.
*                 x2, y2 - coordinates of the second point of the first segment (also the first point of the second segment).
*                 x3, y3 - coordinates of the third point of the second segment.
* Return value  : The angle in radians between the first and second segments.
*****************************************/
double MoveToTarget::calculateAngle(double x1, double y1, double x2, double y2, double x3, double y3) {
    // Calculate directional vectors for each segment
    double Ax = x2 - x1;
    double Ay = y2 - y1;
    double Bx = x3 - x2;
    double By = y3 - y2;

    // Calculate the perpendicular vector to vector A
    double ApX = Ay;
    double ApY = -Ax;

    // Calculate the dot product and magnitudes
    double dot = dotProduct(ApX, ApY, Bx, By);
    double magAp = magnitude(ApX, ApY);
    double magB = magnitude(Bx, By);

    // Calculate the cosine of the angle between the two vectors
    double cosTheta = dot / (magAp * magB);

    // Ensure the cosine value is within the valid range [-1, 1] due to precision errors
    if (cosTheta > 1) cosTheta = 1;
    if (cosTheta < -1) cosTheta = -1;

    // Calculate the angle in radians
    double angle = std::acos(cosTheta);
    return angle;
}
void MoveToTarget::handleRequest(const TargetRequest &req)
{
 //store globaly that the robot is moving
	//robot_is_moving = true;
	double x = req.x;
	double y = req.y;
	int direction = req.direction;

    // Initialize coordinates for the arm segments and motors
    double x1, x2, x3, x4;
    double y0, y1, y2, y3, y4;
    x1 = x2 = x3 = x4 = 0; // Initializing x axis positions of each point to 0
    y1 = y2 = y3 = x4 = 0; // Initializing y axis positions of each point to 0

    // Set initial positions for the first motor
    x1 = 0;
    y1 = 12; // Position of the first motor

    // Initial angles for the motors in upright position
    double theta1 = 38; // Initial angle for Motor 1
    double theta2 = -20; // Initial angle for Motor 2
	double correctedTheta2,correctedTheta3;
    double theta3 = -68; // Initial angle for Motor 3

    // Convert angles from degrees to radians
    double radTheta1 = theta1 * M_PI / 180.0;
    double radTheta2 = theta2 * M_PI / 180.0;
    double radTheta3 = theta3 * M_PI / 180.0;

    std::cout << "radTheta1 = " << radTheta1 << std::endl;
    std::cout << "radTheta2 = " << radTheta2 << std::endl;
    std::cout << "radTheta3 = " << radTheta3 << std::endl;

    // Define the limits for motor angles (in radians)
    double maxradTheta1 = 1.5708; // 90 degrees
    double minradTheta1 = 0.5236; // 30 degrees
    double maxradTheta2 = 0.7854; // 45 degrees
    double minradTheta2 = -0.7854; // -45 degrees
    double maxradTheta3 = 0; // 0 degrees
    double minradTheta3 = -1.5708; // -90 degrees

    bool conditionMeet = false; // Condition to check if target is reached
    // Iterate through possible angles for the motors
    for (radTheta1 = minradTheta1; (radTheta1 <= maxradTheta1); radTheta1 += 0.001) {
        // Calculate the position of point 2 based on motor angle
        x2 = x1 + LENGHT1 * cos(radTheta1);
        y2 = y1 + LENGHT1 * sin(radTheta1);

        for (radTheta2 = minradTheta2; (radTheta2 <= maxradTheta2); radTheta2 += 0.001) {
            // Calculate the position of point 3 based on motor angle
            x3 = x2 + LENGHT2 * cos(radTheta2);
            y3 = y2 + LENGHT2 * sin(radTheta2);

            for (radTheta3 = minradTheta3; (radTheta3 <= maxradTheta3); radTheta3 += 0.001) {
                // Calculate the position of point 4 based on motor angle
                x4 = x3 + LENGHT3 * cos(radTheta3);
                y4 = y3 + LENGHT3 * sin(radTheta3);

                // Calculate the distance to the target
                double distance = std::sqrt((x4 - x) * (x4 - x) + (y4 - y) * (y4 - y));

                // Check if the distance is within the acceptable range
                if ((distance < 2) && (y4 >= y) && (x4 <= x)&& (x4 >= (x-1))) {
                    conditionMeet = true;
                    std::cout << "distance = " << distance << std::endl;
                    std::cout << "x4 = " << x4 << std::endl;
                    std::cout << "y4 = " << y4 << std::endl;
                break;  // Exit innermost loop
            }
        }
        if (conditionMeet) break;  // Exit second loop
    }
    if (conditionMeet) break;  // Exit outermost loop
    }

    if (!conditionMeet) {
        //do nothing; and remove lock
		std::cout << "could not calculate angles to distance"<< std::endl;
		robot_is_moving.store(false);
    }
	else
{
    // Output the adjusted angles
    std::cout << " theta1: " << theta1 << " and in radians: " << radTheta1 << std::endl;
    std::cout << " theta2: " << theta2 << " and in radians: " << radTheta2 << std::endl;
    std::cout << " theta3: " << theta3 << " and in radians: " << radTheta3 << std::endl;
    
    // Calculate the angles between the segments
    radTheta2 = calculateAngle(x1, y1, x2, y2, x3, y3);
    radTheta3 = calculateAngle(x2, y2, x3, y3, x4, y4);

    // Convert the angles back to degrees
    theta1 = 180-(radTheta1 * 180.0 / M_PI);
    theta2 = 180-(radTheta2 * 180.0 / M_PI);
    theta3 = 180-(radTheta3 * 180.0 / M_PI);
	
	//apply angle corection
	correctedTheta2 = theta2 + calculateAngleTheta2Corection(x);
	correctedTheta3 = theta3 + calculateAngleTheta3Corection(x);
	//sRobotControl->startWorker();
	sRobotControl->moveMotorInSteps(0x04, correctedTheta3);
	std::cout << "posDegres_14 moving to the following angle"<< correctedTheta3<<"correction from :"<<theta3<<std::endl;

	sRobotControl->moveMotorInSteps(0x03, correctedTheta2);
	std::cout << "posDegres_13  moving to the following angle"<< correctedTheta2<<"correction from :"<<theta2<<std::endl;

	sRobotControl->moveMotorInSteps(0x02,  theta1);
	std::cout << "posDegres_12  moving to the following angle"<< theta1<<std::endl;

	//wait 2 seconds
	std::this_thread::sleep_for(std::chrono::seconds(3));

    //Close the gripper
    sRobotControl->moveMotorInSteps(0x06, 145);

	//wait 4 second
    std::this_thread::sleep_for(std::chrono::seconds(2));  // if needed

    //Move back to initial angles
    double initAngle14 = 215;
    double initAngle13 = 150;
    double initAngle12 = 50;

	//move to init position
    sRobotControl->moveMotorInSteps(0x02,  initAngle12);
    sRobotControl->moveMotorInSteps(0x03,  initAngle13);
    sRobotControl->moveMotorInSteps(0x04,  initAngle14);
	std::cout << "move to init position"<<std::endl;
	//wait 4 second
    //std::this_thread::sleep_for(std::chrono::seconds(5));
		switch(direction)
	{
		case 1:
		{
		moveBlue();
std::cout << "move to blue position"<<std::endl;
		break;
		}
		case 2:
		{
		moveRed();
std::cout << "move to red position"<<std::endl;
		break;
		}
		case 3:
		{
		moveGreen();
std::cout << "move to green position"<<std::endl;
		break;
		}
		case 4:
		{
		moveYellow();
std::cout << "move to yellow position"<<std::endl;
		break;
		}
		default:
		{
			//do nothing
		}
	}
	//wait 7 seconds
	std::this_thread::sleep_for(std::chrono::seconds(7));
	//open the gripper
    sRobotControl->moveMotorInSteps(0x06, 20);
std::cout << "open grip"<<std::endl;
	//move back to init positon
    sRobotControl->moveMotorInSteps(0x02, initAngle12);
    sRobotControl->moveMotorInSteps(0x03, initAngle13);
    sRobotControl->moveMotorInSteps(0x04,  initAngle14);
	sRobotControl->moveMotorInSteps(0x01, 92);
std::cout << "move to init position"<<std::endl;
	//wait 1 seconds before he can detect again cubes
	std::this_thread::sleep_for(std::chrono::seconds(2));
	robot_is_moving.store(false);
}
}