
vec4 interpolate_3d_sampler(sampler3D samp, ivec3 c, int level, vec3 frac) {
	vec4 data000 = texelFetch(samp, c + ivec3(0, 0, 0), int(level));
	vec4 data100 = texelFetch(samp, c + ivec3(1, 0, 0), int(level));
	vec4 data010 = texelFetch(samp, c + ivec3(0, 1, 0), int(level));
	vec4 data110 = texelFetch(samp, c + ivec3(1, 1, 0), int(level));
	vec4 data001 = texelFetch(samp, c + ivec3(0, 0, 1), int(level));
	vec4 data101 = texelFetch(samp, c + ivec3(1, 0, 1), int(level));
	vec4 data011 = texelFetch(samp, c + ivec3(0, 1, 1), int(level));
	vec4 data111 = texelFetch(samp, c + ivec3(1, 1, 1), int(level));
			
	vec4 dataX00 = mix(data000, data100, frac.x);
	vec4 dataX10 = mix(data010, data110, frac.x);
	vec4 dataX01 = mix(data001, data101, frac.x);
	vec4 dataX11 = mix(data011, data111, frac.x);
			
	vec4 dataXY0 = mix(dataX00, dataX10, frac.y);
	vec4 dataXY1 = mix(dataX01, dataX11, frac.y);
			
	vec4 data = mix(dataXY0, dataXY1, frac.z);

	return data;
}
