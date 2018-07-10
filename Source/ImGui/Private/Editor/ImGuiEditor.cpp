// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#include "ImGuiPrivatePCH.h"

#if WITH_EDITOR

#include "ImGuiEditor.h"

#include "ImGuiSettings.h"

#include <ISettingsModule.h>


#define LOCTEXT_NAMESPACE "ImGuiEditor"

#define SETTINGS_CONTAINER TEXT("Project"), TEXT("Plugins"), TEXT("ImGui")


namespace
{
	ISettingsModule* GetSettingsModule()
	{
		return FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	}
}

FImGuiEditor::FImGuiEditor()
{
	Register();

	// As a side effect of being part of the ImGui module, we need to support deferred registration (only executed if
	// module is loaded manually at the very early stage).
	if (!IsRegistrationCompleted())
	{
		CreateRegistrator();
	}
}

FImGuiEditor::~FImGuiEditor()
{
	Unregister();
}

void FImGuiEditor::Register()
{
	// Only register after UImGuiSettings class is initialized (necessary to check in early loading stages).
	if (!bSettingsRegistered && UImGuiSettings::StaticClass()->IsValidLowLevelFast())
	{
		if (ISettingsModule* SettingsModule = GetSettingsModule())
		{
			bSettingsRegistered = true;

			SettingsModule->RegisterSettings(SETTINGS_CONTAINER,
				LOCTEXT("ImGuiSettingsName", "ImGui"),
				LOCTEXT("ImGuiSettingsDescription", "Configure the Unreal ImGui plugin."),
				GetMutableDefault<UImGuiSettings>());
		}
	}
}

void FImGuiEditor::Unregister()
{
	if (bSettingsRegistered)
	{
		bSettingsRegistered = false;

		if (ISettingsModule* SettingsModule = GetSettingsModule())
		{
			SettingsModule->UnregisterSettings(SETTINGS_CONTAINER);
		}
	}
}

void FImGuiEditor::CreateRegistrator()
{
	if (!RegistratorHandle.IsValid())
	{
		RegistratorHandle = FModuleManager::Get().OnModulesChanged().AddLambda([this](FName Name, EModuleChangeReason Reason)
		{
			if (Reason == EModuleChangeReason::ModuleLoaded)
			{
				Register();
			}

			if (IsRegistrationCompleted())
			{
				ReleaseRegistrator();
			}
		});
	}
}

void FImGuiEditor::ReleaseRegistrator()
{
	if (RegistratorHandle.IsValid())
	{
		FModuleManager::Get().OnModulesChanged().Remove(RegistratorHandle);
		RegistratorHandle.Reset();
	}
}


#undef SETTINGS_CONTAINER
#undef LOCTEXT_NAMESPACE

#endif // WITH_EDITOR
