#include "pch.h"
#include "TKickback.h"


#include "control.h"
#include "loader.h"
#include "maths.h"
#include "render.h"
#include "timer.h"
#include "TPinballTable.h"

TKickback::TKickback(TPinballTable* table, int groupIndex): TCollisionComponent(table, groupIndex, true)
{
	MessageField = 0;
	Timer = 0;
	KickActiveFlag = 0;
	TimerTime = 0.7f;
	TimerTime2 = 0.1f;
	Threshold = 1000000000.0f;
}

int TKickback::Message(MessageCode code, float value)
{
	if ((code == MessageCode::SetTiltLock || code == MessageCode::Reset) && Timer)
	{
		timer::kill(Timer);
		if (ListBitmap)
			RenderSprite->set_bitmap(nullptr);
		Timer = 0;
		KickActiveFlag = 0;
		Threshold = 1000000000.0;
	}
	return 0;
}

void TKickback::Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance,
                          TEdgeSegment* edge)
{
	if (PinballTable->TiltLockFlag)
	{
		maths::basic_collision(ball, nextPosition, direction, Elasticity, Smoothness, 0.0,
		                       Boost);
	}
	else
	{
		if (!KickActiveFlag)
		{
			Threshold = 1000000000.0;
			KickActiveFlag = 1;
			Timer = timer::set(TimerTime, this, TimerExpired);
		}
		if (DefaultCollision(ball, nextPosition, direction))
			KickActiveFlag = 0;
	}
}

void TKickback::TimerExpired(int timerId, void* caller)
{
	auto kick = static_cast<TKickback*>(caller);

	if (kick->KickActiveFlag)
	{
		kick->Threshold = 0.0;
		kick->Timer = timer::set(kick->TimerTime2, kick, TimerExpired);
		loader::play_sound(kick->HardHitSoundId, kick, "TKickback");
		if (kick->ListBitmap)
		{
			auto bmp = kick->ListBitmap->at(1);
			auto zMap = kick->ListZMap->at(1);
			kick->RenderSprite->set(
				bmp,
				zMap,
				bmp->XPosition - kick->PinballTable->XOffset,
				bmp->YPosition - kick->PinballTable->YOffset);
		}
	}
	else
	{
		if (kick->ListBitmap)
		{
			auto bmp = kick->ListBitmap->at(0);
			auto zMap = kick->ListZMap->at(0);
			kick->RenderSprite->set(
				bmp,
				zMap,
				bmp->XPosition - kick->PinballTable->XOffset,
				bmp->YPosition - kick->PinballTable->YOffset);
		}
		kick->Timer = 0;
		control::handler(MessageCode::ControlTimerExpired, kick);
	}
}
