/*!
	@file
	@author		Losev Vasiliy aka bool
	@date		06/2009
*/

#include "windows.h"
#include "EFF3D.h"
USE_EFF3D

#include "MyGUI_EFFVertexBuffer.h"
#include "MyGUI_VertexData.h"
#include "MyGUI_EFFDiagnostic.h"

namespace MyGUI
{

	const size_t VERTEX_IN_QUAD = 6;
	const size_t RENDER_ITEM_STEEP_REALLOCK = 5 * VERTEX_IN_QUAD;

	MyGUI_EFFVertexBuffer::MyGUI_EFFVertexBuffer(EFF3DDevice* _device, MyGUI_EFFRenderManager* _pRenderManager) :
		mNeedVertexCount(0),
		mVertexCount(RENDER_ITEM_STEEP_REALLOCK),
		mpEFF3DDevice(_device),
		pRenderManager(_pRenderManager),
		mpBuffer(NULL)
	{
	}

	MyGUI_EFFVertexBuffer::~MyGUI_EFFVertexBuffer()
	{
		destroy();
	}

	void MyGUI_EFFVertexBuffer::setVertexCount(size_t _count)
	{
		if (_count != mNeedVertexCount)
		{
			mNeedVertexCount = _count;
			resize();
		}
	}

	size_t MyGUI_EFFVertexBuffer::getVertexCount()
	{
		return mNeedVertexCount;
	}

	Vertex* MyGUI_EFFVertexBuffer::lock()
	{
		void* lockPtr = nullptr;
		HRESULT result = mpBuffer->LockBuffer(0, 0, (void**)&lockPtr, 0);
		if (FAILED(result))
		{
			MYGUI_PLATFORM_EXCEPT("Failed to lock vertex buffer (error code " << result << ").");
		}
		return reinterpret_cast<Vertex*>(lockPtr);
	}

	void MyGUI_EFFVertexBuffer::unlock()
	{
		HRESULT result = mpBuffer->UnlockBuffer();
		if (FAILED(result))
		{
			MYGUI_PLATFORM_EXCEPT("Failed to unlock vertex buffer (error code " << result << ").");
		}
	}

	bool MyGUI_EFFVertexBuffer::setToStream(size_t stream)
	{
		if (mpEFF3DDevice->SetStreamSource(stream, mpBuffer, 0, sizeof(MyGUI::Vertex)))
			return true;
		return false;
	}

	bool MyGUI_EFFVertexBuffer::create()
	{
		if (mNeedVertexCount == 0)
		{
			return false;
		}

		DWORD length = mNeedVertexCount * sizeof(MyGUI::Vertex);

		if (mpEFF3DDevice->CreateVertexBuffer(length, EFF3DUSAGE_WRITEONLY, 0, EFF3DPOOL_DEFAULT, &mpBuffer))
		{
			return true;
		}
		return false;
	}

	void MyGUI_EFFVertexBuffer::destroy()
	{
		SF_RELEASE(mpBuffer);
	}

	void MyGUI_EFFVertexBuffer::resize()
	{
		if (mpEFF3DDevice)
		{
			destroy();
			create();
		}
	}

} // namespace MyGUI
