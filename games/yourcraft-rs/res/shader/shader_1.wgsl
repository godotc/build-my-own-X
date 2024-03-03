
struct VertexInput{
	@builtin(vertex_index) vertex_index: u32,
};

struct VertexOutput{
	@builtin(position) clip_position: vec4f,
	@location(0) color: vec4f,
};

@vertex
fn vs_main (input: VertexInput)-> VertexOutput {
	var out: VertexOutput;

	let x = f32(1 - i32(input.vertex_index)) * 0.5;
	let y = f32(i32(input.vertex_index &1u) * 2 - 1 ) * 0.5;

	out.clip_position = vec4f(x, y, 0.0, 1.0);
	out.color = vec4f(x+y, x-y, x*y, x/y);
	//out.color = vec4f(1.0, 0.0, 0.0, 1.0);

	return out;
}


struct FragmentOutput{
	@location(0) color: vec4f,
};

@fragment
fn fs_main(in: VertexOutput) -> FragmentOutput{
	var out : FragmentOutput;
	out.color = in.color;
	return out;
}


