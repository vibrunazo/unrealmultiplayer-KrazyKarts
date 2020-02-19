// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

UCLASS()
class KRAZYKARTS_API AGoKart : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGoKart();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MoveForward(float Val);
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MoveRight(float Val);
	void MoveForward(float Val);
	void MoveRight(float Val);
	float CurSpeed = 0.0f;
	float CurTurnSpeed = 0.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Kart")
	float Accel = 80.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Kart")
	float Friction = 0.02f;
	// UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Kart")
	// float MaxTurnSpeed = 80.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Kart")
	float TurnAccel = 0.2f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Kart")
	float TurnRadius = 20.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Kart")
	float TurnFriction = 0.08f;
	UPROPERTY(replicated)
	FTransform ReplicatedTran = GetActorTransform();

private:
	void UpdateLocation(float DeltaTime);
	void UpdateRotation(float DeltaTime);
	float ForwardAxis = 0.0f;
	float RightAxis = 0.0f;

};
