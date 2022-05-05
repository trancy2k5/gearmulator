#include "Tabs.h"
#include "../juceUiLib/uiObject.h"
#include "VirusEditor.h"

namespace genericVirusUI
{
	Tabs::Tabs(VirusEditor& _editor): m_editor(_editor)
	{
		std::set<std::string> array;
		m_editor.findBindingComponent(array, "bindComponentName", false);
		for(const auto& pages : array)
		{
			m_tabs.push_back(m_editor.findComponent(pages));
		}

		array.clear();
		m_editor.findBindingComponent(array, "bindButtonName", false);

		for(const auto& pages : array)
		{
			m_tabButtons.push_back(m_editor.findComponentT<juce::Button>(pages));
		}

		if(m_tabs.size() != m_tabButtons.size())
			throw std::runtime_error("Number of tabs does not match number of tab buttons, not all requested objects have been found");

		for(size_t i=0; i<m_tabButtons.size(); ++i)
			m_tabButtons[i]->onClick = [this, i] { setPage(i); };

		setPage(0);
	}

	void Tabs::setPage(const size_t _page) const
	{
		for(size_t i=0; i<m_tabs.size(); ++i)
		{
			m_tabs[i]->setVisible(_page == i);
			m_tabButtons[i]->setToggleState(_page == i, juce::dontSendNotification);
		}
	}
}
