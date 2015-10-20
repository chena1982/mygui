/*
 * This source file is part of MyGUI. For the latest info, see http://mygui.info/
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#include "MyGUI_Precompiled.h"
#include "MyGUI_SeparatorH.h"
#include "MyGUI_InputManager.h"


namespace MyGUI
{

	SeparatorH::SeparatorH() :
		mTopPanel(nullptr),
		mBottomPanel(nullptr),
		mMinSizeTop(0),
		mMinSizeBottom(0)	
	{
	}

	SeparatorH::~SeparatorH()
	{
	}

	void SeparatorH::initialiseOverride()
	{
		Base::initialiseOverride();

		MYGUI_ASSERT(getParent(), "SeparatorH parent must not be null.");
	}



	std::string FindParentPrefix(MyGUI::Widget* _parent)
	{
		std::string prefix = _parent->getUserString("BaseLayoutPrefix");
		if (!prefix.empty())
			return prefix;
		if (_parent->getParent() != nullptr)
			return FindParentPrefix(_parent->getParent());

		return "";
	}

	void SeparatorH::assignTopBottomPanel()
	{
		std::string prefix = FindParentPrefix(getParent());

		mTopPanel = getParent()->findWidget(prefix + "Top");
		mBottomPanel = getParent()->findWidget(prefix + "Bottom");

		if (mTopPanel == NULL || mBottomPanel == NULL)
		{
			return;
		}

		mMinSizeTop = MyGUI::utility::parseValue<int>(mTopPanel->getUserString("MinSize"));
		mMinSizeBottom = MyGUI::utility::parseValue<int>(mBottomPanel->getUserString("MinSize"));

		eventMouseButtonPressed += MyGUI::newDelegate(this, &SeparatorH::notifyMouseButtonPressed);
		eventMouseDrag += MyGUI::newDelegate(this, &SeparatorH::notifyMouseDrag);
	}

	void SeparatorH::shutdownOverride()
	{
		Base::shutdownOverride();

		eventMouseButtonPressed -= MyGUI::newDelegate(this, &SeparatorH::notifyMouseButtonPressed);
		eventMouseDrag -= MyGUI::newDelegate(this, &SeparatorH::notifyMouseDrag);
	}

	void SeparatorH::notifyMouseButtonPressed(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id)
	{
		MYGUI_ASSERT(mTopPanel, "SeparatorH top panel must not be null.");
		MYGUI_ASSERT(mBottomPanel, "SeparatorH bottom panel must not be null.");

		if (_id == MyGUI::MouseButton::Left)
		{
			mStartTopPanel = mTopPanel->getCoord();
			mStartBottomPanel = mBottomPanel->getCoord();
			mStartSeparatorH = getCoord();
			mStartMousePosition = MyGUI::InputManager::getInstance().getMousePosition();
		}
	}

	void SeparatorH::notifyMouseDrag(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id)
	{
		MYGUI_ASSERT(mTopPanel, "SeparatorH top panel must not be null.");
		MYGUI_ASSERT(mBottomPanel, "SeparatorH bottom panel must not be null.");

		if (_id == MyGUI::MouseButton::Left)
		{
			MyGUI::IntPoint mousePosition = MyGUI::InputManager::getInstance().getMousePosition();
			int delta = mousePosition.top - mStartMousePosition.top;

			MyGUI::IntCoord topPanel = mStartTopPanel;
			MyGUI::IntCoord bottomPanel = mStartBottomPanel;
			MyGUI::IntCoord separatorHPanel = mStartSeparatorH;

			topPanel.height += delta;
			separatorHPanel.top += delta;
			bottomPanel.top += delta;
			bottomPanel.height -= delta;

			int diffTop = mMinSizeTop - topPanel.height;
			if (diffTop > 0)
			{
				topPanel.height += diffTop;
				separatorHPanel.top += diffTop;
				bottomPanel.top += diffTop;
				bottomPanel.height -= diffTop;
			}

			int diffBottom = mMinSizeBottom - bottomPanel.height;
			if (diffBottom > 0)
			{
				topPanel.height -= diffBottom;
				separatorHPanel.top -= diffBottom;
				bottomPanel.top -= diffBottom;
				bottomPanel.height += diffBottom;
			}

			mTopPanel->setCoord(topPanel);
			mBottomPanel->setCoord(bottomPanel);
			setCoord(separatorHPanel);
		}
	}

} // namespace MyGUI
