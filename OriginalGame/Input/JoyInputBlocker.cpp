#include "JoyInputBlocker.h"

bool UJoyInputBlocker::BlockMoveInput(TConstArrayView<TObjectPtr<UObject>> const& Blockers, UObject* InputReceiver,
	FInputActionValue const& InputActionValue)
{
	for (UObject* Blocker : Blockers)
	{
		if (!Blocker)
		{
			continue;
		}

		if (IJoyInputBlocker::Execute_BlockMoveInput(Blocker, InputReceiver, InputActionValue))
		{
			return true;
		}
	}

	return false;
}

bool UJoyInputBlocker::BlockAbilityTagPressInput(
	TConstArrayView<TObjectPtr<UObject>> const& Blockers, UObject* InputReceiver, FGameplayTag const& InputTag)
{
	for (UObject* Blocker : Blockers)
	{
		if (!Blocker)
		{
			continue;
		}

		if (IJoyInputBlocker::Execute_BlockAbilityTagPressInput(Blocker, InputReceiver, InputTag))
		{
			return true;
		}
	}

	return false;
}

bool UJoyInputBlocker::BlockAbilityTagReleaseInput(
	TConstArrayView<TObjectPtr<UObject>> const& Blockers, UObject* InputReceiver, FGameplayTag const& InputTag)
{
	for (UObject* Blocker : Blockers)
	{
		if (!Blocker)
		{
			continue;
		}

		if (IJoyInputBlocker::Execute_BlockAbilityTagReleaseInput(Blocker, InputReceiver, InputTag))
		{
			return true;
		}
	}

	return false;
}

bool UJoyInputBlocker::BlockLookMoveInput(TConstArrayView<TObjectPtr<UObject>> const& Blockers, UObject* InputReceiver,
	const FInputActionValue& InputActionValue)
{
	for (UObject* Blocker : Blockers)
	{
		if (!Blocker)
		{
			continue;
		}

		// 通过拦截器禁用了输入事件
		if (IJoyInputBlocker::Execute_BlockLookMoveInput(Blocker, InputReceiver, InputActionValue))
		{
			return true;
		}
	}

	return false;
}

bool UJoyInputBlocker::BlockMouseScrollInput(TConstArrayView<TObjectPtr<UObject>> const& Blockers,
	UObject* InputReceiver, const FInputActionValue& InputActionValue)
{
	for (UObject* Blocker : Blockers)
	{
		if (!Blocker)
		{
			continue;
		}

		// 通过拦截器禁用了输入事件
		if (IJoyInputBlocker::Execute_BlockMouseScrollInput(Blocker, InputReceiver, InputActionValue))
		{
			return true;
		}
	}

	return false;
}

bool IJoyInputBlocker::BlockMoveInput_Implementation(UObject*, const FInputActionValue&)
{
	return false;
}

bool IJoyInputBlocker::BlockAbilityTagPressInput_Implementation(UObject*, FGameplayTag const&)
{
	return false;
}

bool IJoyInputBlocker::BlockAbilityTagReleaseInput_Implementation(UObject*, FGameplayTag const&)
{
	return false;
}

bool IJoyInputBlocker::BlockLookMoveInput_Implementation(UObject*, const FInputActionValue&)
{
	return false;
}

bool IJoyInputBlocker::BlockMouseScrollInput_Implementation(UObject*, const FInputActionValue&)
{
	return false;
}