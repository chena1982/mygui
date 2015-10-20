/*!
	@file
	@author		Losev Vasiliy aka bool
	@date		06/2009
*/
#include "windows.h"
#include "EFF3D.h"
USE_EFF3D

#include "MyGUI_EFFRenderManager.h"
#include "MyGUI_EFFTexture.h"
#include "MyGUI_EFFVertexBuffer.h"
#include "MyGUI_EFFDiagnostic.h"
#include "MyGUI_Gui.h"
#include "MyGUI_Timer.h"

namespace MyGUI
{

	MyGUI_EFFRenderManager::MyGUI_EFFRenderManager() :
		mIsInitialise(false),
		mpEFF3DDevice(NULL),
		mUpdate(false)
	{
	}

	void MyGUI_EFFRenderManager::initialise(EFF3DDevice* _device)
	{
		MYGUI_PLATFORM_ASSERT(!mIsInitialise, getClassTypeName() << " initialised twice");
		MYGUI_PLATFORM_LOG(Info, "* Initialise: " << getClassTypeName());

		mpEFF3DDevice = _device;

		mVertexFormat = VertexColourType::ColourARGB;

		memset(&mInfo, 0, sizeof(mInfo));
		if (mpEFF3DDevice != nullptr)
		{
			EFF3DVIEWPORT9 vp;
			mpEFF3DDevice->GetViewport(&vp);
			setViewSize(vp.Width, vp.Height);
		}

		mUpdate = false;

		/*if (mpEFF3DDevice != nullptr)
		{
			EFF3DCAPS9 caps;
			mpEFF3DDevice->GetDeviceCaps(&caps);
			if (caps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
			{
				MYGUI_PLATFORM_LOG(Warning, "Non-squared textures not supported.");
			}
		}*/

		MYGUI_PLATFORM_LOG(Info, getClassTypeName() << " successfully initialized");
		mIsInitialise = true;
	}

	void MyGUI_EFFRenderManager::shutdown()
	{
		MYGUI_PLATFORM_ASSERT(mIsInitialise, getClassTypeName() << " is not initialised");
		MYGUI_PLATFORM_LOG(Info, "* Shutdown: " << getClassTypeName());

		destroyAllResources();
		mpEFF3DDevice = NULL;

		MYGUI_PLATFORM_LOG(Info, getClassTypeName() << " successfully shutdown");
		mIsInitialise = false;
	}

	IVertexBuffer* MyGUI_EFFRenderManager::createVertexBuffer()
	{
		return new MyGUI_EFFVertexBuffer(mpEFF3DDevice, this);
	}

	void MyGUI_EFFRenderManager::destroyVertexBuffer(IVertexBuffer* _buffer)
	{
		delete _buffer;
	}

	void MyGUI_EFFRenderManager::doRender(IVertexBuffer* _buffer, ITexture* _texture, size_t _count)
	{
		MyGUI_EFFTexture* dxTex = static_cast<MyGUI_EFFTexture*>(_texture);
		mpEFF3DDevice->SetTexture(0, dxTex->getEFFTexture());
		MyGUI_EFFVertexBuffer* dxVB = static_cast<MyGUI_EFFVertexBuffer*>(_buffer);
		dxVB->setToStream(0);
		// count in vertexes, triangle_list = vertexes / 3
		mpEFF3DDevice->DrawPrimitive(EFF3DPT_TRIANGLELIST, 0, _count / 3);
	}

	void MyGUI_EFFRenderManager::drawOneFrame()
	{
		Gui* gui = Gui::getInstancePtr();
		if (gui == nullptr)
			return;

		static Timer timer;
		static unsigned long last_time = timer.getMilliseconds();
		unsigned long now_time = timer.getMilliseconds();
		unsigned long time = now_time - last_time;

		onFrameEvent((float)((double)(time) / (double)1000));

		last_time = now_time;

		begin();
		onRenderToTarget(this, mUpdate);
		end();

		mUpdate = false;
	}

	void MyGUI_EFFRenderManager::begin()
	{
		mpEFF3DDevice->SetRenderState(EFF3DRS_ALPHABLENDENABLE, effTRUE);

		mpEFF3DDevice->SetTextureStageState(0, EFF3DTSS_COLOROP, EFF3DTOP_MODULATE);
		mpEFF3DDevice->SetTextureStageState(0, EFF3DTSS_COLORARG0, EFF3DTA_DIFFUSE);
		mpEFF3DDevice->SetTextureStageState(0, EFF3DTSS_COLORARG1, EFF3DTA_TEXTURE);
		mpEFF3DDevice->SetTextureStageState(0, EFF3DTSS_COLORARG2, EFF3DTA_DIFFUSE);

		mpEFF3DDevice->SetTextureStageState(0, EFF3DTSS_ALPHAOP, EFF3DTOP_MODULATE);
		mpEFF3DDevice->SetTextureStageState(0, EFF3DTSS_ALPHAARG0, EFF3DTA_DIFFUSE);
		mpEFF3DDevice->SetTextureStageState(0, EFF3DTSS_ALPHAARG1, EFF3DTA_TEXTURE);
		mpEFF3DDevice->SetTextureStageState(0, EFF3DTSS_ALPHAARG2, EFF3DTA_DIFFUSE);

		mpEFF3DDevice->SetSamplerState(0, EFF3DSAMP_MINFILTER, EFF3DTEXF_LINEAR);
		mpEFF3DDevice->SetSamplerState(0, EFF3DSAMP_MAGFILTER, EFF3DTEXF_LINEAR);
		mpEFF3DDevice->SetSamplerState(0, EFF3DSAMP_MIPFILTER, EFF3DTEXF_NONE);

		mpEFF3DDevice->SetSamplerState(0, EFF3DSAMP_ADDRESSU, EFF3DTADDRESS_CLAMP);
		mpEFF3DDevice->SetSamplerState(0, EFF3DSAMP_ADDRESSV, EFF3DTADDRESS_CLAMP);

		mpEFF3DDevice->SetRenderState(EFF3DRS_SRCBLEND,   EFF3DBLEND_SRCALPHA);
		mpEFF3DDevice->SetRenderState(EFF3DRS_DESTBLEND,  EFF3DBLEND_INVSRCALPHA);

		mpEFF3DDevice->SetRenderState(EFF3DRS_ZWRITEENABLE, effFALSE);
		mpEFF3DDevice->SetRenderState(EFF3DRS_ZENABLE, effFALSE);
		mpEFF3DDevice->SetRenderState(EFF3DRS_LIGHTING, 0);
		mpEFF3DDevice->SetRenderState(EFF3DRS_CULLMODE, EFF3DCULL_NONE);

		mpEFF3DDevice->SetRenderState(EFF3DRS_ALPHATESTENABLE, effFALSE);

		mpEFF3DDevice->SetFVF(EFF3DFVF_XYZ | EFF3DFVF_DIFFUSE | EFF3DFVF_TEX1);

		EFFMatrix4 m;
		EFFMatrix4Identity(&m);
		mpEFF3DDevice->SetTransform(EFF3DTS_WORLD, &m);
		mpEFF3DDevice->SetTransform(EFF3DTS_VIEW, &m);
		mpEFF3DDevice->SetTransform(EFF3DTS_PROJECTION, &m);
	}

	void MyGUI_EFFRenderManager::end()
	{
	}

	ITexture* MyGUI_EFFRenderManager::createTexture(const std::string& _name)
	{
		MapTexture::const_iterator item = mTextures.find(_name);
		MYGUI_PLATFORM_ASSERT(item == mTextures.end(), "Texture '" << _name << "' already exist");

		MyGUI_EFFTexture * texture = new MyGUI_EFFTexture(_name, mpEFF3DDevice);
		mTextures[_name] = texture;
		return texture;
	}

	void MyGUI_EFFRenderManager::destroyTexture(ITexture* _texture)
	{
		if (_texture == nullptr)
			return;

		MapTexture::iterator item = mTextures.find(_texture->getName());
		MYGUI_PLATFORM_ASSERT(item != mTextures.end(), "Texture '" << _texture->getName() << "' not found");

		mTextures.erase(item);
		delete _texture;
	}

	ITexture* MyGUI_EFFRenderManager::getTexture(const std::string& _name)
	{
		MapTexture::const_iterator item = mTextures.find(_name);
		if (item != mTextures.end())
			return item->second;
		return nullptr;
	}

	bool MyGUI_EFFRenderManager::isFormatSupported(PixelFormat _format, TextureUsage _usage)
	{
		EFF3DFORMAT internalFormat = EFF3DFMT_UNKNOWN;
		unsigned long internalUsage = 0;
		EFF3DPOOL internalPool = EFF3DPOOL_DEFAULT;

		if (_usage.isValue(TextureUsage::RenderTarget))
		{
			internalUsage |= EFF3DUSAGE_RENDERTARGET;
		}
		else if (_usage.isValue(TextureUsage::Dynamic))
		{
			internalUsage |= EFF3DUSAGE_DYNAMIC;
		}
		else if (_usage.isValue(TextureUsage::Stream))
		{
			internalUsage |= EFF3DUSAGE_DYNAMIC;
		}

		if (_format == PixelFormat::R8G8B8A8)
		{
			internalFormat = EFF3DFMT_A8R8G8B8;
		}
		else if (_format == PixelFormat::R8G8B8)
		{
			internalFormat = EFF3DFMT_R8G8B8;
		}
		else if (_format == PixelFormat::L8A8)
		{
			internalFormat = EFF3DFMT_A8L8;
		}
		else if (_format == PixelFormat::L8)
		{
			internalFormat = EFF3DFMT_L8;
		}

		EFF3DFORMAT requestedlFormat = internalFormat;
		mpEFF3DDevice->CheckFormatSupport(NULL, NULL, NULL, internalUsage, &internalFormat, internalPool);

		bool result = requestedlFormat == internalFormat;
		if (!result)
			MYGUI_PLATFORM_LOG(Warning, "Texture format '" << requestedlFormat << "'is not supported.");
		return result;
	}

	void MyGUI_EFFRenderManager::destroyAllResources()
	{
		for (MapTexture::const_iterator item = mTextures.begin(); item != mTextures.end(); ++item)
		{
			delete item->second;
		}
		mTextures.clear();
	}

	void MyGUI_EFFRenderManager::setViewSize(int _width, int _height)
	{
		if (_height == 0)
			_height = 1;
		if (_width == 0)
			_width = 1;

		mViewSize.set(_width, _height);

		mInfo.maximumDepth = 0.0f;
		mInfo.hOffset = -0.5f / float(mViewSize.width);
		mInfo.vOffset = -0.5f / float(mViewSize.height);
		mInfo.aspectCoef = float(mViewSize.height) / float(mViewSize.width);
		mInfo.pixScaleX = 1.0f / float(mViewSize.width);
		mInfo.pixScaleY = 1.0f / float(mViewSize.height);

		onResizeView(mViewSize);

		mUpdate = true;
	}

	void MyGUI_EFFRenderManager::deviceLost()
	{
		MYGUI_PLATFORM_LOG(Info, "device D3D lost");

		for (MapTexture::const_iterator item = mTextures.begin(); item != mTextures.end(); ++item)
		{
			static_cast<MyGUI_EFFTexture*>(item->second)->deviceLost();
		}
	}

	void MyGUI_EFFRenderManager::deviceRestore()
	{
		MYGUI_PLATFORM_LOG(Info, "device D3D restore");

		for (MapTexture::const_iterator item = mTextures.begin(); item != mTextures.end(); ++item)
		{
			static_cast<MyGUI_EFFTexture*>(item->second)->deviceRestore();
		}

		mUpdate = true;
	}

} // namespace MyGUI
