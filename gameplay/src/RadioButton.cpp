#include "Base.h"
#include "RadioButton.h"

namespace gameplay
{
static std::vector<RadioButton*> __radioButtons;

RadioButton::RadioButton() : _selected(false)
{
}

RadioButton::RadioButton(const RadioButton& copy)
{
    // Hidden.
}

RadioButton::~RadioButton()
{
    // Remove this RadioButton from the global list.
    std::vector<RadioButton*>::iterator it = std::find(__radioButtons.begin(), __radioButtons.end(), this);
    if (it != __radioButtons.end())
    {
        __radioButtons.erase(it);
    }
}

RadioButton* RadioButton::create(Theme::Style* style, Properties* properties)
{
    RadioButton* radioButton = new RadioButton();
    radioButton->initialize(style, properties);

    properties->getVector2("imageSize", &radioButton->_imageSize);

    if (properties->getBool("selected"))
    {
        RadioButton::clearSelected(radioButton->_groupId);
        radioButton->_selected = true;
    }

    const char* groupId = properties->getString("group");
    if (groupId)
    {
        radioButton->_groupId = groupId;
    }

    __radioButtons.push_back(radioButton);

    return radioButton;
}

bool RadioButton::isSelected() const
{
    return _selected;
}

void RadioButton::setImageSize(float width, float height)
{
    _imageSize.set(width, height);
}

const Vector2& RadioButton::getImageSize() const
{
    return _imageSize;
}

void RadioButton::addListener(Control::Listener* listener, int eventFlags)
{
    if ((eventFlags & Listener::TEXT_CHANGED) == Listener::TEXT_CHANGED)
    {
        assert("TEXT_CHANGED event is not applicable to RadioButton.");
        eventFlags &= ~Listener::TEXT_CHANGED;
    }

    Control::addListener(listener, eventFlags);
}

bool RadioButton::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex)
{
    if (!isEnabled())
    {
        return false;
    }

    switch (evt)
    {
    case Touch::TOUCH_RELEASE:
        {
            if (_state == Control::ACTIVE)
            {
                if (x > 0 && x <= _clipBounds.width &&
                    y > 0 && y <= _clipBounds.height)
                {
                    if (!_selected)
                    {
                        RadioButton::clearSelected(_groupId);
                        _selected = true;
                        notifyListeners(Listener::VALUE_CHANGED);
                    }
                }
            }
        }
        break;
    }

    return Button::touchEvent(evt, x, y, contactIndex);
}

void RadioButton::clearSelected(const std::string& groupId)
{
    std::vector<RadioButton*>::const_iterator it;
    for (it = __radioButtons.begin(); it < __radioButtons.end(); it++)
    {
        RadioButton* radioButton = *it;
        if (groupId == radioButton->_groupId)
        {
            radioButton->_selected = false;
            radioButton->_dirty = true;
            radioButton->notifyListeners(Listener::VALUE_CHANGED);
        }
    }
}

void RadioButton::update(const Rectangle& clip)
{
    Label::update(clip);

    Vector2 size;
    if (_imageSize.isZero())
    {
        if (_selected)
        {
            const Rectangle& selectedRegion = getImageRegion("selected", _state);
            size.set(selectedRegion.width, selectedRegion.height);
        }
        else
        {
            const Rectangle& unselectedRegion = getImageRegion("unselected", _state);
            size.set(unselectedRegion.width, unselectedRegion.height);
        }
    }
    else
    {
        size.set(_imageSize);
    }
    float iconWidth = size.x;

    _textBounds.x += iconWidth + 5;
    _textBounds.width -= iconWidth + 5;

    if (_selected)
    {
        _image = getImage("selected", _state);
    }
    else
    {
        _image = getImage("unselected", _state);
    }
}

void RadioButton::drawImages(SpriteBatch* spriteBatch, const Rectangle& clip)
{
    // Left, v-center.
    // TODO: Set an alignment for radio button images.
    const Theme::Border& border = getBorder(_state);
    const Theme::Padding padding = getPadding();
    
    const Rectangle& region = _image->getRegion();
    const Theme::UVs& uvs = _image->getUVs();
    Vector4 color = _image->getColor();
    color.w *= _opacity;

    Vector2 size;
    if (_imageSize.isZero())
    {
        size.set(region.width, region.height);
    }
    else
    {
        size.set(_imageSize);
    }

    Vector2 pos(clip.x + _bounds.x + border.left + padding.left,
        clip.y + _bounds.y + (_clipBounds.height - border.bottom - padding.bottom) / 2.0f - size.y / 2.0f);

    spriteBatch->draw(pos.x, pos.y, size.x, size.y, uvs.u1, uvs.v1, uvs.u2, uvs.v2, color, _clip);
}

}