/*!
	@file
	@author		Losev Vasiliy aka bool
	@date		06/2009
*/

#include "windows.h"
#include "EFF3D.h"
USE_EFF3D

#include "MyGUI_EFFTexture.h"
#include "MyGUI_EFFDataManager.h"
#include "MyGUI_EFFRTTexture.h"
#include "MyGUI_EFFDiagnostic.h"

namespace MyGUI
{

	MyGUI_EFFTexture::MyGUI_EFFTexture(const std::string& _name, EFF3DDevice * _device) :
		mName(_name),
		mpEFF3DDevice(_device),
		mpTexture(NULL),
		mNumElemBytes(0),
		mLock(false),
		mRenderTarget(nullptr),
		mInternalPool(EFF3DPOOL_DEFAULT),
		mInternalFormat(EFF3DFMT_UNKNOWN),
		mInternalUsage(0)
	{
	}

	MyGUI_EFFTexture::~MyGUI_EFFTexture()
	{
		destroy();
	}

	const std::string& MyGUI_EFFTexture::getName() const
	{
		return mName;
	}

	void MyGUI_EFFTexture::createManual(int _width, int _height, TextureUsage _usage, PixelFormat _format)
	{
		destroy();

		mInternalUsage = 0;
		mInternalFormat = EFF3DFMT_UNKNOWN;

		mSize.set(_width, _height);
		mTextureUsage = _usage;
		mPixelFormat = _format;
		mInternalPool = EFF3DPOOL_DEFAULT;

		if (mTextureUsage.isValue(TextureUsage::RenderTarget))
		{
			mInternalUsage |= EFF3DUSAGE_RENDERTARGET;
		}
		else if (mTextureUsage.isValue(TextureUsage::Dynamic))
		{
			mInternalUsage |= EFF3DUSAGE_DYNAMIC;
		}
		else if (mTextureUsage.isValue(TextureUsage::Stream))
		{
			mInternalUsage |= EFF3DUSAGE_DYNAMIC;
		}

		if (mPixelFormat == PixelFormat::R8G8B8A8)
		{
			mInternalFormat = EFF3DFMT_A8R8G8B8;
			mNumElemBytes = 4;
		}
		else if (mPixelFormat == PixelFormat::R8G8B8)
		{
			mInternalFormat = EFF3DFMT_R8G8B8;
			mNumElemBytes = 3;
		}
		else if (mPixelFormat == PixelFormat::L8A8)
		{
			mInternalFormat = EFF3DFMT_A8L8;
			mNumElemBytes = 2;
		}
		else if (mPixelFormat == PixelFormat::L8)
		{
			mInternalFormat = EFF3DFMT_L8;
			mNumElemBytes = 1;
		}
		else
		{
			MYGUI_PLATFORM_EXCEPT("Creating texture with unknown pixel formal.");
		}


		if (!mpEFF3DDevice->CreateTexture(mSize.width, mSize.height, 1, mInternalUsage, mInternalFormat, mInternalPool, &mpTexture))
		{
			MYGUI_PLATFORM_EXCEPT("Failed to create texture: " << "size '" << mSize <<
				"' internal usage '" << mInternalUsage <<
				"' internal format '" << mInternalFormat << "'."
				);
		}

	}

	void MyGUI_EFFTexture::loadFromFile(const std::string& _filename)
	{
		destroy();
		mTextureUsage = TextureUsage::Default;
		mPixelFormat = PixelFormat::R8G8B8A8;
		mNumElemBytes = 4;

		std::string fullname = MyGUI_EFFDataManager::getInstance().getDataPath(_filename);

		if (!mpEFF3DDevice->CreateTextureFromFile(ANSI2EFFSTRING(fullname.c_str()), &mpTexture))
		{
			MYGUI_PLATFORM_EXCEPT("Failed to load texture '" << _filename);
			return;
		}

		if (mpTexture->GetImageInfo().format == EFF3DFMT_A8R8G8B8)
		{
			mPixelFormat = PixelFormat::R8G8B8A8;
			mNumElemBytes = 4;
		}
		else if (mpTexture->GetImageInfo().format == EFF3DFMT_R8G8B8)
		{
			mPixelFormat = PixelFormat::R8G8B8;
			mNumElemBytes = 3;
		}
		else if (mpTexture->GetImageInfo().format == EFF3DFMT_A8L8)
		{
			mPixelFormat = PixelFormat::L8A8;
			mNumElemBytes = 2;
		}
		else if (mpTexture->GetImageInfo().format == EFF3DFMT_L8)
		{
			mPixelFormat = PixelFormat::L8;
			mNumElemBytes = 1;
		}

		mSize.set(mpTexture->GetImageInfo().width, mpTexture->GetImageInfo().height);
	}

	void MyGUI_EFFTexture::destroy()
	{
		if (mRenderTarget != nullptr)
		{
			delete mRenderTarget;
			mRenderTarget = nullptr;
		}

		SF_RELEASE(mpTexture);
	}

	int MyGUI_EFFTexture::getWidth()
	{
		return mSize.width;
	}

	int MyGUI_EFFTexture::getHeight()
	{
		return mSize.height;
	}

	void* MyGUI_EFFTexture::lock(TextureUsage _access)
	{
		EFF3DLOCKED_RECT d3dlr;
		int lockFlag = (_access == TextureUsage::Write && mTextureUsage.isValue(TextureUsage::Dynamic)) ? EFF3DLOCK_DISCARD : EFF3DLOCK_READONLY;

		HRESULT result = mpTexture->LockRect(0, &d3dlr, NULL, lockFlag);
		if (FAILED(result))
		{
			MYGUI_PLATFORM_EXCEPT("Failed to lock texture.");
		}

		mLock = true;
		return d3dlr.pBits;
	}

	void MyGUI_EFFTexture::unlock()
	{
		HRESULT result = mpTexture->UnlockRect(0);
		if (FAILED(result))
		{
			MYGUI_PLATFORM_EXCEPT("Failed to unlock texture (error code " << result << ").");
		}

		mLock = false;
	}

	bool MyGUI_EFFTexture::isLocked()
	{
		return mLock;
	}

	PixelFormat MyGUI_EFFTexture::getFormat()
	{
		return mPixelFormat;
	}

	size_t MyGUI_EFFTexture::getNumElemBytes()
	{
		return mNumElemBytes;
	}

	TextureUsage MyGUI_EFFTexture::getUsage()
	{
		return mTextureUsage;
	}

	IRenderTarget* MyGUI_EFFTexture::getRenderTarget()
	{
		if (mpTexture == nullptr)
			return nullptr;

		if (mRenderTarget == nullptr)
			mRenderTarget = new MyGUI_EFFRTTexture(mpEFF3DDevice, mpTexture);

		return mRenderTarget;
	}

	void MyGUI_EFFTexture::deviceLost()
	{
		if (mInternalPool == EFF3DPOOL_DEFAULT)
		{
			destroy();
		}
	}

	void MyGUI_EFFTexture::deviceRestore()
	{
		if (mInternalPool == EFF3DPOOL_DEFAULT)
		{
			if (!mpEFF3DDevice->CreateTexture(mSize.width, mSize.height, 1, mInternalUsage, mInternalFormat, EFF3DPOOL_DEFAULT, &mpTexture))
			{
				MYGUI_PLATFORM_EXCEPT("Failed to recreate texture on device restore.");
			}
		}
	}

} // namespace MyGUI
