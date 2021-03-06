#include "Engine.h"

#include "../audio/Audio.h"
#include "../component/Camera.h"
#include "../component/PostProcessComponent.h"
#include "CullVolume.h"
#include "../dev/profiling.h"
#include "../hash.h"
#include "../rendering/Pipeline.h"
#include "../rendering/shader_lib.h"
#include "Resource.h"
#include "../system/Device.h"
#include "../system/Window.h"
#include "../stream/CFileStream.h"

using namespace L;

Array<void(*)()> Engine::_parallel_updates, Engine::_updates, Engine::_sub_updates, Engine::_late_updates;
Array<void(*)(const Camera&, const RenderPass&)> Engine::_renders;
Array<void(*)(void* frames, uint32_t frame_count)> Engine::_audio_renders;
Array<void(*)(const Camera&)> Engine::_guis;
Array<void(*)(const L::Window::Event&)> Engine::_win_events;
Array<void(*)(const Device::Event&)> Engine::_dev_events;
Array<Engine::DeferredAction> Engine::_deferred_actions;
Timer Engine::_timer;
const Time Engine::_sub_delta(0, 10);
L::Time Engine::_real_delta_time, Engine::_delta_time, Engine::_accumulator(0), Engine::_average_frame_work_duration, Engine::_max_frame_work_duration;
L::Time Engine::_frame_work_durations[64];
float Engine::_real_delta_seconds, Engine::_delta_seconds, Engine::_sub_delta_seconds(Engine::_sub_delta.fSeconds()), Engine::_timescale(1.f);
uint32_t Engine::_frame(0);

void Engine::update() {
  L_SCOPE_MARKER("Engine update");
  _real_delta_time = _timer.frame();
  _delta_time = min(_real_delta_time*_timescale, Time(0, 100)); // Cap delta time to avoid weird behaviour
  _real_delta_seconds = _real_delta_time.fSeconds();
  _delta_seconds = _delta_time.fSeconds();
  ScriptGlobal("real_delta") = _real_delta_seconds;
  ScriptGlobal("delta") = _delta_seconds;
  ScriptGlobal("avg_frame_work_duration") = _average_frame_work_duration;
  ScriptGlobal("max_frame_work_duration") = _max_frame_work_duration;

  {
    L_SCOPE_MARKER("Parallel updates");
    for(const auto& parallel_update : _parallel_updates) {
      TaskSystem::push([](void* f) {
        ((void(*)())f)();
      }, (void*)parallel_update);
    }
    TaskSystem::join();
  }
  {
    L_SCOPE_MARKER("Window events");
    for(const Window::Event& e : Window::events())
      for(const auto& event : _win_events)
        event(e);
    Window::flush_events();
  }
  {
    L_SCOPE_MARKER("Device events");
    Device::Event e;
    while(Device::new_event(e))
      for(const auto& event : _dev_events)
        event(e);
  }
  {
    L_SCOPE_MARKER("Updates");
    for(const auto& update : _updates)
      update();
  }
  {
    L_SCOPE_MARKER("Sub updates");
    _accumulator += _delta_time;
    _sub_delta_seconds = _sub_delta.fSeconds();
    while(_sub_delta < _accumulator) {
      for(const auto& sub_update : _sub_updates)
        sub_update();
      _accumulator -= _sub_delta;
    }
  }
  {
    L_SCOPE_MARKER("Late updates");
    for(const auto& late_update : _late_updates)
      late_update();
  }

  Entity::flush_destroy_queue();

  const bool render_this_frame(ComponentPool<Camera>::size()>0 && Window::opened());
  if(render_this_frame) {
    L_SCOPE_MARKER("Graphics rendering");
    VkCommandBuffer cmd_buffer(Vulkan::begin_render_command_buffer());
    ComponentPool<Camera>::iterate([&](Camera& camera) {
      camera.prerender(cmd_buffer);
      CullVolume::cull(camera);
      camera.geometry_buffer().begin(cmd_buffer);
      for(auto render : _renders)
        render(camera, camera.geometry_buffer().render_pass());
      camera.geometry_buffer().end(cmd_buffer);

      camera.light_buffer().begin(cmd_buffer);
      for(auto render : _renders)
        render(camera, camera.light_buffer().render_pass());

      {
        Entity* cam_entity(camera.entity());
        Array<PostProcessComponent*> post_processes;
        cam_entity->components(post_processes);
        for(PostProcessComponent* post_process : post_processes) {
          post_process->render(camera, camera.light_buffer().render_pass());
        }
      }
      camera.light_buffer().end(cmd_buffer);
    });
    Vulkan::begin_present_pass();
    ComponentPool<Camera>::iterate([](Camera& camera) {
      camera.present();
      for(auto gui : _guis)
        gui(camera);
    });
    Vulkan::end_present_pass();
  }

  {
    L_SCOPE_MARKER("Audio rendering");
    void* frames;
    uint32_t frame_count(0);
    Audio::acquire_buffer(frames, frame_count);
    if(frame_count) {
      for(const auto& audio_render : _audio_renders)
        audio_render(frames, frame_count);
      Audio::commit_buffer();
    }
  }

  { // Flush deferred actions
    L_SCOPE_MARKER("Deferred actions");
    for(uintptr_t i(0); i<_deferred_actions.size(); i++) {
      const DeferredAction& deferred_action(_deferred_actions[i]);
      deferred_action.func(deferred_action.data);
    }
    _deferred_actions.clear();
  }

  // Compute work duration
  _frame_work_durations[_frame%L_COUNT_OF(_frame_work_durations)] = _timer.since();
  _average_frame_work_duration = _max_frame_work_duration = 0;
  for(const Time& duration : _frame_work_durations) {
    _average_frame_work_duration += duration;
    _max_frame_work_duration = max(_max_frame_work_duration, duration);
  }
  _average_frame_work_duration /= L_COUNT_OF(_frame_work_durations);

  if(render_this_frame) {
    Vulkan::end_render_command_buffer();
  }
  _frame++;
}
void Engine::clear() {
  Entity::clear();
}
