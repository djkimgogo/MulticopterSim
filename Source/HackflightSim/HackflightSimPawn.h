/*
* HackflightSimPawn.h: Class declaration for pawn class in HackflightSim
*
* Copyright (C) 2018 Simon D. Levy
*
* MIT License
*/

#pragma once

// Math support
#define _USE_MATH_DEFINES
#include <math.h>

#include <hackflight.hpp>
using namespace hf;

#include "ThreadedSocketServer.h"

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Runtime/Engine/Classes/Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "HackflightSimPawn.generated.h"

UCLASS(Config=Game)
// Override both APawn and Hackflight::Board to simplify the API
class AHackflightSimPawn : public APawn, public Board
{
    private:

        GENERATED_BODY()

        // StaticMesh component that will be the visuals for our flying pawn 
        UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
        class UStaticMeshComponent* PlaneMesh;

        // Propeller meshes for spinning
        class UStaticMeshComponent* PropMeshes[4];

        // Audio support: see http://bendemott.blogspot.com/2016/10/unreal-4-playing-sound-from-c-with.html
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
            class USoundCue* propellerAudioCue;
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
            class USoundCue* propellerStartupCue;
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
            class UAudioComponent* propellerAudioComponent;

        // FPV camera support
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
            class UCameraComponent* fpvCamera;
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
            class USpringArmComponent* fpvSpringArm;

        // Support for spinning propellers
        const int8_t motordirs[4] = {+1, -1, -1, +1};
        float _motorvals[4];

        // Support for quaternions
        FQuat _quat;

        // Support for sensor emulation
        FVector _eulerPrev;
        FVector _gyro;
		float _accelZ;
		float _varioPrev;
		float _groundAltitude;
		float _elapsedTime;

        // Converts a set of motor values to angular forces in body frame
        float motorsToAngularForce(int a, int b, int c, int d);

        // Supports MSP over socket
        void serverError(void);
        bool _serverRunning;
		int _serverAvailableBytes;
		int _serverByteIndex;
		char _serverBuffer[ThreadedSocketServer::BUFLEN];

		// Helpers
		float getAltitude(void);
		FVector getEulerAngles(void);
        
        // Helps us simulate sensor noise.  XXX We should simulate ODR (output data rates) as well, but 
        // UE4 frame rate is currently to slow to do that realistically.
        class Sensor {

            public:

                Sensor(uint8_t size, float noise);

                void addNoise(float vals[]);

            private:

                uint8_t _size;
                float   _noise;
        };

        Sensor _gyroSensor  = Sensor(3, 0.01);
        Sensor _accelSensor = Sensor(3, 0.01);
        Sensor _quatSensor  = Sensor(4, 0.01);
        Sensor _rangeSensor = Sensor(1, 0.01);
        Sensor _flowSensor  = Sensor(2, 0.01);
        Sensor _baroSensor  = Sensor(1, 0.01);

    public:

        AHackflightSimPawn();

        // AActor overrides
        virtual void BeginPlay() override;
        virtual void PostInitializeComponents() override;
        virtual void Tick(float DeltaSeconds) override;
        virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, 
                bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
        virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

        // Hackflight::Board overrides
        virtual bool		getQuaternion(float quat[4]) override;
        virtual bool		getGyrometer(float gyroRates[3]) override;
        virtual bool		getAccelerometer(float accelGs[3]) override;
        virtual void		writeMotor(uint8_t index, float value) override;
        virtual uint8_t		serialAvailableBytes(void) override;
        virtual uint8_t		serialReadByte(void) override;
        virtual void		serialWriteByte(uint8_t c) override;
        virtual bool		getBarometer(float & pressure) override;
        virtual bool		getOpticalFlow(float & x, float & y) override;
        virtual bool		getRangefinder(float & distance) override;
        virtual uint32_t	getMicroseconds(void) override;

        // Returns PlaneMesh subobject 
        FORCEINLINE class UStaticMeshComponent* GetPlaneMesh() const { return PlaneMesh; }
};
