#ifndef MOVE_TO_TARGET_HPP
#define MOVE_TO_TARGET_HPP

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "singleton.hpp"
#include "define.h"
#define sMoveToTarget  MoveToTarget::GetInstance()

/**
 * A simple struct representing a (x,y) target request.
 */
struct TargetRequest {
    double x;
    double y;
int direction; //1 means move left; 2 means move right
};
// A simple struct to hold the four motor angles.
struct MotorAngles {
    double angle1; // e.g. Motor 1
    double angle2; // e.g. Motor 2
    double angle3; // e.g. Motor 3
    double angle4; // e.g. Motor 4
};
/**
 * @brief MoveToTarget Class
 *
 * This class runs in its own thread. You enqueue (x,y) targets, and it
 * computes the angles (e.g., inverse kinematics) and calls into
 * RobotControl (sRobotControl) to move each motor in small steps.
 */
class MoveToTarget: public Singleton<MoveToTarget> {
friend class Singleton<MoveToTarget>;
public:
    MoveToTarget();
    ~MoveToTarget();

    // Start the worker thread
    void startWorker();

    // Stop the worker thread
    void stopWorker();

    // Enqueue a request to move the arm to (x, y)
    void enqueueTarget(double x, double y, int direction);
	bool initUART();
private:

	MotorAngles getInterpolatedAngles(double H, double W);

    // The worker thread function
    void workerThread();

    // Some internal method that does the actual IK and calls RobotControl
    void handleRequest(const TargetRequest &req);

	double dotProduct(double Ax, double Ay, double Bx, double By);

	double magnitude(double x, double y) ;

	double calculateAngle(double x1, double y1, double x2, double y2, double x3, double y3);
	double calculateAngleTheta2Corection(double x) ;
	double calculateAngleTheta3Corection(double x) ;
	
	bool moveGreen();
	bool moveYellow();
	bool moveBlue();
	bool moveRed();
    // Thread & synchronization
    std::thread worker_;
    std::mutex queueMutex_;
    std::condition_variable queueCV_;

    // A queue of TargetRequests
    std::queue<TargetRequest> requestQueue_;

    // Control flags
    bool stop_{false};
    std::atomic<bool> running_{false};
};

#endif // MOVE_TO_TARGET_HPP