#version 460 core

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec4 in_worldPos;
layout(location = 2) in float in_scalling;
layout(location = 3) in float in_fraction;

layout(location = 0) out vec4 out_color;
uniform vec4 g_cameraPosition;
void main()
{
	const float maxFadeDistance = 25.0f;
	const float heightToFadeDistanceRatio = 15.0f;

	float fadeDistance = abs(g_cameraPosition.z) * heightToFadeDistanceRatio;
	fadeDistance = min(fadeDistance, in_scalling * 0.05f);
	fadeDistance = max(fadeDistance, in_scalling * 0.5f);

	float distanceToCamera = length(in_worldPos.xy - g_cameraPosition.xy);
	float opacityFalloff = smoothstep(1.0, 0.0, distanceToCamera / fadeDistance);

	out_color = vec4(in_color, opacityFalloff * (1.0 - in_fraction));
};