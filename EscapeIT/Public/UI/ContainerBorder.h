// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Border.h"
#include "ContainerBorder.generated.h"

UCLASS()
class ESCAPEIT_API UContainerBorder : public UBorder
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Color")
	FLinearColor NormalBorder = { 0.0f,0.0f,0.0f,0.0f };
	
	UPROPERTY(EditAnywhere, Category = "Color")
	FLinearColor HoverBorder = { 1.0f,1.0f,1.0f,0.2f };

	UFUNCTION(BlueprintCallable)
	void OnNormalBorder();

	UFUNCTION(BlueprintCallable)
	void OnHoverBorder();
};
