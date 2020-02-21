// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"
#include "Components/InputComponent.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

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
    DOREPLIFETIME( AGoKart, ServerState );
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

	FGoKartMove NewMove = CreateMove(DeltaTime);
	if (IsLocallyControlled())
	{
		Server_Move(NewMove);
	}
	if (!HasAuthority())
	{
		SimulateMove(NewMove);
	}
	FString SpeedString = FString::Printf(TEXT("%F"), CurSpeed);
	FString FAxisString = FString::Printf(TEXT("%F"), LastMove.ForwardAxis);
	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(GetLocalRole()), this, FColor::Green, 0.0f);
	DrawDebugString(GetWorld(), FVector(0, 0, 130), ServerState.Transform.GetLocation().ToString(), this, FColor::Yellow, 0.0f);
	DrawDebugString(GetWorld(), FVector(0, 0, 160), SpeedString, this, FColor::Yellow, 0.0f);
	DrawDebugString(GetWorld(), FVector(0, 0, 190), FAxisString, this, FColor::Yellow, 0.0f);
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
	LastMove.ForwardAxis = Val;
}
void AGoKart::MoveRight(float Val)
{
	LastMove.RightAxis = Val;
}

void AGoKart::Server_Move_Implementation(FGoKartMove Move)
{
	// LastMove = Move;
	SimulateMove(Move);
	ServerState.LastMove = Move;
	ServerState.Transform = GetActorTransform();
	ServerState.CurSpeed = CurSpeed;
	ServerState.CurTurnSpeed = CurTurnSpeed;
}
bool AGoKart::Server_Move_Validate(FGoKartMove Move)
{
	return (FMath::Abs(Move.ForwardAxis) <= 1) &&(FMath::Abs(Move.RightAxis) <= 1);
}

void AGoKart::SimulateMove(FGoKartMove Move)
{
	UpdateRotation(Move);
	UpdateLocation(Move);
}

FGoKartMove AGoKart::CreateMove(float DeltaTime)
{
	FGoKartMove Move;
	Move.DeltaTime = DeltaTime;
	Move.Time = GetWorld()->TimeSeconds;
	Move.ForwardAxis = LastMove.ForwardAxis;
	Move.RightAxis = LastMove.RightAxis;
	if (!HasAuthority() && IsLocallyControlled())
	{
		PastMoves.Add(Move);
		UE_LOG(LogTemp, Warning, TEXT("Queue length: %d"), PastMoves.Num());
	}
	return Move;
}

// Keeps only moves that happened after the LastMove in the ServerState
void AGoKart::ClearPastMoves()
{
	PastMoves = PastMoves.FilterByPredicate([&](const FGoKartMove Move){
		return Move.Time > ServerState.LastMove.Time;
	});
	// UE_LOG(LogTemp, Warning, TEXT("Clearing Past: %d, Filter: %d, CurTime: %f,  LastMove: %f"), PastMoves.Num(), Filter.Num(), CurTime, ServerState.LastMove.Time);
}

void AGoKart::UpdateRotation(FGoKartMove Move)
{
	CurTurnSpeed += TurnAccel * Move.DeltaTime * Move.RightAxis;
	if (CurTurnSpeed * Move.RightAxis <= 0) CurTurnSpeed *= (1 - TurnFriction);
	CurTurnSpeed = FMath::Clamp(CurTurnSpeed, -1.0f, 1.0f);
	float TurnAngle = CurSpeed * CurTurnSpeed / TurnRadius;
	FQuat NewRotation(GetActorUpVector(), FMath::DegreesToRadians(TurnAngle));

	AddActorWorldRotation(NewRotation);
}

void AGoKart::UpdateLocation(FGoKartMove Move)
{
	CurSpeed += Move.DeltaTime * Accel * Move.ForwardAxis;
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

void AGoKart::OnRep_ServerState()
{
	UE_LOG(LogTemp, Warning, TEXT("Replicated ServerState! on %s"), *GetEnumText(GetLocalRole()));
	ClearPastMoves();
	SetActorTransform(ServerState.Transform);
	CurSpeed = ServerState.CurSpeed;
	CurTurnSpeed = ServerState.CurTurnSpeed;
	LastMove.ForwardAxis = ServerState.LastMove.ForwardAxis;
	LastMove.RightAxis = ServerState.LastMove.RightAxis;
}

