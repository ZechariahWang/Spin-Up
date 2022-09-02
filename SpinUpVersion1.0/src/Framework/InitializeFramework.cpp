#include "main.h"

unsigned short int SelectedAuton = 0;

const unsigned short int MaxLimit = 10;
const unsigned short int MinLimit = 0;

unsigned short int globalAuton = 1;

void Init_AutonSwitchMain::ReceiveInput(long int time){
    int currentTime = 0;

    while (currentTime <= time){
        if (AutonSwitchForward.get_new_press()){
            SelectedAuton++;
            if (SelectedAuton >= MaxLimit){
                SelectedAuton = 0;
            }
            if (SelectedAuton <= MinLimit){
                SelectedAuton = 10;
            }
        }
        else if (AutonSwitchBackward.get_new_press()){
            SelectedAuton--;
            if (SelectedAuton >= MaxLimit){
                SelectedAuton = 0;
            }
            if (SelectedAuton <= MinLimit){
                SelectedAuton = 10;
            }
        }

        pros::lcd::print(4, "Current Auton: %d", SelectedAuton);
        currentTime += 10;
        pros::delay(10);
    }
}

void ResetSensors::ResetAllPrimarySensors(){
    imu_sensor.tare_rotation();
    FrontAux.reset();
	ForwardAux.reset();
}

void FinalizeAuton::SelectAuton(){

    int chosenAuton = SelectedAuton;
    switch (chosenAuton)
    {
    case 0:
        pros::lcd::print(5, "Auton 0 selected");
        globalAuton = 0;
        break;
    case 1:
        pros::lcd::print(5, "Auton 1 selected");
        globalAuton = 1;
        break;
    case 2:
        pros::lcd::print(5, "Auton 2 selected");
        globalAuton = 2;
        break;
    case 3:
        pros::lcd::print(5, "Auton 3 selected");
        globalAuton = 3;
        break;
    case 4:
        pros::lcd::print(5, "Auton 4 selected");
        globalAuton = 4;
        break;
    case 5:
        pros::lcd::print(5, "Auton 5 selected");
        globalAuton = 5;
        break;
    case 6:
        pros::lcd::print(5, "Auton 6 selected");
        globalAuton = 6;
        break;
    case 7:
        pros::lcd::print(5, "Auton 7 selected");
        globalAuton = 7;
        break;
    case 8:
        pros::lcd::print(5, "Auton 8 selected");
        globalAuton = 8;
        break;
    case 9:
        pros::lcd::print(5, "Auton 9 selected");
        globalAuton = 9;
        break;
    case 10:
        pros::lcd::print(5, "Auton 10 selected");
        globalAuton = 10;
        break;
    default:
         pros::lcd::print(5, "Default Auton selected");
         globalAuton = 0;
        break;
    }
}