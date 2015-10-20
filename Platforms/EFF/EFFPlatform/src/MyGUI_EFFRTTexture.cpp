/*!
	@file
	@author		Albert Semenov
	@date		12/2009
*/

#include "windows.h"
#include "EFF3D.h"
USE_EFF3D

#include "MyGUI_EFFRTTexture.h"
#include "MyGUI_EFFRenderManager.h"

namespace MyGUI
{

	MyGUI_EFFRTTexture::MyGUI_EFFRTTexture(EFF3DDevice * _device, EFF3DTexture * _texture) :
		mpEFF3DDevice(_device),
		mpTexture(_texture),
		mpRenderSurface(NULL),
		mpBackBuffer(NULL)
	{
		mpTexture->GetSurfaceLevel(0, &mpRenderSurface);

		EFF3DSURFACE_DESC info;
		mpTexture->GetLevelDesc(0, &info);
		int width = info.Width;
		int height = info.Height;

		mRenderTargetInfo.maximumDepth = 0.0f;
		mRenderTargetInfo.hOffset = -0.5f / float(width);
		mRenderTargetInfo.vOffset = -0.5f / float(height);
		mRenderTargetInfo.aspectCoef = float(height) / float(width);
		mRenderTargetInfo.pixScaleX = 1.0f / float(width);
		mRenderTargetInfo.pixScaleY = 1.0f / float(height);
	}

	MyGUI_EFFRTTexture::~MyGUI_EFFRTTexture()
	{
		if (mpRenderSurface != nullptr)
		{
			mpRenderSurface->Release();
			mpRenderSurface = nullptr;
		}
	}

	void MyGUI_EFFRTTexture::begin()
	{
		mpEFF3DDevice->GetRenderTarget(0, &mpBackBuffer);

		mpEFF3DDevice->SetRenderTarget(0, mpRenderSurface);
		mpEFF3DDevice->Clear(0, NULL, EFF3DCLEAR_TARGET,
			EFF3DCOLOR_RGBA(0, 0, 0, 0), 1, 0);

		//mpEFF3DDevice->BeginScene();
	}

	void MyGUI_EFFRTTexture::end()
	{
		//mpEFF3DDevice->EndScene();

		mpEFF3DDevice->SetRenderTarget(0, mpBackBuffer);
		mpBackBuffer->Release();
	}

	void MyGUI_EFFRTTexture::doRender(IVertexBuffer* _buffer, ITexture* _texture, size_t _count)
	{
		MyGUI_EFFRenderManager::getInstance().doRender(_buffer, _texture, _count);
	}

} // namespace MyGUI
