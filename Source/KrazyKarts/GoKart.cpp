// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"
#include "Components/InputComponent.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGoKart::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME( AGoKart, ReplicatedTran );
}

FString GetEnumText(ENetRole Role)
{
	switch (Role)
	{
	case ROLE_None:
		return "None";
	case ROLE_SimulatedProxy:
		return "SimulatedProxy";
	case ROLE_AutonomousProxy:
		return "AutonomousProxy";
	case ROLE_Authority:
		return "Authority";
	default:
		return "ERROR";
	}
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateRotation(DeltaTime);
	UpdateLocation(DeltaTime);

	if (GetLocalRole() == ROLE_Authority) 
	{
		ReplicatedTran = GetActorTransform();
	}
	else
	{
		SetActorTransform(ReplicatedTran);
	}

	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(GetLocalRole()), this, FColor::Green, 0.0f);
	DrawDebugString(GetWorld(), FVector(0, 0, 130), ReplicatedTran.GetLocation().ToString(), this, FColor::Yellow, 0.0f);
}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGoKart::MoveRight);
}

void AGoKart::MoveForward(float Val)
{
	ForwardAxis = Val;
	Server_MoveForward(Val);
}
void AGoKart::MoveRight(float Val)
{
	RightAxis = Val;
	Server_MoveRight(Val);
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

void AGoKart::UpdateLocation(float DeltaTime)
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
