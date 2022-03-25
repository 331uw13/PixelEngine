#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "shader.h"



u8 shader_ok(u32 id, u32 type) {
	int p = 0;
	u8 res = 1;

	if(type == SHADER_MODULE) {
		glGetShaderiv(id, GL_COMPILE_STATUS, &p);
	}
	else if(type == SHADER_PROGRAM) {
		glGetProgramiv(id, GL_LINK_STATUS, &p);
	}

	if(!p) {
		res = 0;
		char* buf = NULL;
		int max_length = 0;
		int length = 0;

		if(type == SHADER_MODULE) {
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &max_length);
		}
		else if(type == SHADER_PROGRAM) {
			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &max_length);
		}

		if(max_length > 1) {
		
			buf = malloc(max_length);

			if(type == SHADER_MODULE) {
				glGetShaderInfoLog(id, max_length, &length, buf);
			}
			else if(type == SHADER_PROGRAM) {
				glGetProgramInfoLog(id, max_length, &length, buf);
			}

			fprintf(stderr, "\033[31m%s\033[0m\n", buf);
			free(buf);
		}
	}
	return res;
}


u32 create_shader(const char* vssrc, const char* fssrc) {

	u32 program = 0;

	u32 fs = glCreateShader(GL_FRAGMENT_SHADER);
	if(fs == 0) {
		fprintf(stderr, "\033[31mERROR: Failed to create GL_FRAGMENT_SHADER!\033[0m\n");
		goto finish;
	}
	
	u32 vs = glCreateShader(GL_VERTEX_SHADER);
	if(vs == 0) {
		glDeleteShader(fs);
		fprintf(stderr, "\033[31mERROR: Failed to create GL_VERTEX_SHADER!\033[0m\n");
		goto finish;
	}


	glShaderSource(fs, 1, &fssrc, NULL);
	glCompileShader(fs);

	if(!shader_ok(fs, SHADER_MODULE)) {
		goto finish;
	}

	glShaderSource(vs, 1, &vssrc, NULL);
	glCompileShader(vs);

	if(!shader_ok(vs, SHADER_MODULE)) {
		goto finish;
	}

	program = glCreateProgram();

	glAttachShader(program, fs);
	glAttachShader(program, vs);
	glLinkProgram(program);

	glDeleteShader(fs);
	glDeleteShader(vs);
	
	if(!shader_ok(program, SHADER_PROGRAM)) {
		goto finish;
	}

finish:
	return program;
}

