#ifndef EOYS_TABBED_GUI
#define EOYS_TABBED_GUI

#include "al/ui/al_ControlGUI.hpp"

class TabbedGUI : public al::ControlGUI {
private:
  al::ParameterMenu mTabs{ "Tabs", "", 0 };
  std::vector<al::ParameterBundle*> mBundles;

  int drawTabBar(al::ParameterMenu& menu) {
    auto tabNames = menu.getElements();

    // Draw tab headers
    ImGui::Columns(tabNames.size(), nullptr, true);
    for (size_t i = 0; i < tabNames.size(); i++) {
      if (ImGui::Selectable(tabNames[i].c_str(), menu.get() == i)) {
        menu.set(i);
      }
      ImGui::NextColumn();
    }
    // Draw tab contents
    ImGui::Columns(1);
    return menu.get();
  }

public:
  void addTab(std::string name) {
    auto elements = mTabs.getElements();
    elements.push_back(name);
    mTabs.setElements(elements);
  }

  TabbedGUI() {
    // override draw function for ControlGUI
    drawFunction = [this]() {  
      // Draw tab bar
      int currentTab = drawTabBar(mTabs);
  
      // Draw tab contents according to currently selected tab
      if (currentTab < mBundles.size()) {
        al::ParameterGUI::drawBundle(mBundles[currentTab]);
      } else {
        ImGui::Text(" !!! Missing Params For This Tab !!! ");
      }
    };
  }

  void addBundle(al::ParameterBundle& bundle) {
    mBundles.push_back(&bundle);
  }

  void addTab(al::ParameterBundle& bundle) {
    addTab(bundle.name());
    mBundles.push_back(&bundle);
  }

  TabbedGUI& operator<<(al::ParameterBundle& bundle) {
    this->addTab(bundle);
    return *this;
  }

  void init(int x = 5, int y = 5, bool manageImgui = true) {
    al::ControlGUI::init(x, y, manageImgui);
  }
};

#endif // EOYS_TABBED_GUI