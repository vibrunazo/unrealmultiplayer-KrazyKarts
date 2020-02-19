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
	if (HasAuthority())
	{
		NetUpdateFrequency = 1;
	}
}

void AGoKart::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME( AGoKart, ReplicatedTran );
    DOREPLIFETIME( AGoKart, CurSpeed );
    DOREPLIFETIME( AGoKart, CurTurnSpeed );
    DOREPLIFETIME( AGoKart, ForwardAxis );
    DOREPLIFETIME( AGoKart, RightAxis );
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

	if (HasAuthority()) 
	{
		ReplicatedTran = GetActorTransform();
		// ReplicatedSpeed = CurSpeed;
	}
	// FString SpeedString = FString::Printf(TEXT("%F"), ReplicatedSpeed);
	FString SpeedString = FString::Printf(TEXT("%F"), CurSpeed);
	FString TurnString = FString::Printf(TEXT("%F"), CurTurnSpeed);
	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(GetLocalRole()), this, FColor::Green, 0.0f);
	DrawDebugString(GetWorld(), FVector(0, 0, 130), ReplicatedTran.GetLocation().ToString(), this, FColor::Yellow, 0.0f);
	DrawDebugString(GetWorld(), FVector(0, 0, 160), SpeedString, this, FColor::Yellow, 0.0f);
	DrawDebugString(GetWorld(), FVector(0, 0, 190), TurnString, this, FColor::Yellow, 0.0f);
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
	if (FMath::Abs(CurSpeed) < 1.0f) CurSpeed = 0.0f;
	FVector NewSpeed = GetActorForwardVector() * CurSpeed;

	FHitResult OutSweepHitResult;
	AddActorWorldOffset(NewSpeed, true, &OutSweepHitResult);
	if (OutSweepHitResult.IsValidBlockingHit())
	{
		CurSpeed = 0;
	}
}

void AGoKart::OnRep_ReplicatedTran()
{
	UE_LOG(LogTemp, Warning, TEXT("Replicated Transforms! on %s"), *GetEnumText(GetLocalRole()));
	SetActorTransform(ReplicatedTran);
}

// void AGoKart::OnRep_ReplicatedSpeed()
// {
// 	CurSpeed = ReplicatedSpeed;
// 	UE_LOG(LogTemp, Warning, TEXT("Replicated Speed of %d! on %s"), CurSpeed, *GetEnumText(GetLocalRole()));
// }