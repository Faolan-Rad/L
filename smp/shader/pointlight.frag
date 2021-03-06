layout(location = 0) out vec4 fragcolor;

layout(binding = 1) uniform Parameters {
  vec4 color;
  float intensity;
};
 
layout(binding = 2) uniform sampler2D color_buffer;
layout(binding = 3) uniform sampler2D normal_buffer;
layout(binding = 4) uniform sampler2D depth_buffer;

void main() {
  vec3 light_pos = model[3].xyz;
  float radius = length(model[0].xyz)*0.83;
  GBufferSample gbuffer = sample_gbuffer(color_buffer, normal_buffer, depth_buffer);
  vec3 frag_to_light = (light_pos-gbuffer.position);
  vec3 to_light_dir = normalize(frag_to_light);
  vec3 view_dir = normalize(eye.xyz - gbuffer.position);
  vec3 halfway = normalize(view_dir+to_light_dir);
  float dist = length(frag_to_light);
  float att = light_attenuation(dist,radius,intensity);
  vec3 radiance = color.rgb*att;
  vec3 F0 = mix(vec3(0.04f),gbuffer.color,gbuffer.metalness);
  vec3 F = fresnel_schlick(max(dot(halfway,view_dir),0.f),F0);
  float NDF = distribution_GGX(gbuffer.normal,halfway,gbuffer.roughness);
  float G = geometry_smith(gbuffer.normal,view_dir,to_light_dir,gbuffer.roughness);
  vec3 spec = specular(NDF,G,F,gbuffer.normal,view_dir,to_light_dir);
  vec3 diff = (vec3(1.0)-F) * (1.f-gbuffer.metalness) * gbuffer.color;
  float NdotL = max(dot(gbuffer.normal, to_light_dir), 0.0);
  fragcolor.rgb = (diff / PI + spec) * radiance * NdotL;
  fragcolor.a = 1.f;
}
