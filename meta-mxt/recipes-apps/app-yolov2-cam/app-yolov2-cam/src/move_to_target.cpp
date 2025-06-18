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
	sRobotControl->moveMotorInSteps(0x1, 117);
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
	int direction = req.direction;

	    // now interpret them as pixel coordinates (H= x, W=y) if that's how you call it.
    double H = req.x; // or req.y if you prefer
    double W = req.y; // depends how you're passing the detection center

    // 1) Compute angles:
    MotorAngles angles = getInterpolatedAngles(H, W);

	std::cout << "angles1: "<<angles.angle1<<std::endl;
	std::cout << "angles2: "<<angles.angle2<<std::endl;
	std::cout << "angles3: "<<angles.angle3<<std::endl;
	std::cout << "angles4: "<<angles.angle4<<std::endl;


	sRobotControl->moveMotorInSteps(0x01, angles.angle1);

	sRobotControl->moveMotorInSteps(0x04, angles.angle4);

	sRobotControl->moveMotorInSteps(0x03, angles.angle3);

	sRobotControl->moveMotorInSteps(0x02,  angles.angle2);

	//wait 2 seconds
	std::this_thread::sleep_for(std::chrono::seconds(4));

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
//------------------------------------------------------------------------------
// getInterpolatedAngles
//   Takes a pixel center (H, W) from a 1920x1080 image
//   Returns the four motor angles via bilinear interpolation across the corners
//
//   You provided these four corners and angles:
//
//   Corner   (H, W)      angles = (angle1, angle2, angle3, angle4)
//   ---------------------------------------------------------------
//   Bottom-L  542,  860  -> (102, 123, 150, 154)
//   Bottom-R  542, 1215  -> ( 78, 123, 150, 154)
//   Top-L     253,  876  -> ( 99, 154, 114, 138)
//   Top-R     253, 1171  -> ( 81, 154, 114, 138)
//
// Note: The left corners have slightly different W coords (860 vs 876).
//       We'll define a single "W_left" and "W_right" anyway, so this is approximate.
//------------------------------------------------------------------------------
MotorAngles MoveToTarget::getInterpolatedAngles(double H, double W)
{
    // read from global:
    double H_tl = gTopLeft_H.load();
    double W_tl = gTopLeft_W.load();
    double H_tr = gTopRight_H.load();
    double W_tr = gTopRight_W.load();
    double H_bl = gBottomLeft_H.load();
    double W_bl = gBottomLeft_W.load();
    double H_br = gBottomRight_H.load();
    double W_br = gBottomRight_W.load();

		std::cout << "gTopLeft_H" << H_tl<< std::endl;
		std::cout << "gTopLeft_W" << W_tl<< std::endl;
		std::cout << "gTopRight_H" << H_tr  << std::endl;
		std::cout << "gTopRight_W" << W_tr << std::endl;
		std::cout << "gBottomLeft_H" << H_bl << std::endl;
		std::cout << "gBottomLeft_W" << W_bl << std::endl;
		std::cout << "gBottomRight_H" << H_br << std::endl;
		std::cout << "gBottomRight_W" << W_br << std::endl;
    // You also need to define the angles for each corner. If you want them also
    // stored from a file, do the same approach. If they're known constants, keep them as is:
    MotorAngles A_tl { 97, 154, 115, 140 }; 
    MotorAngles A_tr { 81, 154, 115, 140 }; 
    MotorAngles A_bl {102, 123, 150, 150 };
    MotorAngles A_br { 78, 123, 150, 150 };

    // Then unify W_leftFinal etc. 
    // Because  corners might not form a perfect rectangle, do approximate:
    double W_leftFinal  = (W_tl + W_bl) * 0.5; 
    double W_rightFinal = (W_tr + W_br) * 0.5;
    double H_top        = (H_tl + H_tr) * 0.5;
    double H_bottom     = (H_bl + H_br) *0.5;

		std::cout << "W_leftFinal" << W_leftFinal << std::endl;
		std::cout << "W_rightFinal" << W_rightFinal << std::endl;
		std::cout << "H_top" << H_top << std::endl;
		std::cout << "H_bottom" << H_bottom << std::endl;
    // We'll do a short function to "blend" the angles from top-left, top-right.
    auto interpolateAngles = [&](const MotorAngles &L, const MotorAngles &R, double u){
        // linear from L..R by fraction u
        MotorAngles out;
        out.angle1 = (1.0 - u)*L.angle1 + u*R.angle1;
        out.angle2 = (1.0 - u)*L.angle2 + u*R.angle2;
        out.angle3 = (1.0 - u)*L.angle3 + u*R.angle3;
        out.angle4 = (1.0 - u)*L.angle4 + u*R.angle4;
        return out;
    };

    //--------------------------------------------------------------------------
    // 3) Compute normalized coordinates:
    //    v = fraction from top to bottom
    //    u = fraction from left to right

    double v = (H - H_top) / (H_bottom - H_top);
    double u = (W - W_leftFinal) / (W_rightFinal - W_leftFinal);

    // clamp them to [0..1] to avoid going outside corners
    v = std::max(0.0, std::min(1.0, v));
    u = std::max(0.0, std::min(1.0, u));

    //--------------------------------------------------------------------------
    // 4) Interpolate top row angles: top-left => top-right
    MotorAngles topAngles = interpolateAngles(A_tl, A_tr, u);

    //    Interpolate bottom row angles: bottom-left => bottom-right
    MotorAngles bottomAngles = interpolateAngles(A_bl, A_br, u);

    //--------------------------------------------------------------------------
    // 5) Now interpolate vertically between topAngles and bottomAngles by v
    MotorAngles finalAngles = interpolateAngles(topAngles, bottomAngles, v);
		std::cout << "angle1" << finalAngles.angle1 << std::endl;
		std::cout << "angle2" << finalAngles.angle2  << std::endl;
		std::cout << "angle3" << finalAngles.angle3  << std::endl;
		std::cout << "angle4" << finalAngles.angle4  << std::endl;
    return finalAngles;
}