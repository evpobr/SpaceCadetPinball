#include "TFlagSpinner.h"

#include "control.h"
#include "loader.h"
#include "objlist_class.h"
#include "render.h"
#include "TBall.h"
#include "timer.h"
#include "TLine.h"
#include "TPinballTable.h"
#include "TZmapList.h"

TFlagSpinner::TFlagSpinner(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, false)
{
	visualStruct visual{};
	vector_type end{}, start{};

	Timer = 0;
	loader::query_visual(groupIndex, 0, &visual);
	end.X = *visual.FloatArr;
	end.Y = visual.FloatArr[1];
	start.X = visual.FloatArr[2];
	start.Y = visual.FloatArr[3];
	auto line = new TLine(this, &ActiveFlag, visual.CollisionGroup, &start, &end);
	if (line)
	{
		line->place_in_grid();
		EdgeList->Add(line);
	}

	line = new TLine(this, &ActiveFlag, visual.CollisionGroup, &end, &start);
	PrevCollider = line;
	if (line)
	{
		line->place_in_grid();
		EdgeList->Add(line);
	}

	SpeedDecrement = 0.64999998f;
	MaxSpeed = 50000.0f;
	MinSpeed = 5.0f;
	auto speedDec = loader::query_float_attribute(groupIndex, 0, 1202);
	if (speedDec)
		SpeedDecrement = *speedDec;
	auto maxSpeed = loader::query_float_attribute(groupIndex, 0, 1200);
	if (maxSpeed)
		MaxSpeed = *maxSpeed;
	auto minSpeed = loader::query_float_attribute(groupIndex, 0, 1201);
	if (minSpeed)
		MinSpeed = *minSpeed;
}

int TFlagSpinner::Message(int code, float value)
{
	if (code == 1024)
	{
		if (Timer)
		{
			timer::kill(Timer);
			Timer = 0;
		}
		BmpIndex = 0;
		auto bmp = static_cast<gdrv_bitmap8*>(ListBitmap->Get(0));
		auto zMap = static_cast<zmap_header_type*>(ListZMap->Get(0));
		render::sprite_set(
			RenderSprite,
			bmp,
			zMap,
			bmp->XPosition - PinballTable->XOffset,
			bmp->YPosition - PinballTable->YOffset);
	}
	return 0;
}

void TFlagSpinner::Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef,
                             TEdgeSegment* edge)
{
	ball->Position.X = nextPosition->X;
	ball->Position.Y = nextPosition->Y;
	ball->RayMaxDistance = ball->RayMaxDistance - coef;
	ball->not_again(edge);

	SpinDirection = 2 * (PrevCollider != edge) - 1;
	if (ball->Speed == 0.0)
		Speed = MinSpeed;
	else
		Speed = ball->Speed * 20.0f;
	if (Speed < MinSpeed)
		Speed = MinSpeed;
	if (Speed > MaxSpeed)
		Speed = MaxSpeed;
	NextFrame();
}

void TFlagSpinner::put_scoring(int index, int score)
{
	if (index < 2)
		Scores[index] = score;
}

int TFlagSpinner::get_scoring(int index)
{
	return index < 2 ? Scores[index] : 0;
}

void TFlagSpinner::NextFrame()
{
	BmpIndex += SpinDirection;
	int bmpIndex = BmpIndex;
	int bmpCount = ListBitmap->Count();
	if (bmpIndex >= bmpCount)
		BmpIndex = 0;
	else if (bmpIndex < 0)
		BmpIndex = bmpCount - 1;

	if (!PinballTable->TiltLockFlag)
	{
		control::handler(63, this);
		if (SoftHitSoundId)
			loader::play_sound(SoftHitSoundId);
		if (!BmpIndex)
			control::handler(62, this);
	}

	auto bmp = static_cast<gdrv_bitmap8*>(ListBitmap->Get(BmpIndex));
	auto zMap = static_cast<zmap_header_type*>(ListZMap->Get(BmpIndex));
	render::sprite_set(
		RenderSprite,
		bmp,
		zMap,
		bmp->XPosition - PinballTable->XOffset,
		bmp->YPosition - PinballTable->YOffset);

	Speed *= SpeedDecrement;
	if (Speed >= MinSpeed)
	{
		timer::set(1.0f / Speed, this, SpinTimer);
	}
}

void TFlagSpinner::SpinTimer(int timerId, void* caller)
{
	auto spinner = static_cast<TFlagSpinner*>(caller);
	spinner->Timer = 0;
	spinner->NextFrame();
}
