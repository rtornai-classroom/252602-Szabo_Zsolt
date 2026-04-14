#version 330

out		vec4	outColor;

flat in	int	aLine;


void main(void) {
	if (aLine == 1) {
		outColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
	else{
		outColor = vec4(0.0, 0.0, 1.0, 1.0);
	}
	
}