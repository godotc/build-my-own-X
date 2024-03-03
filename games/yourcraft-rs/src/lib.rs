mod state;
mod vertex;

#[cfg(target_arch = "wasm32")]
use wasm_bindgen::prelude::*;

use crate::state::State;
use winit::{
    dpi::PhysicalSize,
    event::{self, *},
    event_loop::{EventLoop, EventLoopWindowTarget},
    keyboard::{self, NamedKey},
    window::{Window, WindowBuilder, WindowId},
};

#[cfg_attr(target_arch = "wasm32", wasm_bindgen(start))]
pub async fn run() {
    // wasm
    cfg_if::cfg_if! {
        if #[cfg(target_arch = "wasm32")]{
            std::panic::set_hook(Box::new(console_error_panic_hook::hook));
            console_log::init_with_level(log::Level::Warn).expect("Couldn't initialize logger");
        }else{
            env_logger::init();
        }
    }

    let event_loop = EventLoop::new().unwrap();
    let window = WindowBuilder::new().build(&event_loop).unwrap();

    #[cfg(target_arch = "wasm32")]
    {
        // Winit prevents sizing with CSS, so we have to set
        // the size manually when on web.
        use winit::dpi::PhysicalSize;
        window.set_inner_size(PhysicalSize::new(450, 400));

        use winit::platform::web::WindowExtWebSys;
        web_sys::window()
            .and_then(|win| win.document())
            .and_then(|doc| {
                let dst = doc.get_element_by_id("wgpu_test")?;
                let canvas = web_sys::Element::from(window.canvas());
                dst.append_child(&canvas).ok()?;
                Some(())
            })
            .expect("Couldn't append canvas to document body.");
    }

    main_loop(event_loop, window).await;
}

async fn main_loop(event_loop: EventLoop<()>, window: winit::window::Window) {
    let mut state = State::new(&window).await;

    event_loop.run(move |event, event_loop_target| match event {
        // event_loop.run(move |event, _, control_flow| match event {
        Event::WindowEvent {
            event: ref window_event,
            window_id,
        } => handle_window_event(
            &mut state,
            &window,
            &event_loop_target,
            window_event,
            window_id,
        ),

        // Event::MainEventsCleared => {
        //     window.request_redraw();
        // }
        _ => {}
    });
}

fn handle_window_event(
    state: &mut State,
    window: &Window,
    event_loop: &EventLoopWindowTarget<()>,
    window_event: &winit::event::WindowEvent,
    window_id: WindowId,
) {
    if window_id != window.id() {
        return;
    }

    let mut event_handled = false;
    match window_event {
        WindowEvent::RedrawRequested => {
            if window_id == window.id() {
                state.update();
                match state.render() {
                    Ok(_) => {}
                    // lost surfce context
                    Err(wgpu::SurfaceError::Lost) => state.resize(state.size),
                    // exit when out of memory
                    Err(wgpu::SurfaceError::OutOfMemory) => event_loop.exit(),
                    Err(e) => println!("{:?}", e),
                }
                {}
            }
        }

        WindowEvent::Resized(physical_size) => {
            state.resize(*physical_size);
        }

        WindowEvent::ScaleFactorChanged {
            scale_factor,
            inner_size_writer,
            ..
        } => {
            let size = window.inner_size();
            state.resize(PhysicalSize {
                width: (size.width as f64 * scale_factor) as u32,
                height: (size.height as f64 * scale_factor) as u32,
            });
        }

        WindowEvent::CursorMoved {
            device_id: _,
            position,
        } => {
            let r = position.x / state.size.width as f64;
            let g = position.y / state.size.height as f64;
            let b = r / g;
            state.clear_color = wgpu::Color {
                r,
                g,
                b,
                a: state.clear_color.a,
            };
        }

        WindowEvent::MouseWheel {
            device_id: _, // DeviceId(Wayland(DeviceId))
            delta,        // LineDelta(0.0, -1.0)
            phase: _,     // Moved
        } => {
            let weight = 0.03
                * match delta {
                    MouseScrollDelta::LineDelta(_, y) => y,
                    _ => &0.0,
                };
            state.clear_color.a += weight as f64;

            println!("New apparent is {}", state.clear_color.a);
        }

        WindowEvent::CloseRequested
        | WindowEvent::KeyboardInput {
            event:
                KeyEvent {
                    state: ElementState::Pressed,
                    logical_key: keyboard::Key::Named(NamedKey::Escape),
                    ..
                },
            ..
        } => event_loop.exit(),

        WindowEvent::KeyboardInput {
            device_id,
            event,
            is_synthetic,
        } => match event.logical_key {
            keyboard::Key::Named(name) => match name {
                NamedKey::Space => {
                    println!("Pressed space bar...");
                    state.pipeline_index =
                        (state.pipeline_index + 1) % state.render_pipelines.len();
                }
                _ => {}
            },
            keyboard::Key::Character(_) => todo!(),
            keyboard::Key::Unidentified(_) => todo!(),
            keyboard::Key::Dead(_) => todo!(),
        },

        _ => {}
    }

    if !event_handled {
        let _ = state.input(&window_event);
    }
}
