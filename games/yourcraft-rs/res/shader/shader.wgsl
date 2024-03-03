struct VertexInput{
	@builtin(vertex_index) vertex_index: u32,
	@location(0) position: vec3f,
	@location(1) color: vec3f
};
struct VertexOutput{
	@builtin(position) clip_position: vec4f,
	@location(0) color: vec3f
};

@vertex
fn vs_main (input: VertexInput)-> VertexOutput {
	var out: VertexOutput;

	//let x = f32(1 - i32(input.vertex_index)) * 0.5;
	//let y = f32(i32(input.vertex_index &1u) * 2 - 1 ) * 0.5;
	//out.clip_position = vec4f(x, y, 0.0, 1.0);

	out.color = input.color;
	out.clip_position = vec4f(input.position, 1.0);

	return out;
}


struct FragmentOutput{
	@location(0) color: vec4f,
};

@fragment
fn fs_main(in: VertexOutput) -> FragmentOutput{
	var out : FragmentOutput;
	out.color = vec4f(in.color, 1.0);
	return out;
}


