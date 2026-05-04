#pragma once

#include <sstream>
#include <string>
#include <UnigineFileSystem.h>
#include <UnigineMathLib.h>
#include <UnigineNode.h>
#include <UnigineObjects.h>
#include <UnigineWidgets.h>
#include <UnigineWorld.h>

namespace unigine2_plugin
{
    template <class T>
    constexpr T* GetUniginePlugin(std::string plugin_name)
    {
        T* plugin = nullptr;

        auto plugin_id = Unigine::Engine::get()->findPlugin(plugin_name.c_str());
        if (plugin_id == -1)
        {
            auto error_msg = std::string("Cannot find {" + plugin_name + "}!\n");
            Unigine::Log::message(error_msg.c_str());
            return plugin;
        }

        Unigine::Plugin* plugin_interface = Unigine::Engine::get()->getPluginInterface(plugin_id);
        if (!plugin_interface)
        {
            auto error_msg = std::string(plugin_name + " is null!\n");
            Unigine::Log::message(error_msg.c_str());
            return plugin;
        }

        plugin = static_cast<T*>(plugin_interface);
        return plugin;
    };
}

namespace unigine2_ui
{
    namespace colors
    {
        const auto BTN_COLOR = Unigine::Math::vec4(81.0f / 255.0f, 82.0f / 255.0f, 85.0f / 255.0f, 1.0f);

        const auto LABEL_COLOR        = Unigine::Math::vec4(189.0f / 255.0f, 189.0f / 255.0f, 189.0f / 255.0f, 1.0f);
        const auto BORDER_COLOR       = Unigine::Math::vec4(81.0f / 255.0f, 82.0f / 255.0f, 85.0f / 255.0f, 1.0f);
        const auto FILL_COLOR         = Unigine::Math::vec4(40.0f / 255.0f, 42.0f / 255.0f, 44.0f / 255.0f, 1.0f);
        const auto PROGRESS_BAR_COLOR = Unigine::Math::vec4(39.0f / 255.0f, 133.0f / 255.0f, 200.0f / 255.0f, 1.0f);
        const auto TAB_BOX_FONT_COLOR = Unigine::Math::vec4(255.f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 1.0f);

        const auto X_SPRITE_COLOR = Unigine::Math::vec4(195.0f / 255.0f, 73.0f / 255.0f, 73.0f / 255.0f, 1.0f);
        const auto Y_SPRITE_COLOR = Unigine::Math::vec4(65.0f / 255.0f, 164.0f / 255.0f, 59.0f / 255.0f, 1.0f);
        const auto Z_SPRITE_COLOR = Unigine::Math::vec4(80.0f / 255.0f, 109.0f / 255.0f, 195.0f / 255.0f, 1.0f);

        const auto TEXT_SELECTION_COLOR = Unigine::Math::vec4(1.0f, 1.0f, 1.0f, 0.9f);

        inline Unigine::Math::vec4 HexToVec4Color(std::string hex, float alpha = 1)
        {
            hex = hex.substr(1); // Remove # character.
            Unigine::Math::vec4 color;
            std::stringstream   ss;
            int                 intensity;
            for (int i = 0; i < 3; i++)
            {
                ss << std::hex << hex.substr(i * 2, 2);
                ss >> intensity;
                color[i] = intensity / 255.0f;
                ss.clear();
            }
            color[3] = alpha;
            return color;
        }
    }
    inline void SetupSpriteWidgetForMouseEvents(Unigine::WidgetSpritePtr widget,
                                                std::string              default_icon,
                                                const std::string        onhover_icon,
                                                const std::string        selected_icon)
    {
        auto default_virtual_path = Unigine::FileSystem::getVirtualPath(
          Unigine::FileSystem::resolvePartialVirtualPath(default_icon.c_str()));
        auto default_btn_image = Unigine::Image::create(default_virtual_path);
        widget->setImage(default_btn_image);

        auto button_pressed_cb = [&, default_icon](const Unigine::WidgetPtr& widget_pressed)
          {
              auto path = Unigine::FileSystem::getVirtualPath(
                Unigine::FileSystem::resolvePartialVirtualPath(default_icon.c_str()));
              auto pressed_btn_image = Unigine::Image::create(path);

              auto sprite_widget = Unigine::checked_ptr_cast<Unigine::WidgetSprite>(widget_pressed);
              sprite_widget->setImage(pressed_btn_image);
          };
        widget->getEventPressed().connect(button_pressed_cb);

        auto button_clicked_cb = [&, selected_icon](const Unigine::WidgetPtr& widget_clicked)
          {
              auto path = Unigine::FileSystem::getVirtualPath(
                Unigine::FileSystem::resolvePartialVirtualPath(selected_icon.c_str()));
              auto clicked_btn_image = Unigine::Image::create(path);

              auto sprite_widget = Unigine::checked_ptr_cast<Unigine::WidgetSprite>(widget_clicked);
              sprite_widget->setImage(clicked_btn_image);
          };
        widget->getEventClicked().connect(button_clicked_cb);

        auto button_hover_cb = [&, onhover_icon](const Unigine::WidgetPtr& widget_hover)
          {
              auto path = Unigine::FileSystem::getVirtualPath(
                Unigine::FileSystem::resolvePartialVirtualPath(onhover_icon.c_str()));
              auto hover_btn_image = Unigine::Image::create(path);

              auto sprite_widget = Unigine::checked_ptr_cast<Unigine::WidgetSprite>(widget_hover);
              sprite_widget->setImage(hover_btn_image);
          };
        widget->getEventEnter().connect(button_hover_cb);

        auto button_hover_leave_cb = [&, default_icon](const Unigine::WidgetPtr& widget_hover_leave)
          {
              auto path = Unigine::FileSystem::getVirtualPath(
                Unigine::FileSystem::resolvePartialVirtualPath(default_icon.c_str()));
              auto hover_leave_btn_image = Unigine::Image::create(path);

              auto sprite_widget = Unigine::checked_ptr_cast<Unigine::WidgetSprite>(widget_hover_leave);
              sprite_widget->setImage(hover_leave_btn_image);
          };
        widget->getEventLeave().connect(button_hover_leave_cb);
    }

    inline void SetupTogglableSpriteWidgetForMouseEvents(Unigine::WidgetButtonPtr widget,
                                                         std::string              toggled_off_icon,
                                                         const std::string        onhover_icon,
                                                         const std::string        toggled_on_icon)
    {
        auto default_virtual_path = Unigine::FileSystem::getVirtualPath(
          Unigine::FileSystem::resolvePartialVirtualPath(toggled_off_icon.c_str()));
        auto default_btn_image = Unigine::Image::create(default_virtual_path);
        widget->setImage(default_btn_image);

        widget->setBackground(0);

        auto button_state_changed_cb = [&, toggled_on_icon, toggled_off_icon](const Unigine::WidgetPtr& widget_clicked)
          {
              auto              sprite_button = Unigine::checked_ptr_cast<Unigine::WidgetButton>(widget_clicked);
              Unigine::ImagePtr new_sprite;

              bool is_toggled = sprite_button->isToggled();

              if (is_toggled)
              {
                  auto path = Unigine::FileSystem::getVirtualPath(
                    Unigine::FileSystem::resolvePartialVirtualPath(toggled_on_icon.c_str()));
                  new_sprite = Unigine::Image::create(path);
              }
              else
              {
                  auto path = Unigine::FileSystem::getVirtualPath(
                    Unigine::FileSystem::resolvePartialVirtualPath(toggled_off_icon.c_str()));
                  new_sprite = Unigine::Image::create(path);
              }
              sprite_button->setImage(new_sprite);
          };
        widget->getEventChanged().connect(button_state_changed_cb);

        auto button_hover_cb = [&, onhover_icon](const Unigine::WidgetPtr& widget_hover)
          {
              auto sprite_button = Unigine::checked_ptr_cast<Unigine::WidgetButton>(widget_hover);

              bool is_toggled = sprite_button->isToggled();

              if (is_toggled)
              {
                  return;
              }

              auto path = Unigine::FileSystem::getVirtualPath(
                Unigine::FileSystem::resolvePartialVirtualPath(onhover_icon.c_str()));
              auto hover_btn_image = Unigine::Image::create(path);

              sprite_button->setImage(hover_btn_image);
          };
        widget->getEventEnter().connect(button_hover_cb);

        auto button_hover_leave_cb = [&, toggled_off_icon, toggled_on_icon](const Unigine::WidgetPtr& widget_hover_leave)
          {
              auto              sprite_button = Unigine::checked_ptr_cast<Unigine::WidgetButton>(widget_hover_leave);
              Unigine::ImagePtr new_sprite;

              bool is_toggled = sprite_button->isToggled();

              if (is_toggled)
              {
                  auto path = Unigine::FileSystem::getVirtualPath(
                    Unigine::FileSystem::resolvePartialVirtualPath(toggled_on_icon.c_str()));
                  new_sprite = Unigine::Image::create(path);
              }
              else
              {
                  auto path = Unigine::FileSystem::getVirtualPath(
                    Unigine::FileSystem::resolvePartialVirtualPath(toggled_off_icon.c_str()));
                  new_sprite = Unigine::Image::create(path);
              }
              sprite_button->setImage(new_sprite);
          };
        widget->getEventLeave().connect(button_hover_leave_cb);
    }

    inline Unigine::ImagePtr CreateImage(std::string icon)
    {
        auto sprite_virtual_path = Unigine::FileSystem::getVirtualPath(
          Unigine::FileSystem::resolvePartialVirtualPath(icon.c_str()));
        auto image = Unigine::Image::create(sprite_virtual_path);

        return image;
    }


    inline Unigine::WidgetSpritePtr CreateColoredSprite(Unigine::Math::vec4 color, int width = 5, int height = 5, const char* baseimagename = "empty.png")
    {
        auto white_sprite_sprite_virtual_path = Unigine::FileSystem::getVirtualPath(
          Unigine::FileSystem::resolvePartialVirtualPath(baseimagename));
        auto full_white_sprite = Unigine::Image::create(white_sprite_sprite_virtual_path);

        auto sprite = Unigine::WidgetSprite::create();
        sprite->setImage(full_white_sprite);
        sprite->setWidth(width);
        sprite->setHeight(height);
        sprite->setColor(color);

        return sprite;
    }

    inline Unigine::WidgetLabelPtr CreateWidgetLabel(std::string text, int width = 0, int height = 0)
    {
        auto label = Unigine::WidgetLabel::create(text.c_str());
        label->setWidth(width);
        label->setHeight(height);
        label->setFontColor(Unigine::Math::vec4_white);

        return label;
    }

    inline Unigine::WidgetEditLinePtr CreatePropertyEditLineLabel(int width  = 10,
                                                                  int height = 15,
                                                                  const int background = 1,
                                                                  const bool editable   = false,
                                                                  const Unigine::Math::vec4& borderColor = colors::BORDER_COLOR,
                                                                  const Unigine::Math::vec4& fillColor = colors::FILL_COLOR,
                                                                  const Unigine::Math::vec4& fontColor = Unigine::Math::vec4_white)
    {
        auto widget = Unigine::WidgetEditLine::create();

        widget->setEditable(editable);
        widget->setBackground(background);
        widget->setWidth(width);
        widget->setHeight(height);
        //widget->setBorderColor(borderColor);
        //widget->setBackgroundColor(fillColor);
        //widget->setFontColor(fontColor);

        return widget;
    }

    inline float UpdateNodeFromEditLine(Unigine::WidgetEditLinePtr widget, bool* valid, bool accept_zero)
    {
        try {
            const char* new_text = widget->getText();
            // Validating the text value entered into the Unigine Text Box Widget.
            //Verify input new_text entered is empty or returning nullptr
            bool        condition = new_text == nullptr || strlen(new_text) == 0 ;
            //Verify input new_text entered is Invalid or not
            if (!accept_zero)
                condition = condition || std::atof(new_text) == 0.0;

            if (condition) // entered value in the unigine text box is Invalid
            {
                *valid = false;
                return -1;
            }
            else // entered value in the unigine text box is valid
            {
                *valid = true;
                return atof(new_text); // converting string to float
            }
        }
        catch (...) {
            *valid = false;
             return -1;
        }
    }
}

namespace unigine2_node
{
    inline bool DoesNodeHaveSurface(Unigine::NodePtr node, std::string test_surface_name)
    {
        auto type = node->getType();

        if (type != Unigine::Node::OBJECT_MESH_STATIC)
        {
            return false;
        }

        Unigine::ObjectMeshStaticPtr static_mesh = Unigine::static_ptr_cast<Unigine::ObjectMeshStatic>(node);

        for (int i = 0; i < static_mesh->getNumSurfaces(); i++)
        {
            auto surface_name = std::string(static_mesh->getSurfaceName(i));

            if (surface_name == test_surface_name)
            {
                return true;
            }
        }

        return false;
    }

    inline int GetNumberOfSurfaces(Unigine::NodePtr node)
    {
        auto type = node->getType();

        if (type != Unigine::Node::OBJECT_MESH_STATIC)
        {
            return -1;
        }

        Unigine::ObjectMeshStaticPtr static_mesh = Unigine::static_ptr_cast<Unigine::ObjectMeshStatic>(node);

        return static_mesh->getNumSurfaces();
    }

    inline Unigine::NodePtr GetChildByName(Unigine::NodePtr parent, std::string name)
    {
        for (int i = 0; i < parent->getNumChildren(); i++)
        {
            auto child = parent->getChild(i);
            if (std::string(child->getName()) == name)
                return child;
        }

        return nullptr;
    }
}
