/*==================================================================================
Copyright (c) 2008, binaryzebra
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the binaryzebra nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#ifndef __SINGLE_TEST_SCREEN_H__
#define __SINGLE_TEST_SCREEN_H__

#include "DAVAEngine.h"
#include "BaseScreen.h"
#include "BaseTest.h"

using namespace DAVA;

class SingleTestScreen : public BaseScreen
{
public:
	SingleTestScreen(BaseTest* test, uint32 fixedTime, uint32 fixedFramesCount, float32 fixedDelta, uint32 targetFrame);

	virtual void OnStart() override;
	virtual void OnFinish() override;

	virtual void BeginFrame() override;
	virtual void EndFrame() override;

	virtual void Update(float32 timeElapsed) override;
	virtual void Draw() override;
	
	virtual bool IsFinished() const override;

	bool isDebuggableTest() const;

protected:
	virtual ~SingleTestScreen();

private:
	BaseTest* singleTest;

	uint32 currentFrame;
	uint32 targetFrame;

	uint32 fixedTime;
	uint32 fixedFramesCount;
	float32 fixedDelta;
};

inline bool SingleTestScreen::IsFinished() const
{
	return singleTest->IsFinished();
}

inline bool SingleTestScreen::isDebuggableTest() const
{
	return targetFrame > 0;
}

#endif