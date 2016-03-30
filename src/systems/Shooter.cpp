/*
 * Shooter.cpp
 *
 *  Created on: Mar 7, 2016
 *      Author: 1750800404
 */

#include "Shooter.h"

Shooter::Shooter(){
	raiseShoot = new CANTalon(4); //7
	raiseShoot->SetFeedbackDevice(CANTalon::CtreMagEncoder_Absolute);
	UpLimit = new DigitalInput(4);
	DownLimit = new DigitalInput(5);
	ballSense = new DigitalInput(0); //3
	shootSol = new Solenoid(1, 0);
	lShooter = new CANTalon(1); //5
	rShooter = new CANTalon (2); //6
	lRPMSensor = new DigitalInput(1);
	rRPMSensor = new DigitalInput(2);
	picker = new CANTalon(3); //8
	rpmTimerL = new Timer();
	rpmTimerR = new Timer();
}

Shooter::~Shooter(){
	delete raiseShoot;
	delete UpLimit;
	delete DownLimit;
	delete ballSense;
	delete shootSol;
	delete lShooter;
	delete rShooter;
	delete lRPMSensor;
	delete rRPMSensor;
	delete picker;
	delete rpmTimerL;
	delete rpmTimerR;
}

//Detect if the ball is in the cradle
bool Shooter::DetectBall(){
	bool ball;
	if(ballSense->Get()){
		ball = true;
	}
	else{
		ball = false;
	}
	return ball;
}

//Pickup the ball and stop when a ball is detected
void Shooter::Pickup(float speed){
	if(DetectBall() == false){
		picker->Set(speed);
		lShooter->Set(-(speed/rotorPickupDivisor));
		rShooter->Set(speed/rotorPickupDivisor);
	}
	else{
		picker->Set(0.0);
		lShooter->Set(0.0);
		rShooter->Set(0.0);
	}
	DetectBall();
}

//Pickup the ball if there's a sensor failure
void Shooter::PickupNoSensors(float speed){
	picker->Set(speed);
	lShooter->Set(-(speed/rotorPickupDivisor));
	rShooter->Set(speed/rotorPickupDivisor);
}

//Reset the raising and lowering encoder manually
void Shooter::ResetRaisePositionManual(){
	raiseShoot->SetPosition(0);
}

//Raise manually, will stop at top limit
void Shooter::Raise(float speed){
	raiseShoot->StopMotor();
	if(raiseShoot->GetControlMode() == CANSpeedController::kPercentVbus){
		if(UpLimit->Get() == false || raiseShoot->GetEncPosition() >= minShooterPos){
			raiseShoot->Set(-speed);
		}
		else{
			raiseShoot->Set(0.0);
			ResetRaisePositionManual();
		}
	}
	else{
		raiseShoot->SetControlMode(CANSpeedController::kPercentVbus);
		raiseShoot->EnableControl();
		if(UpLimit->Get() == false || raiseShoot->GetEncPosition() >= minShooterPos){
			raiseShoot->Set(-speed);
		}
		else{
			raiseShoot->Set(0.0);
			ResetRaisePositionManual();
		}
	}
}

//Raise manually when there's a sensor failure
void Shooter::RaiseNoSensors(float speed){
		raiseShoot->Set(-speed);
}

//Lower manually, will stop at top limit
void Shooter::Lower(float speed){
	raiseShoot->StopMotor();
	if(raiseShoot->GetControlMode() == CANSpeedController::kPercentVbus){
		if(DownLimit->Get() == false || raiseShoot->GetPosition() <= maxShooterPos){
			raiseShoot->Set(speed);
		}
		else{
			raiseShoot->Set(0.0);
		}
	}
	else{
		raiseShoot->SetControlMode(CANSpeedController::kPercentVbus);
		raiseShoot->EnableControl();
		if(DownLimit->Get() == false || raiseShoot->GetPosition() <= maxShooterPos){
			raiseShoot->Set(speed);
		}
		else{
			raiseShoot->Set(0.0);
		}
	}
}

//Lower manually when there's a sensor failure
void Shooter::LowerNoSensors(float speed){
		raiseShoot->Set(speed);
}

//Aim for the high goal from far
void Shooter::BombShotAim(){
	/*if(raiseShoot->GetEncPosition() > (bombEnco + shooterAimTolerance)){
		while(raiseShoot->GetEncPosition() > (bombEnco + shooterAimTolerance)){
			raiseShoot->Set(PMotorPower(raiseShoot->GetEncPosition(), bombEnco, raiseShooterP, speed, -speed));
		}
		raiseShoot->StopMotor();
	}
	else if (raiseShoot->GetEncPosition() < bombEnco){
		while(raiseShoot->GetEncPosition() < bombEnco){
			raiseShoot->Set(PMotorPower(raiseShoot->GetEncPosition(), bombEnco, raiseShooterP, speed, -speed));
		}
		raiseShoot->StopMotor();
	}
	else{
		raiseShoot->StopMotor();
	}*/
	raiseShoot->StopMotor();
	if(raiseShoot->GetControlMode() == CANSpeedController::kPosition){
		raiseShoot->Set(bombPos);
	}
	else{
		raiseShoot->SetControlMode(CANSpeedController::kPosition);
		raiseShoot->SetPID(shootPosP, shootPosI, shootPosD);
		raiseShoot->EnableControl();
		raiseShoot->Set(bombPos);
	}
}

//Aim for the high goal from close
void Shooter::TurretShotAim(){
	/*if(raiseShoot->GetEncPosition() > (turretEnco + shooterAimTolerance)){
		while(raiseShoot->GetEncPosition() > (turretEnco + shooterAimTolerance)){
			raiseShoot->Set(PMotorPower(raiseShoot->GetEncPosition(), turretEnco, raiseShooterP, speed, -speed));
		}
		raiseShoot->StopMotor();
	}
	else if (raiseShoot->GetEncPosition() < turretEnco){
		while(raiseShoot->GetEncPosition() < turretEnco){
			raiseShoot->Set(PMotorPower(raiseShoot->GetEncPosition(), turretEnco, raiseShooterP, speed, -speed));
		}
		raiseShoot->StopMotor();
	}
	else{
		raiseShoot->StopMotor();
	}*/
	raiseShoot->StopMotor();
	if(raiseShoot->GetControlMode() == CANSpeedController::kPosition){
		raiseShoot->Set(turretPos);
	}
	else{
		raiseShoot->SetControlMode(CANSpeedController::kPosition);
		raiseShoot->SetPID(shootPosP, shootPosI, shootPosD);
		raiseShoot->EnableControl();
		raiseShoot->Set(turretPos);
	}
}

//Aim for the low goal
void Shooter::LowGoalAim(float speed){
	if(DownLimit->Get()){
		while(DownLimit->Get()){
			raiseShoot->Set(speed);
		}
		raiseShoot->Set(0.0);
	}
	else{
		raiseShoot->Set(0.0);
	}
}

void Shooter::CustomAim(double pos){
	/*if(encoVal < maxShooterEnco && encoVal > minShooterEnco){
		if(raiseShoot->GetEncPosition() > (encoVal+shooterAimTolerance)){
			while(raiseShoot->GetEncPosition() > (encoVal+shooterAimTolerance)){
				raiseShoot->Set(PMotorPower(raiseShoot->GetEncPosition(), (encoVal+shooterAimTolerance), raiseShooterP, speed, -speed));
			}
			raiseShoot->Set(0.0);
		}
		else if (raiseShoot->GetEncPosition() < encoVal){
			while(raiseShoot->GetEncPosition() < encoVal){
				raiseShoot->Set(PMotorPower(raiseShoot->GetEncPosition(), encoVal, raiseShooterP, speed, -speed));
			}
			raiseShoot->Set(0.0);
		}
		else{
			raiseShoot->Set(0.0);
		}
	}
	else{
		raiseShoot->Set(0.0);
	}*/
	raiseShoot->StopMotor();
	if(raiseShoot->GetControlMode() == CANSpeedController::kPosition){
		raiseShoot->Set(pos);
	}
	else{
		raiseShoot->SetControlMode(CANSpeedController::kPosition);
		raiseShoot->SetPID(shootPosP, shootPosI, shootPosD);
		raiseShoot->EnableControl();
		raiseShoot->Set(pos);
	}
}

float Shooter::ReadRPM(DigitalInput *banner, Timer *time){
	float rpmReading;
	bool bannerToggle = true;
	int reads = 0;
	time->Reset();
	time->Start();
	while(time->Get() <= 0.05){
		if(banner->Get() == false && bannerToggle){
			bannerToggle = false;
			reads++;
		}
		else if(banner->Get()){
			bannerToggle = true;
		} //Test this
	}
	time->Stop();
	rpmReading = reads * 400; //1200
	return rpmReading;
}

void Shooter::Shoot(int leftRPM, int rightRPM, float rollPow){
	lShooter->Set(0.1);
	rShooter->Set(-0.1);
	picker->Set(rollPow);
	if(leftRPM > 2400){
		leftRPM = 2400;
	}
	if(rightRPM > 2400){
		rightRPM = 2400;
	}
	for(int lPow = 0.1; ReadRPM(lRPMSensor, rpmTimerL) < (leftRPM - 200) || lPow <= 1.0; lPow += 0.1){
		lShooter->Set(lPow);
	}
	for(int rPow = 0.1; ReadRPM(rRPMSensor, rpmTimerR) < (rightRPM - 200) || rPow <= 1.0; rPow -= 0.1){
		rShooter->Set(-rPow);
	}
	shootSol->Set(true);
	Wait(1.0);
	shootSol->Set(false);
	lShooter->Set(0.0);
	rShooter->Set(0.0);
	picker->Set(0.0);
}

//Shoot when there's a sensor failure
void Shooter::ShootNoSensors(float leftPow, float rightPow, float rollPow){
	lShooter->Set(leftPow);
	rShooter->Set(-rightPow);
	picker->Set(-rollPow);
	Wait(shooterRampTime);
	shootSol->Set(true);
	Wait(shooterWaitTime);
	shootSol->Set(false);
	lShooter->Set(0.0);
	rShooter->Set(0.0);
	picker->Set(0.0);
}

