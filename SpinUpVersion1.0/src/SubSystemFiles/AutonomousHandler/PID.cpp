#include "main.h"
#include "vector"
#include "variant"
#include "array"

// Dual PID Settings
double leftError                               = 0;
double rightError                              = 0;
double leftPreviousError                       = 0;
double rightPreviousError                      = 0;
double leftVelocity                            = 0;
double rightVelocity                           = 0;
double leftIntegral                            = 0;
double rightIntegral                           = 0;
double leftDerivative                          = 0;
double rightDerivative                         = 0;
double et_error                                = 0;
double et_prevError                            = 0;
double et_integral                             = 0;
double et_derivative                           = 0;
double c_threshholdCounter                     = 0;
double c_failSafeCounter                       = 0;
double c_kp                                    = 40;
double c_ki                                    = 0;
double c_kd                                    = 0.3;
double c_tkp                                   = 4;


// Combined Turn PID Settings
double te_kp                                   = 2;
double te_ki                                   = 0.002;
double te_kd                                   = 0;

double te_derivative                           = 0;
double te_integral                             = 0;
double te_tolerance                            = 12;
double te_error                                = 0;
double te_previouserror                        = 0;
double te_multiplier                           = 3000;
double te_averageposition                      = 0;
double te_averageHeading                       = 0;
double te_FailSafeCounter                      = 0;
int te_threshholdcounter                       = 0;

// Combined Translation PID Settings
double p_kp                                    = 0.3; // 0.4
double p_ki                                    = 0;
double p_kd                                    = 0.03;

double p_derivative                            = 0;
double p_integral                              = 0;
double pt_tolerance                            = 90;
double p_error                                 = 0;
double p_previouserror                         = 0;
double p_multiplier                            = 200;
double p_maxSpeed                              = 12000;
double p_averageposition                       = 0;
double p_currentposition                       = 0;
double p_averageHeading                        = 0;
double p_FailSafeCounter                       = 0;
int p_threshholdcounter                        = 0;

double wheelDiameter                           = 0;
double ratio                                   = 0;
double cartridge                               = 0;
double circumfrance                            = 0;
double ticks_per_rev                           = 0;
double ticks_per_inches                        = 0;

// Class init
eclipse_PID translationHandler;
eclipse_PID turnHandler;
FinalizeAuton data;
PID pid;

void eclipse_PID::reset_pid_targets() {
  this->e_target = 0;
  this->e_maxSpeed = 0;
  this->e_headingStat = false;
}

void eclipse_PID::reset_pid_inputs() {
  this->e_current = 0;
  this->e_error = 0;
  this->e_prevError = 0;
  this->e_integral = 0;
  this->e_derivative = 0;
  this->e_timer = 0;
}

void eclipse_PID::set_constants(double n_wheelDiameter, double n_gearRatio, double n_motorCartridge){
  wheelDiameter = n_wheelDiameter;
  ratio = n_gearRatio;
  cartridge = n_motorCartridge;
}

void eclipse_PID::set_pid_targets(double kp, double ki, double kd, double rkp) {
  this->e_kp = kp;
  this->e_ki = ki;
  this->e_kd = kd;
  this->e_rkp = rkp;
  c_kp = kp;
  c_ki = ki;
  c_kd = kd;
  c_tkp = rkp;
  p_kp = kp;
  p_ki = ki;
  p_kd = kd;
}

void eclipse_PID::set_turn_pid_targets(double kp, double ki, double kd) {
  this->e_kp = kp;
  this->e_ki = ki;
  this->e_kd = kd;
  te_kp = kp;
  te_ki = ki;
  te_kd = kd;
}

void eclipse_PID::reset_combined_targets(){
  leftError = 0;
  rightError = 0;
  leftPreviousError = 0;
  rightPreviousError = 0;
  leftIntegral = 0;
  rightIntegral = 0;
  leftDerivative = 0;
  rightDerivative = 0;
  c_failSafeCounter = 0;
  c_threshholdCounter = 0;

  et_error = 0;
  et_prevError = 0;
  et_integral = 0;
  et_derivative = 0;

  p_error = 0;
  p_previouserror = 0;
  p_integral = 0;
  p_derivative = 0;
  p_FailSafeCounter = 0;
  p_threshholdcounter = 0;
}

void eclipse_PID::reset_turn_combined_targets(){
  te_error = 0;
  te_previouserror = 0;
  te_integral = 0;
  te_derivative = 0;
  te_FailSafeCounter = 0;
}

int eclipse_PID::find_min_angle(int targetHeading, int currentrobotHeading){
  double turnAngle = targetHeading - currentrobotHeading;
  if (turnAngle > 180 || turnAngle < -180){
    turnAngle = turnAngle - (utility::sgn(turnAngle) * 360);
  }
  std::cout << "Turn Angle " << turnAngle << std::endl;
  return turnAngle;
}

void eclipse_PID::combined_TranslationPID(short int target, short int maxSpeed, short int minSpeed, bool headingStat, bool averagePosStat){
  translationHandler.reset_combined_targets();
  utility::eclipse_fullreset(0, false);
  double targetHeading_G = ImuMon();
  int counter = 0;

  circumfrance = wheelDiameter * M_PI;
  ticks_per_rev = (50.0 * (3600.0 / cartridge) * ratio);
  ticks_per_inches = (ticks_per_rev/ circumfrance);
  target = target * ticks_per_inches;

  while (true){
    SecondOdometry();
    double turnPID = translationHandler.find_min_angle(targetHeading_G, ImuMon()) * 2;
    p_currentposition = (DriveFrontRight.get_position() + DriveFrontLeft.get_position()) / 2; // Getting average position of drivetrain
    p_error = target - (p_currentposition - p_averageposition); // Getting error between distance of target and robot
    p_integral += p_error; // Adding area (integral) between each iteration
    counter++;

    // In case we make it to the setpoint or overshoot the target reset integral since we no longer need the extra power
    if (p_error == 0 || p_error > target) {
      p_integral = 0;
    }

    p_derivative = p_error - p_previouserror; // Calculating the rate of change in error 
    p_previouserror = p_error;

    double voltage = ((p_error * p_kp) + (p_integral * p_ki) + (p_derivative * p_kd)); // Merging all calculations into final voltage power

    if (counter <= 10){
      if (utility::sgn(target) == 1){
        voltage = counter * 2;
      }
      else if (utility::sgn(target) == -1){
        voltage = counter * -2;
      }
    }

    double difference = DriveFrontLeft.get_position() - DriveFrontRight.get_position();
    double compensation = utility::sgn(difference);

    if (voltage > maxSpeed){
      voltage = maxSpeed;
    }
    if (voltage < minSpeed){
      voltage = minSpeed;
    }

    if (headingStat){
      if (counter <= 10){
        utility::leftvelreq(0);
        utility::rightvelreq(0);
      }
      else {
        utility::leftvelreq(voltage + turnPID);
        utility::rightvelreq(voltage - turnPID);
      }
    }
    else{
      utility::leftvelreq(voltage);
      utility::rightvelreq(voltage);
    }

    if(fabs(p_error) < pt_tolerance){
      p_threshholdcounter++;
    }
    else{
      p_threshholdcounter = 0;
    }
    if (p_threshholdcounter > 10){
      utility::stop();
      break;
    }

    if (fabs(p_error - p_previouserror) < 0.3) {
      p_FailSafeCounter++;
    }
    else {
      p_FailSafeCounter = 0;
    }

    if (p_FailSafeCounter >= 1000) {
      utility::stop();
      break;
    }

    data.DisplayData();
    char buffer[300];
    sprintf(buffer, "target heading: %f", targetHeading_G);
    lv_label_set_text(debugLine1, buffer);

    pros::delay(10);

  }
}

void eclipse_PID::combined_TurnPID(double te_theta, double turnSpeed){
  turnHandler.reset_turn_combined_targets();
  utility::fullreset(0, false);
  FinalizeAuton data;
  while(true){ 
    te_error = te_theta - imu_sensor.get_rotation();
    std::cout << "error " << te_error << std::endl;
    te_integral += te_error; 

    if (te_error == 0 || te_error > te_theta) {
      te_integral = 0;
    }
    te_derivative = te_error - te_previouserror;
    
    double voltage = ((te_error * te_kp) + (te_integral * te_ki) + (te_derivative * te_kd)) * 94; 
    if (voltage >= turnSpeed){
      voltage = turnSpeed;
    }
    if (voltage <= -turnSpeed){
      voltage = -turnSpeed;
    }
  

    utility::leftvoltagereq(voltage);
    utility::rightvoltagereq(voltage * -1); 
    if(fabs(te_error) < 12){
      te_threshholdcounter++;
    }
    else{
      te_threshholdcounter = 0;
    }
    if (te_threshholdcounter > 20){
      utility::stop();
      break;
    }
    if (fabs(te_error - te_previouserror) < 0.3) {
      te_FailSafeCounter++;
    }
    else {
      te_FailSafeCounter = 0;
    }
    if (te_FailSafeCounter >= 300) {
      utility::stop();
      break;
    }

    te_previouserror = te_error;
    pros::delay(10);
  }
}

// CONCEPT CODE
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void translationPIDTesting(double target, double maxSpeed){
  utility::fullreset(0, false);
  p_error = 0;
  p_previouserror = 0;
  p_integral = 0;
  p_derivative = 0;
  p_FailSafeCounter = 0;
  p_averageposition = (DriveFrontRight.get_position() + DriveFrontLeft.get_position()) / 2; // Getting average position of drivetrain
  double targetHeading = ImuMon();
  int counter = 0;
  
  while(true){
    double turnPID = translationHandler.find_min_angle(targetHeading, ImuMon()) * 2;
    p_currentposition = (DriveFrontRight.get_position() + DriveFrontLeft.get_position()) / 2; // Getting average position of drivetrain
    p_error = target - (p_currentposition - p_averageposition); // Getting error between distance of target and robot
    p_integral += p_error; // Adding area (integral) between each iteration
    counter++;

    // In case we make it to the setpoint or overshoot the target reset integral since we no longer need the extra power
    if (p_error == 0 || p_error > target) {
      p_integral = 0;
    }

    p_derivative = p_error - p_previouserror; // Calculating the rate of change in error 
    p_previouserror = p_error;

    double voltage = ((p_error * p_kp) + (p_integral * p_ki) + (p_derivative * p_kd)); // Merging all calculations into final voltage power

    if (counter <= 100){
      voltage = counter * 2;
    }

    double difference = DriveFrontLeft.get_position() - DriveFrontRight.get_position();
    double compensation = utility::sgn(difference);

    if (voltage > maxSpeed){
      voltage = maxSpeed;
    }

    utility::leftvelreq(voltage + turnPID); // Making motors move amount in volts
    utility::rightvelreq(voltage - turnPID); // Making motors move amount in volts

    if(fabs(p_error) < pt_tolerance){
      p_threshholdcounter++;
    }
    else{
      p_threshholdcounter = 0;
    }
    if (p_threshholdcounter > 10){
      utility::stop();
      break;
    }

    if (fabs(p_error - p_previouserror) < 0.3) {
      p_FailSafeCounter++;
    }
    else {
      p_FailSafeCounter = 0;
    }

    if (p_FailSafeCounter >= 300) {
      utility::stop();
      break;
    }
    pros::delay(10);

  }
}

const double GTC_kp = 4;
const double GTC_ki = 0;
const double GTC_kd = 2.4;

double GTC_derivative          = 0;
double GTC_integral            = 0;
double GTC_tolerance           = 3;
double GTC_error               = 0;
double GTC_previouserror       = 0;
double GTC_multiplier          = 3000;
double GTC_averageposition     = 0;
double GTC_averageHeading      = 0;
double GTC_FailSafeCounter     = 0;
int GTC_threshholdcounter      = 0;

float Turn_PID_LogicHandler(double GTC_theta){
  GTC_error = 0;
  GTC_previouserror = 0;
  GTC_integral = 0;
  GTC_derivative = 0;
  GTC_FailSafeCounter = 0;

  GTC_averageHeading = imu_sensor.get_rotation(); // Getting average heading of imu
  GTC_error = GTC_theta - GTC_averageHeading; // Getting error between distance of target and robot
  GTC_integral += GTC_error; // Adding area (integral) between each iteration

  // In case we make it to the setpoint or overshoot the target reset integral since we no longer need the extra power
  if (GTC_error == 0 || GTC_error > GTC_theta) {
    GTC_integral = 0;
  }

  GTC_derivative = GTC_error - GTC_previouserror; // Calculating the rate of change in error 
  GTC_previouserror = GTC_error;

  double voltage = (GTC_error * GTC_kp * 0.01); // Merging all calculations into final voltage power

  return voltage;

}

const double kp = 0.03; // 0.4
const double ki = 0;
const double kd = 0.03;

double derivative          = 0;
double integral            = 0;
double tolerance           = 90;
double error               = 0;
double previouserror       = 0;
double multiplier          = 200;
double maxSpeed            = 12000;
double averageposition     = 0;
double currentposition     = 0;
double averageHeading      = 0;
double FailSafeCounter     = 0;
int threshholdcounter      = 0;

// Kind of has correction now? very scuffed, might need to turn kp down later
void PID::TranslationPID(int target, int maxVoltage){

  utility::fullreset(0, false);
  error = 0;
  previouserror = 0;
  integral = 0;
  derivative = 0;
  FailSafeCounter = 0;
  averageposition = (DriveFrontRight.get_position() + DriveFrontLeft.get_position()) / 2; // Getting average position of drivetrain
  double targetHeading = imu_sensor.get_rotation();
  
  while(true){
    SecondOdometry();
    double turnPID = Turn_PID_LogicHandler(targetHeading);
    averageHeading = imu_sensor.get_rotation(); // Getting average heading of imu
    currentposition = (DriveFrontRight.get_position() + DriveFrontLeft.get_position()) / 2; // Getting average position of drivetrain
    error = target - (currentposition - averageposition); // Getting error between distance of target and robot
    integral += error; // Adding area (integral) between each iteration
    //pros::lcd::print(2, "error: %f ", error); // Debugging

    // In case we make it to the setpoint or overshoot the target reset integral since we no longer need the extra power
    if (error == 0 || error > target) {
      integral = 0;
    }

    derivative = error - previouserror; // Calculating the rate of change in error 
    //pros::lcd::print(4, "error - prev: %f ", error - previouserror); // Debugging
    previouserror = error;

    double voltage = ((error * kp) + (integral * ki) + (derivative * kd)) * 94; // Merging all calculations into final voltage power
    //pros::lcd::print(3, "voltage: %f ", voltage); // Debugging

    if (voltage > maxVoltage){
      voltage = maxVoltage;
    }

    double difference = DriveFrontLeft.get_position() - DriveFrontRight.get_position();
    double compensation = utility::sgn(difference);

    utility::leftvelreq(voltage + turnPID); // Making motors move amount in volts
    utility::rightvelreq(voltage - turnPID); // Making motors move amount in volts

    if(fabs(error) < tolerance){
      threshholdcounter++;
    }
    else{
      threshholdcounter = 0;
    }
    if (threshholdcounter > 10){
      utility::stop();
      break;
    }

    if (fabs(error - previouserror) < 0.3) {
      FailSafeCounter++;
    }
    else {
      FailSafeCounter = 0;
    }

    if (FailSafeCounter >= 300) {
      utility::stop();
      break;
    }
    pros::delay(10);

  }
  // pros::lcd::print(6, "Drive PID sequence finished, exiting control.");
}

double ImuMonitorTheta() {
  double theta = imu_sensor.get_rotation();
 
  while (theta < 0) {
    theta += 360;
  }
  while (theta > 360) {
    theta -= 360;
  }
 
  return theta;
 
}

// Only for the GTC function in motionAlg. RETURNS a voltage value only
float PID::Turn_PID(double GTC_theta){

  utility::fullreset(0, false);
  GTC_error = 0;
  GTC_previouserror = 0;
  GTC_integral = 0;
  GTC_derivative = 0;
  GTC_FailSafeCounter = 0;

    SecondOdometry();
    GTC_averageHeading = imu_sensor.get_rotation(); // Getting average heading of imu
    GTC_error = GTC_theta - GTC_averageHeading; // Getting error between distance of target and robot
    GTC_integral += GTC_error; // Adding area (integral) between each iteration

    // In case we make it to the setpoint or overshoot the target reset integral since we no longer need the extra power
    if (GTC_error == 0 || GTC_error > GTC_theta) {
      GTC_integral = 0;
    }

    GTC_derivative = GTC_error - GTC_previouserror; // Calculating the rate of change in error 
    GTC_previouserror = GTC_error;

    double voltage = (GTC_error * GTC_kp * 0.01) * 94; // Merging all calculations into final voltage power
    //pros::lcd::print(3, "voltage: %f ", voltage); // Debugging
    if(fabs(GTC_error) < GTC_tolerance){
      GTC_threshholdcounter++;
    }
    else{
      GTC_threshholdcounter = 0;
    }

    if (fabs(GTC_error - GTC_previouserror) < 0.3) {
      GTC_FailSafeCounter++;
    }
    else {
      GTC_FailSafeCounter = 0;
    }

  return voltage;

}

const double t_kp = 2;
const double t_ki = 0.002;
const double t_kd = 0;

double t_derivative          = 0;
double t_integral            = 0;
double t_tolerance           = 12;
double t_error               = 0;
double t_previouserror       = 0;
double t_multiplier          = 3000;
double t_averageposition     = 0;
double t_averageHeading      = 0;
double t_FailSafeCounter     = 0;
int t_threshholdcounter      = 0;


// When turning only use this one not the above one
void PID::TurnPID(double t_theta){
  utility::fullreset(0, false);
  t_error = 0;
  t_previouserror = 0;
  t_integral = 0;
  t_derivative = 0;
  t_FailSafeCounter = 0;
  while(true){ 
    SecondOdometry();
    t_averageHeading = imu_sensor.get_rotation(); // Getting average heading of imu
    t_error = t_theta - t_averageHeading; // Getting error between distance of target and robot
    t_integral += t_error; // Adding area (integral) between each iteration

    // In case we make it to the setpoint or overshoot the target reset integral since we no longer need the extra power
    if (t_error == 0 || t_error > t_theta) {
      t_integral = 0;
    }

    t_derivative = t_error - t_previouserror; // Calculating the rate of change in error 
    t_previouserror = t_error;

    double voltage = ((t_error * t_kp) + (t_integral * t_ki) + (t_derivative * t_kd)) * 94; // Merging all calculations into final voltage power
    //pros::lcd::print(4, "error: %f ", t_error); // Debugging

    utility::leftvoltagereq(voltage); // Making motors move amount in volts
    utility::rightvoltagereq(voltage * -1); // Making motors move amount in volts

    if(fabs(t_error) < t_tolerance){
      t_threshholdcounter++;
    }
    else{
      t_threshholdcounter = 0;
    }
    if (t_threshholdcounter > 40){
      utility::stop();
      //pros::lcd::print(6, "Broke out: %f "); // Debugging
      break;
    }

    if (fabs(t_error - t_previouserror) < 0.3) {
      t_FailSafeCounter++;
    }
    else {
      t_FailSafeCounter = 0;
    }

    if (t_FailSafeCounter >= 300) {
      utility::stop();
      break;
    }
    pros::delay(10);

  }
  // pros::lcd::print(6, "Turn PID sequence finished, exiting control."); // Acknowledge end

}

// Curve to a point. Current WIP
void PID::ArcPID(double targetX, double targetY){

  double startError = sqrt(pow(targetX - gx, 2) + pow(targetY - gy, 2));

  const double a_kP              = 0.8;
  const double a_driveMultiplier = 95;

  double a_previousTurnAngle = 0;
  double a_distanceError = startError;
  double a_previousError = startError;
  double a_failsafeCheck = 0;

  int a_failsafeCounter = 0;
  int a_breakCounter = 0;

  bool a_turnFixToggle = false;
  bool a_rightTurn = false;
  bool a_switched = false;

  SecondOdometry();
  gy = 0;

  while (true){
    SecondOdometry();
    a_distanceError = sqrt(pow(targetX - gx, 2) + pow(targetY - gy, 2));
    double speed = a_kP * a_distanceError * a_driveMultiplier;
    double modifier = 0.48;

    if (fabs(speed) > 12000)
    {
      speed = 12000;
    }
    if (a_failsafeCounter % 50 == 0){
      if (fabs(a_distanceError) < 0.5){
        // pros::lcd::print(2, "Broke out");
        break;
      }
      a_failsafeCheck = a_distanceError;
    }

    double targetTheta = atan2f(targetX - gx, targetY - gy);
    targetTheta = (targetTheta - ImuMon() * M_PI / 180);
    targetTheta = atan2f(sinf(targetTheta), cosf(targetTheta)) * 180 / M_PI;

    if (targetTheta >= 0){
      a_rightTurn = true; // turn right
    }
    else{
      a_rightTurn = false; // turn left
    }

    if (fabs(targetTheta) < 1.5) // Close enough to theta just drive lmao
    { 
      utility::leftvreq(speed);
      utility::rightvreq(speed);
    }
    else if (a_rightTurn) // Turning right
    {
      utility::leftvreq(speed);
      utility::rightvreq(speed * modifier);
    }
    else // Turning left
    {
      utility::leftvreq(speed * modifier);
      utility::rightvreq(speed);
    }

    if (fabs(a_distanceError) < 8){
      utility::stop();
      break;
    }

    a_failsafeCounter++;
    a_previousError = a_distanceError;
    pros::delay(10);
  }
}