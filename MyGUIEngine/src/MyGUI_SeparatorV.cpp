/*
 * This source file is part of MyGUI. For the latest info, see http://mygui.info/
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#include "MyGUI_Precompiled.h"
#include "MyGUI_SeparatorV.h"
#include "MyGUI_SeparatorH.h"
#include "MyGUI_InputManager.h"


namespace MyGUI
{

	SeparatorV::SeparatorV() :
		mLeftPanel(nullptr),
		mRightPanel(nullptr),
		mMinSizeLeft(0),
		mMinSizeRight(0)	
	{
	}

	SeparatorV::~SeparatorV()
	{
	}

	void SeparatorV::initialiseOverride()
	{
		Base::initialiseOverride();

		MYGUI_ASSERT(getParent(), "SeparatorV parent must not be null.");
	}

	void SeparatorV::assignLeftRightPanel()
	{
		std::string prefix = FindParentPrefix(getParent());

		mLeftPanel = getParent()->findWidget(prefix + "Left");
		mRightPanel = getParent()->findWidget(prefix + "Right");

		if (mLeftPanel == NULL || mRightPanel == NULL)
		{
			return;
		}

		mMinSizeLeft = MyGUI::utility::parseValue<int>(mLeftPanel->getUserString("MinSize"));
		mMinSizeRight = MyGUI::utility::parseValue<int>(mRightPanel->getUserString("MinSize"));

		eventMouseButtonPressed += MyGUI::newDelegate(this, &SeparatorV::notifyMouseButtonPressed);
		eventMouseDrag += MyGUI::newDelegate(this, &SeparatorV::notifyMouseDrag);
	}

	void SeparatorV::shutdownOverride()
	{
		//mImage = nullptr;

		Base::shutdownOverride();

		eventMouseButtonPressed -= MyGUI::newDelegate(this, &SeparatorV::notifyMouseButtonPressed);
		eventMouseDrag -= MyGUI::newDelegate(this, &SeparatorV::notifyMouseDrag);
	}

	void SeparatorV::notifyMouseButtonPressed(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id)
	{
		MYGUI_ASSERT(mLeftPanel, "SeparatorV left panel must not be null.");
		MYGUI_ASSERT(mRightPanel, "SeparatorV left panel must not be null.");

		if (_id == MyGUI::MouseButton::Left)
		{
			mStartLeftPanel = mLeftPanel->getCoord();
			mStartRightPanel = mRightPanel->getCoord();
			mStartSeparatorV = getCoord();
			mStartMousePosition = MyGUI::InputManager::getInstance().getMousePosition();
		}
	}

	void SeparatorV::notifyMouseDrag(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id)
	{
		MYGUI_ASSERT(mLeftPanel, "SeparatorV left panel must not be null.");
		MYGUI_ASSERT(mRightPanel, "SeparatorV left panel must not be null.");

		if (_id == MyGUI::MouseButton::Left)
		{
			MyGUI::IntPoint mousePosition = MyGUI::InputManager::getInstance().getMousePosition();
			int delta = mousePosition.left - mStartMousePosition.left;

			MyGUI::IntCoord leftPanel = mStartLeftPanel;
			MyGUI::IntCoord rightPanel = mStartRightPanel;
			MyGUI::IntCoord separatorVPanel = mStartSeparatorV;

			leftPanel.width += delta;
			separatorVPanel.left += delta;
			rightPanel.left += delta;
			rightPanel.width -= delta;

			int diffLeft = mMinSizeLeft - leftPanel.width;
			if (diffLeft > 0)
			{
				leftPanel.width += diffLeft;
				separatorVPanel.left += diffLeft;
				rightPanel.left += diffLeft;
				rightPanel.width -= diffLeft;
			}

			int diffRight = mMinSizeRight - rightPanel.width;
			if (diffRight > 0)
			{
				leftPanel.width -= diffRight;
				separatorVPanel.left -= diffRight;
				rightPanel.left -= diffRight;
				rightPanel.width += diffRight;
			}

			mLeftPanel->setCoord(leftPanel);
			mRightPanel->setCoord(rightPanel);
			setCoord(separatorVPanel);
		}
	}

} // namespace MyGUI
