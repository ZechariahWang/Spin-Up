// Motors lolllll

#include "main.h"

extern pros::Motor DriveFrontLeft;
extern pros::Motor DriveFrontRight;
extern pros::Motor DriveBackLeft;
extern pros::Motor DriveBackRight;
extern pros::ADIEncoder FrontAux;
extern pros::ADIEncoder ForwardAux;
extern pros::Imu imu_sensor;
extern pros::Imu imu_sensor_secondary;
extern pros::Vision vision_sensor;
extern pros::Rotation RotationSensor;

extern pros::Motor OuterIntake;
extern pros::Motor InnerIntake;
extern pros::Motor DiskIntake;

extern pros::ADIDigitalOut Launcher;
extern pros::ADIDigitalIn AutonSwitchForward;
extern pros::ADIDigitalIn AutonSwitchBackward;


extern pros::Controller controller;

class Global {

    private:
        int init;

    public:
        double ImuMonitor();

};
