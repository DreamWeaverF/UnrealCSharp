#include "Registry/FDynamicRegistry.h"
#include "Delegate/FUnrealCSharpModuleDelegates.h"
#include "Dynamic/FDynamicClassGenerator.h"
#include "Environment/FCSharpEnvironment.h"
#include "Reflection/FReflectionRegistry.h"

FDynamicRegistry::FDynamicRegistry()
{
	Initialize();
}

FDynamicRegistry::~FDynamicRegistry()
{
	Deinitialize();
}

void FDynamicRegistry::Initialize()
{
	FDynamicClassGenerator::OnPostClassConstructor = [](UObject* InObject)
	{
		if (InObject != nullptr)
		{
			if (IsInGameThread())
			{
				auto& Environment = FCSharpEnvironment::GetEnvironment();

				Environment.Bind<true>(InObject);

				Environment.ConstructManagedObject(InObject);
			}
		}
	};

	OnCSharpEnvironmentInitializeDelegateHandle = FUnrealCSharpModuleDelegates::OnCSharpEnvironmentInitialize.AddRaw(
		this, &FDynamicRegistry::OnCSharpEnvironmentInitialize);
}

void FDynamicRegistry::Deinitialize()
{
	if (OnCSharpEnvironmentInitializeDelegateHandle.IsValid())
	{
		FUnrealCSharpModuleDelegates::OnCSharpEnvironmentInitialize.Remove(OnCSharpEnvironmentInitializeDelegateHandle);
	}

	FDynamicClassGenerator::OnPostClassConstructor = nullptr;
}

void FDynamicRegistry::OnCSharpEnvironmentInitialize() const
{
	RegisterDynamic();
}

void FDynamicRegistry::RegisterDynamic() const
{
	if (FDomain::IsLoadSucceed())
	{
		FDynamicGeneratorCore::Generator(FReflectionRegistry::Get().GetUClassAttributeClass(),
		                                 [](FClassReflection* InClass)
		                                 {
			                                 if (const auto DynamicClass = FDynamicClassGenerator::GetDynamicClass(
				                                 InClass))
			                                 {
				                                 FCSharpEnvironment::GetEnvironment().Bind<true>(DynamicClass);
			                                 }
		                                 }
		);
	}
}
