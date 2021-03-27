#include "game_hooks.hpp"

bool setupDone = false;

void(__thiscall *EditorPauseLayer_saveLevel)(EditorPauseLayer *);
EditorPauseLayer *(__thiscall *EditorPauseLayer_constructor)(void *);

// this is in seconds
constexpr int AUTOSAVE_DELAY = 300;

template <typename T> T *offset_from_base(void *struct_ptr, int addr) {
  return reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(struct_ptr) + addr);
}

class EditorUI_extObj : public cocos2d::CCObject
{
public:
  int getSaveTimer() const {
    return this->_saveTimer;
  }
  void setSaveTimer(int n_timer) { this->_saveTimer = n_timer; }

  void incrementSaveTimer() { this->_saveTimer++; }
  void resetSaveTimer() { this->_saveTimer = 0; }

protected:
  int _saveTimer;
};

class EditorUI_ext : public cocos2d::CCLayer
{
public:
  // fun fact, the editor doesn't actually do saving. is that fun?
  void onSaveInterval(float dt)
  {
    auto user_object = dynamic_cast<EditorUI_extObj *>(this->getUserObject());
    if (user_object != nullptr) {
      user_object->incrementSaveTimer();

      auto current_timer_val = user_object->getSaveTimer();

      auto autosave_label =
          reinterpret_cast<cocos2d::CCLabelBMFont *>(this->getChildByTag(232));
      if (autosave_label == nullptr) {
        return;
      }

      if ((AUTOSAVE_DELAY - current_timer_val) <= 10 &&
              AUTOSAVE_DELAY != current_timer_val) {
        auto save_string = cocos2d::CCString::createWithFormat("Autosave in %i sec", (AUTOSAVE_DELAY - current_timer_val));

        autosave_label->setString(save_string->getCString());
        autosave_label->setVisible(true);
      }
      else if (AUTOSAVE_DELAY == current_timer_val) {
        autosave_label->setString("Autosaving...");
        autosave_label->setVisible(true);

        // this is when we do actual save
        // don't ever repeat this
        auto editor_layer = offset_from_base<LevelEditorLayer *>(this, 0x290);

        // please don't ever do this
        auto pause_layer_memory = malloc(0x1C0);
        if (pause_layer_memory == nullptr) {
          return;
        }

        auto pause_layer = EditorPauseLayer_constructor(pause_layer_memory);
        auto pause_editor_reference =
            offset_from_base<LevelEditorLayer *>(pause_layer, 0x1AC);

        // like i'm not kidding _never_ do this
        *pause_editor_reference = *editor_layer;

        // there's so much undefined behavior there it's no longer funny
        EditorPauseLayer_saveLevel(pause_layer);

        // memory management is no joke
        free(pause_layer_memory);

        user_object->resetSaveTimer();
      }
      else {
        autosave_label->setVisible(false);
      }
    }
  }
};

bool (__thiscall *EditorUI_init_O)(EditorUI *, LevelEditorLayer *);
bool __fastcall EditorUI_init_H(EditorUI *self, void *, LevelEditorLayer *editor_layer) {
  EditorUI_init_O(self, editor_layer);

  auto label = cocos2d::CCLabelBMFont::create("Autosave in 10 seconds", "bigFont.fnt");
  self->addChild(label, 10, 232);

  auto director = cocos2d::CCDirector::sharedDirector();
  auto size = director->getWinSize();
  auto top = director->getScreenTop();

  cocos2d::CCPoint label_pos(size.width / 2, top - 45.0f);
  self->convertToNodeSpace(label_pos);
  label->setPosition(label_pos);
  label->setScale(0.25f);
  label->setVisible(false);

  self->setUserObject(new EditorUI_extObj());
  self->schedule(static_cast<cocos2d::SEL_SCHEDULE>(&EditorUI_ext::onSaveInterval), 1.0f);

  return true;
}

// no need to export this, not putting in .h
struct game_hook {
  void *orig_addr;
  void *hook_fn;
  void **orig_fn;
};

#define CREATE_HOOK(ADDRESS, NAME)                                             \
  {                                                                            \
    reinterpret_cast<void *>(ADDRESS), reinterpret_cast<void *>(&(NAME##_H)),  \
        reinterpret_cast<void **>(&(NAME##_O))                                 \
  }

#define CREATE_GD_HOOK(ADDRESS, NAME)                                          \
  CREATE_HOOK(offset_from_base<void>(gd_handle, ADDRESS), NAME)

#define RESOLVE_GD_FUNC(ADDRESS, NAME)                                         \
  NAME = reinterpret_cast<decltype(NAME)>(reinterpret_cast<uintptr_t>(gd_handle) + ADDRESS)

void doTheHook() {
  if (auto status = MH_Initialize(); status != MH_OK) {
    return;
  }

  HMODULE gd_handle = GetModuleHandleA("GeometryDash.exe");
  if (!gd_handle) {
    return;
  }

  // wall of hooks
  std::array<game_hook, 1> hooks{{
      CREATE_GD_HOOK(0x76310, EditorUI_init),
  }};

  RESOLVE_GD_FUNC(0x75010, EditorPauseLayer_saveLevel);
  RESOLVE_GD_FUNC(0x72F10, EditorPauseLayer_constructor);

  for (const auto &hook : hooks) {
    MH_CreateHook(hook.orig_addr, hook.hook_fn, hook.orig_fn);
    MH_EnableHook(hook.orig_addr);
  }
}
