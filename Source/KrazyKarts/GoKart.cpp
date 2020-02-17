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

	// FVector NewLoc = GetActorLocation();
	// SetActorRelativeLocation();
	CurSpeed += DeltaTime * Accel * ForwardAxis;
	CurSpeed *= (1.0f - Friction);
	FVector NewSpeed = GetActorForwardVector() * CurSpeed;
	AddActorWorldOffset(NewSpeed);

}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);

}

void AGoKart::MoveForward(float Val)
{
	// UE_LOG(LogTemp, Warning, TEXT("move forward by %f"), Val);
	ForwardAxis = Val;
}