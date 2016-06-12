
#include "stdafx.hpp"

#include "gl_utils.hpp"
#include "Log.hpp"
#include "Keyboard.hpp"
#include "Pointer.hpp"
#include "StEngineControl.hpp"
#include "GIRenderer.hpp"
#include "basic_renderer.hpp"
#include "SphericalLight.hpp"
#include "ModelFactory.hpp"
#include "Camera.hpp"
#include "SurfaceFactory.hpp"
#include "Texture2D.hpp"
#include "Scene.hpp"
#include "Object.hpp"
#include "TextManager.hpp"
#include "AttributedString.hpp"
#include "RGB.hpp"
#include "Kelvin.hpp"
#include "Sphere.hpp"
#include "gpu_task.hpp"
#include "profiler.hpp"
#include "future_collection.hpp"
#include "resource_instance.hpp"

#include <imgui/imgui.h>
#include "debug_gui.hpp"

using namespace StE::Core;
using namespace StE::Text;

void display_loading_screen_until(StE::StEngineControl &ctx, StE::Text::TextManager *text_manager, int *w, int *h, std::function<bool()> &&lambda) {
	StE::Graphics::basic_renderer basic_renderer(ctx);

	auto footer_text = text_manager->create_renderer();
	auto footer_text_task = make_gpu_task("footer_text", footer_text.get(), nullptr);

	auto title_text = text_manager->create_renderer();
	auto title_text_task = make_gpu_task("title_text", title_text.get(), nullptr);

	ctx.set_renderer(&basic_renderer);
	basic_renderer.add_task(title_text_task);
	basic_renderer.add_task(footer_text_task);

	while (true) {
		using namespace StE::Text::Attributes;

		AttributedWString str = center(stroke(blue_violet, 2)(purple(vvlarge(b(L"Global Illumination\n")))) +
									azure(large(L"Loading...\n")) +
									orange(regular(L"By Shlomi Steinberg")));
		auto total_vram = std::to_wstring(ctx.gl()->meminfo_total_available_vram() / 1024);
		auto free_vram = std::to_wstring(ctx.gl()->meminfo_free_vram() / 1024);

		auto workers_active = ctx.scheduler().get_thread_pool()->get_active_workers_count();
		auto workers_sleep = ctx.scheduler().get_thread_pool()->get_sleeping_workers_count();
		auto pending_requests = ctx.scheduler().get_thread_pool()->get_pending_requests_count();

		title_text->set_text({ *w / 2, *h / 2 + 100 }, str);
		footer_text->set_text({ 10, 50 },
							line_height(32)(vsmall(b(blue_violet(free_vram) + L" / " + stroke(red, 1)(dark_red(total_vram)) + L" MB")) + L"\n" +
											vsmall(b(L"Thread pool workers: ") +
													olive(std::to_wstring(workers_active)) + 	L" busy, " +
													olive(std::to_wstring(workers_sleep)) + 	L" sleeping | " +
													orange(std::to_wstring(pending_requests) +	L" pending requests"))));

		if (!ctx.run_loop() || !lambda())
			break;
	}
}

auto create_light_object(StE::Graphics::Scene *scene, const glm::vec3 &light_pos, StE::Graphics::SphericalLight *light, std::vector<std::unique_ptr<StE::Graphics::material>> &materials, std::vector<std::unique_ptr<StE::Graphics::material_layer>> &layers) {
	std::unique_ptr<StE::Graphics::Sphere> sphere = std::make_unique<StE::Graphics::Sphere>(20, 20);
	(*sphere) *= light->get_radius();
	auto light_obj = std::make_shared<StE::Graphics::Object>(std::move(sphere));

	light_obj->set_model_transform(glm::mat4x3(glm::translate(glm::mat4(), light_pos)));

	gli::texture2d light_color_tex{ gli::format::FORMAT_RGB8_UNORM_PACK8, { 1, 1 }, 1 };
	auto c = light->get_diffuse();
	*reinterpret_cast<glm::u8vec3*>(light_color_tex.data()) = glm::u8vec3(c.r * 255.5f, c.g * 255.5f, c.b * 255.5f);
	
	auto layer = scene->scene_properties().material_layers_storage().allocate_layer();
	auto mat = scene->scene_properties().materials_storage().allocate_material(layer.get());
	layer->set_basecolor_map(std::make_unique<StE::Core::Texture2D>(light_color_tex, false));
	mat->set_emission(c * light->get_luminance());

	light_obj->set_material(mat.get());

	scene->object_group().add_object(light_obj);

	materials.push_back(std::move(mat));
	layers.push_back(std::move(layer));

	return light_obj;
}

void add_scene_lights(StE::Graphics::Scene &scene, std::vector<std::unique_ptr<StE::Graphics::light>> &lights, std::vector<std::unique_ptr<StE::Graphics::material>> &materials, std::vector<std::unique_ptr<StE::Graphics::material_layer>> &layers) {
	for (auto &v : { glm::vec3{ -622, 645, -310},
					 glm::vec3{  124, 645, -310},
					 glm::vec3{  497, 645, -310},
					 glm::vec3{ -242, 153, -310},
					 glm::vec3{  120, 153, -310},
					 glm::vec3{  124, 645,  552},
					 glm::vec3{  497, 645,  552},
					 glm::vec3{-1008, 153,  552},
					 glm::vec3{ -242, 153,  552},
					 glm::vec3{  120, 153,  552},
					 glm::vec3{  885, 153,  552} }) {
		auto wall_lamp = scene.scene_properties().lights_storage().allocate_light<StE::Graphics::SphericalLight>(5000.f, StE::Graphics::Kelvin(1800), v, 2.f);
		create_light_object(&scene, v, wall_lamp.get(), materials, layers);

		lights.push_back(std::move(wall_lamp));
	}
}


#ifdef _MSC_VER
int CALLBACK WinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPSTR     lpCmdLine,
					 int       nCmdShow)
#else
int main()
#endif
{
	/*
	 *	Create logger
	 */

	StE::Log logger("Global Illumination");
	logger.redirect_std_outputs();
	ste_log_set_global_logger(&logger);
	ste_log() << "Simulation is running";


	/*
	 *	Create GL context and window
	 */

	int w = 1920;
	int h = w * 9 / 16;
	constexpr float clip_near = 1.f;
	float fovy = glm::pi<float>() * .225f;

	GL::gl_context::context_settings settings;
	settings.vsync = false;
	settings.fs = false;
	StE::StEngineControl ctx(std::make_unique<GL::gl_context>(settings, "Shlomi Steinberg - Global Illumination", glm::i32vec2{ w, h }));// , gli::FORMAT_RGBA8_UNORM));
	ctx.set_clipping_planes(clip_near);
	ctx.set_fov(fovy);

	using ResizeSignalConnectionType = StE::StEngineControl::framebuffer_resize_signal_type::connection_type;
	std::shared_ptr<ResizeSignalConnectionType> resize_connection;
	resize_connection = std::make_shared<ResizeSignalConnectionType>([&](const glm::i32vec2 &size) {
		w = size.x;
		h = size.y;
	});
	ctx.signal_framebuffer_resize().connect(resize_connection);


	/*
	 *	Connect input handlers
	 */

	bool running = true;
	bool mouse_down = false;
	auto keyboard_listner = std::make_shared<decltype(ctx)::hid_keyboard_signal_type::connection_type>(
		[&](StE::HID::keyboard::K key, int scanline, StE::HID::Status status, StE::HID::ModifierBits mods) {
		using namespace StE::HID;
		auto time_delta = ctx.time_per_frame().count();

		if (status != Status::KeyDown)
			return;

		if (key == keyboard::K::KeyESCAPE)
			running = false;
		if (key == keyboard::K::KeyPRINT_SCREEN || key == keyboard::K::KeyF12)
			ctx.capture_screenshot();
	});
	auto pointer_button_listner = std::make_shared<decltype(ctx)::hid_pointer_button_signal_type::connection_type>(
		[&](StE::HID::pointer::B b, StE::HID::Status status, StE::HID::ModifierBits mods) {
		using namespace StE::HID;

		mouse_down = b == pointer::B::Left && status == Status::KeyDown;
	});
	ctx.hid_signal_keyboard().connect(keyboard_listner);
	ctx.hid_signal_pointer_button().connect(pointer_button_listner);


	/*
	 *	Create text manager and choose default font
	 */

	auto font = StE::Text::Font("Data/ArchitectsDaughter.ttf");
	StE::Resource::resource_instance<StE::Text::TextManager> text_manager(ctx, font);


	/*
	 *	Create camera
	 */

	StE::Graphics::Camera camera;
	camera.set_position({ 25.8, 549.07, -249.2 });
	camera.lookat({ -5.4, 532.5, -228.71 });


	/*
	 *	Create and load scene object and GI renderer
	 */

	StE::Resource::resource_instance<StE::Graphics::Scene> scene(ctx);
	StE::Resource::resource_instance<StE::Graphics::GIRenderer> renderer(ctx, &camera, &scene.get());


	/*
	 *	Start loading resources and display loading screen
	 */

	std::vector<std::unique_ptr<StE::Graphics::light>> lights;
	std::vector<std::unique_ptr<StE::Graphics::material>> materials;
	std::vector<std::unique_ptr<StE::Graphics::material_layer>> material_layers;

	StE::task_future_collection<void> loading_futures;

	const glm::vec3 light0_pos{ -700.6, 138, -70 };
	const glm::vec3 light1_pos{ 200, 550, 170 };
	auto light0 = scene.get().scene_properties().lights_storage().allocate_light<StE::Graphics::SphericalLight>(8000.f, StE::Graphics::Kelvin(2000), light0_pos, 3.f);
	auto light1 = scene.get().scene_properties().lights_storage().allocate_light<StE::Graphics::SphericalLight>(20000.f, StE::Graphics::Kelvin(7000), light1_pos, 5.f);
	auto light0_obj = create_light_object(&scene.get(), light0_pos, light0.get(), materials, material_layers);
	auto light1_obj = create_light_object(&scene.get(), light1_pos, light1.get(), materials, material_layers);

	add_scene_lights(scene.get(), lights, materials, material_layers);

	loading_futures.insert(StE::Resource::ModelFactory::load_model_async(ctx,
																		 R"(Data/models/crytek-sponza/sponza.obj)",
																		 &scene.get().object_group(),
																		 &scene.get().scene_properties(),
																		 2.5f,
																		 materials,
																		 material_layers));
	
	std::vector<std::unique_ptr<StE::Graphics::material>> ball_materials;
	std::vector<std::unique_ptr<StE::Graphics::material_layer>> ball_layers;
	std::vector<std::shared_ptr<StE::Graphics::Object>> ball_objects;
	loading_futures.insert(StE::Resource::ModelFactory::load_model_async(ctx,
																		 R"(Data/models/ball/Football.obj)",
																		 &scene.get().object_group(),
																		 &scene.get().scene_properties(),
																		 2.5f,
																		 ball_materials,
																		 ball_layers,
																		 &ball_objects));
	loading_futures.insert(ctx.scheduler().schedule_now([&]() {
		renderer.wait();
	}));

	display_loading_screen_until(ctx, &text_manager.get(), &w, &h, [&]() -> bool {
		return running && !loading_futures.ready_all();
	});


	/*
	 *	Create debug view window and material editor
	 */

	std::unique_ptr<StE::Graphics::profiler> gpu_tasks_profiler = std::make_unique<StE::Graphics::profiler>();
	renderer.get().attach_profiler(gpu_tasks_profiler.get());
	std::unique_ptr<StE::Graphics::debug_gui> debug_gui_dispatchable = std::make_unique<StE::Graphics::debug_gui>(ctx, gpu_tasks_profiler.get(), font, &camera);

	auto ball = ball_objects.back().get();
	auto ball_model_transform = glm::translate(glm::mat4(), glm::vec3{ .0f, 100.f, .0f });
	ball->set_model_transform(glm::mat4x3(ball_model_transform));

	constexpr int layers_count = 3;
	
	auto mat = std::move(*ball_materials.begin());
	std::unique_ptr<StE::Graphics::material_layer> layers[layers_count];
	layers[0] = std::move(*ball_layers.begin());

	StE::Graphics::RGB base_color[3];
	float roughness[3];
	float anisotropy[3];
	float metallic[3];
	float index_of_refraction[3];
	float sheen[3];
	float sheen_power[3];
	float thickness[3];

	for (int i = 0; i < layers_count; ++i) {
		if (i > 0) {
			layers[i] = scene.get().scene_properties().material_layers_storage().allocate_layer();
			layers[i-1]->set_next_layer(layers[i].get());
		}

		gli::texture2d base_color_tex{ gli::format::FORMAT_RGB8_UNORM_PACK8, { 1, 1 }, 1 }; 
		*reinterpret_cast<glm::u8vec3*>(base_color_tex.data()) = glm::u8vec3(255); 
		layers[i]->set_basecolor_map(std::make_unique<StE::Core::Texture2D>(base_color_tex, false));

		base_color[i] = {1,1,1};
		roughness[i] = .5f;
		anisotropy[i] = 0;
		metallic[i] = 0;
		index_of_refraction[i] = 1.5f;
		sheen[i] = 0;
		sheen_power[i] = 0;
		thickness[i] = 0;
	}

	debug_gui_dispatchable->add_custom_gui([&](const glm::ivec2 &bbsize) {
		ImGui::SetNextWindowPos(ImVec2(20,bbsize.y - 400), ImGuiSetCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(120,400), ImGuiSetCond_FirstUseEver);
		if (ImGui::Begin("Material", nullptr)) {
			for (int i = 0; i < layers_count; ++i) {
				ImGui::Text((std::string("Layer ") + std::to_string(i)).data());

				ImGui::SliderFloat("R ##value", &base_color[i].R(), .0f, 1.f);
				ImGui::SliderFloat("G ##value", &base_color[i].G(), .0f, 1.f);
				ImGui::SliderFloat("B ##value", &base_color[i].B(), .0f, 1.f);
				ImGui::SliderFloat("Roughness ##value", &roughness[i], .0f, 1.f);
				ImGui::SliderFloat("Anisotropy ##value", &anisotropy[i], -1.f, 1.f);
				ImGui::SliderFloat("Metallic ##value", &metallic[i], .0f, 1.f);
				ImGui::SliderFloat("IOR ##value", &index_of_refraction[i], 1.f, 15.f);
				ImGui::SliderFloat("Sheen ##value", &sheen[i], .0f, 1.f);
				ImGui::SliderFloat("Sheen Power ##value", &sheen_power[i], .0f, 1.f);
				ImGui::SliderFloat("Thickness ##value", &thickness[i], .0f, .1f);
			}
		}

		ImGui::End();
		
		for (int i = 0; i < layers_count; ++i) {
			auto t = glm::u8vec3(base_color[i].R() * 255.5f, base_color[i].G() * 255.5f, base_color[i].B() * 255.5f);
			layers[i]->get_basecolor_map()->clear(&t);
			if (layers[i]->get_roughness() != roughness[i])
				layers[i]->set_roughness(roughness[i]);
			if (layers[i]->get_anisotropy() != anisotropy[i])
				layers[i]->set_anisotropy(anisotropy[i]);
			if (layers[i]->get_metallic() != metallic[i])
				layers[i]->set_metallic(metallic[i]);
			if (layers[i]->get_index_of_refraction() != index_of_refraction[i])
				layers[i]->set_index_of_refraction(index_of_refraction[i]);
			if (layers[i]->get_sheen() != sheen[i])
				layers[i]->set_sheen(sheen[i]);
			if (layers[i]->get_sheen_power() != sheen_power[i])
				layers[i]->set_sheen_power(sheen_power[i]);
			if (layers[i]->get_layer_thickness() != thickness[i])
				layers[i]->set_layer_thickness(thickness[i]);
		}
	});


	/*
	 *	Switch to GI renderer and start render loop
	 */

	ctx.set_renderer(&renderer.get());

	auto footer_text = text_manager.get().create_renderer();
	auto footer_text_task = StE::Graphics::make_gpu_task("footer_text", footer_text.get(), nullptr);

	renderer.get().add_gui_task(footer_text_task);
	renderer.get().add_gui_task(StE::Graphics::make_gpu_task("debug_gui", debug_gui_dispatchable.get(), nullptr));

	glm::ivec2 last_pointer_pos;
	float time = 0;
	while (running) {
		if (ctx.window_active()) {
			auto time_delta = ctx.time_per_frame().count();

			using namespace StE::HID;
			constexpr float movement_factor = 155.f;
			if (ctx.get_key_status(keyboard::K::KeyW) == Status::KeyDown)
				camera.step_forward(time_delta*movement_factor);
			if (ctx.get_key_status(keyboard::K::KeyS) == Status::KeyDown)
				camera.step_backward(time_delta*movement_factor);
			if (ctx.get_key_status(keyboard::K::KeyA) == Status::KeyDown)
				camera.step_left(time_delta*movement_factor);
			if (ctx.get_key_status(keyboard::K::KeyD) == Status::KeyDown)
				camera.step_right(time_delta*movement_factor);

			constexpr float rotation_factor = .09f;
			glm::vec2(.0f);
			auto pp = ctx.get_pointer_position();
			if (mouse_down && !debug_gui_dispatchable->is_gui_active()) {
				auto diff_v = static_cast<glm::vec2>(last_pointer_pos - pp) * time_delta * rotation_factor;
				camera.pitch_and_yaw(-diff_v.y, diff_v.x);
			}
			last_pointer_pos = pp;
		}

		float angle = time * glm::pi<float>() / 2.5f;
		glm::vec3 lp = light0_pos + glm::vec3(glm::sin(angle) * 3, 0, glm::cos(angle)) * 115.f;
		light0->set_position(lp);
		light0_obj->set_model_transform(glm::mat4x3(glm::translate(glm::mat4(), lp)));

		{
			using namespace StE::Text::Attributes;

			static unsigned tpf_count = 0;
			static float total_tpf = .0f;
			total_tpf += ctx.time_per_frame().count();
			++tpf_count;
			static float tpf = .0f;
			if (tpf_count % 5 == 0) {
				tpf = total_tpf / 5.f;
				total_tpf = .0f;
			}

			auto total_vram = std::to_wstring(ctx.gl()->meminfo_total_available_vram() / 1024);
			auto free_vram = std::to_wstring(ctx.gl()->meminfo_free_vram() / 1024);

			footer_text->set_text({ 10, 50 }, line_height(28)(vsmall(b(stroke(dark_magenta, 1)(red(std::to_wstring(tpf * 1000.f))))) + L" ms\n" +
															  vsmall(b((blue_violet(free_vram) + L" / " + stroke(red, 1)(dark_red(total_vram)) + L" MB")))));
		}

		time += ctx.time_per_frame().count();
		if (!ctx.run_loop()) break;
	}

	return 0;
}
