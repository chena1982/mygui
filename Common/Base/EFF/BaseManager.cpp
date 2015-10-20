/*!
	@file
	@author		Albert Semenov
	@date		05/2009
*/


#include "Precompiled.h"

#include "BaseManager.h"
#include <MyGUI_EFFPlatform.h>

#if MYGUI_PLATFORM == MYGUI_PLATFORM_WIN32
#	include <windows.h>
#	include <winuser.h>
#endif


namespace base
{

	BaseManager::BaseManager() :
		mGUI(nullptr),
		mPlatform(nullptr),
		hWnd(0),
		hInstance(nullptr),
		mExit(false),
		mResourceFileName("MyGUI_Core.xml"),
		mIsDeviceLost(false)
	{
		//mApp.Init(effTRUE, 1024, 768, effTRUE, effTRUE);
		//mApp.OnRenderGUI += EFFEventCall(this, &base::BaseManager::drawOneFrame);
	}

	BaseManager::~BaseManager()
	{
	}

	void BaseManager::windowResized()
	{
		RECT rect = { 0, 0, 0, 0 };
		GetClientRect(hWnd, &rect);
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;

		if (mPlatform)
			mPlatform->getRenderManagerPtr()->setViewSize(width, height);

		setInputViewSize(width, height);
	}

	void BaseManager::windowMove()
	{
	}

	bool BaseManager::create(HWND hWnd)
	{

		this->hWnd = hWnd;

		createRender();

		createGui();

		createInput((size_t)hWnd);

		createPointerManager((size_t)hWnd);

		createScene();

		windowResized();

		return true;
	}

	void BaseManager::destroy()
	{
		destroyScene();

		destroyPointerManager();

		destroyInput();

		destroyGui();

		destroyRender();
	}

	void BaseManager::setupResources()
	{
		MyGUI::xml::Document doc;

		if (!doc.open(std::string("resources.xml")))
			doc.getLastError();

		MyGUI::xml::ElementPtr root = doc.getRoot();
		if (root == nullptr || root->getName() != "Paths")
			return;

		MyGUI::xml::ElementEnumerator node = root->getElementEnumerator();
		while (node.next())
		{
			if (node->getName() == "Path")
			{
				if (node->findAttribute("root") != "")
				{
					bool root = MyGUI::utility::parseBool(node->findAttribute("root"));
					if (root)
						mRootMedia = node->getContent();
				}
				addResourceLocation(node->getContent(), false);
			}
		}

		addResourceLocation(getRootMedia() + "/Common/Base");
	}

	void BaseManager::createGui()
	{
		mPlatform = new MyGUI::MyGUI_EFFPlatform();
		mPlatform->initialise(mDevice);

		setupResources();

		mGUI = new MyGUI::Gui();
		mGUI->initialise(mResourceFileName);
	}

	void BaseManager::destroyGui()
	{
		if (mGUI)
		{
			mGUI->shutdown();
			delete mGUI;
			mGUI = nullptr;
		}

		if (mPlatform)
		{
			mPlatform->shutdown();
			delete mPlatform;
			mPlatform = nullptr;
		}
	}

	size_t BaseManager::getWindowHandle()
	{
		return (size_t)hWnd;
	}

	void BaseManager::setWindowCaption(const std::wstring& _text)
	{
		SetWindowTextW(hWnd, _text.c_str());
	}

	void BaseManager::prepare()
	{
	}

	void BaseManager::addResourceLocation(const std::string& _name, bool _recursive)
	{
		mPlatform->getDataManagerPtr()->addResourceLocation(_name, _recursive);
	}

	void BaseManager::injectMouseMove(int _absx, int _absy, int _absz)
	{
		if (!mGUI)
			return;

		MyGUI::InputManager::getInstance().injectMouseMove(_absx, _absy, _absz);
	}

	void BaseManager::injectMousePress(int _absx, int _absy, MyGUI::MouseButton _id)
	{
		if (!mGUI)
			return;

		MyGUI::InputManager::getInstance().injectMousePress(_absx, _absy, _id);
	}

	void BaseManager::injectMouseRelease(int _absx, int _absy, MyGUI::MouseButton _id)
	{
		if (!mGUI)
			return;

		MyGUI::InputManager::getInstance().injectMouseRelease(_absx, _absy, _id);
	}

	void BaseManager::injectKeyPress(MyGUI::KeyCode _key, MyGUI::Char _text)
	{
		if (!mGUI)
			return;

		if (_key == MyGUI::KeyCode::Escape)
		{
			mExit = true;
			return;
		}

		MyGUI::InputManager::getInstance().injectKeyPress(_key, _text);
	}

	void BaseManager::injectKeyRelease(MyGUI::KeyCode _key)
	{
		if (!mGUI)
			return;

		MyGUI::InputManager::getInstance().injectKeyRelease(_key);
	}



	bool BaseManager::createRender()
	{
		// инициализация direct3d
		mDevice = EFF3DGetDevice();

		return true;
	}

	void BaseManager::run()
	{
		//mApp.Run();
	}

	void BaseManager::drawOneFrame(float elapsedTime)
	{

		mPlatform->getRenderManagerPtr()->drawOneFrame();

	}

	void BaseManager::destroyRender()
	{

	}

	void BaseManager::quit()
	{
		mExit = true;
	}

	const std::string& BaseManager::getRootMedia()
	{
		return mRootMedia;
	}

	void BaseManager::setResourceFilename(const std::string& _flename)
	{
		mResourceFileName = _flename;
	}

} // namespace base
