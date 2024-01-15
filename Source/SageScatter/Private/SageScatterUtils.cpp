// 2023 Green Rain Studios


#include "SageScatterUtils.h"

FVector USageScatterUtils::CalculateOffsets(FVector Offset, FVector Forward, FVector Right, FVector Up)
{
	// The Offset vector's components represent distances along the Forward, Right, and Up directions.
	// We scale each direction vector by its corresponding distance and then add the results together.
	return Offset.X*Forward + Offset.Y*Right + Offset.Z*Up;
}

FRotator USageScatterUtils::MakeRotatorFromAxes(FVector Forward, FVector Right, FVector Up)
{
	Forward.Normalize();
	Right.Normalize();
	Up.Normalize();

	FMatrix RotMatrix(Forward, Right, Up, FVector::ZeroVector);

	return RotMatrix.Rotator();
}
