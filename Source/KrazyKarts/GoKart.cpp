// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"
#include "Components/InputComponent.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateRotation(DeltaTime);
	Server_UpdateLocation_Implementation(DeltaTime);
}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::Server_MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGoKart::Server_MoveRight);

}

void AGoKart::Server_MoveForward_Implementation(float Val)
{
	ForwardAxis = Val;
}
bool AGoKart::Server_MoveForward_Validate(float Val)
{
	return FMath::Abs(Val) <= 1;
}

void AGoKart::Server_MoveRight_Implementation(float Val)
{
	RightAxis = Val;
}
bool AGoKart::Server_MoveRight_Validate(float Val)
{
	return FMath::Abs(Val) <= 1;
}

void AGoKart::UpdateRotation(float DeltaTime)
{
	CurTurnSpeed += TurnAccel * DeltaTime * RightAxis;
	if (CurTurnSpeed * RightAxis <= 0) CurTurnSpeed *= (1 - TurnFriction);
	CurTurnSpeed = FMath::Clamp(CurTurnSpeed, -1.0f, 1.0f);
	float TurnAngle = CurSpeed * CurTurnSpeed / TurnRadius;
	FQuat NewRotation(GetActorUpVector(), FMath::DegreesToRadians(TurnAngle));

	AddActorWorldRotation(NewRotation);
}

void AGoKart::Server_UpdateLocation_Implementation(float DeltaTime)
{
	CurSpeed += DeltaTime * Accel * ForwardAxis;
	CurSpeed *= (1.0f - Friction);
	FVector NewSpeed = GetActorForwardVector() * CurSpeed;

	FHitResult OutSweepHitResult;
	AddActorWorldOffset(NewSpeed, true, &OutSweepHitResult);
	if (OutSweepHitResult.IsValidBlockingHit())
	{
		CurSpeed = 0;
	}
}

bool AGoKart::Server_UpdateLocation_Validate(float DeltaTime)
{
	return true;
}