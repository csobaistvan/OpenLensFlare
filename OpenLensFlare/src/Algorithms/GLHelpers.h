#pragma once

#include "../Dependencies.h"

namespace OLEF
{
namespace GLHelpers
{
    /// Helper functions to overload a named uniform.
    inline void uploadUniform(GLuint program, const char* name, GLfloat val)
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform1f(loc, val);
    }

    inline void uploadUniform(GLuint program, const char* name, const glm::vec2& val)
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform2f(loc, val.x, val.y);
    }

    inline void uploadUniform(GLuint program, const char* name, const glm::vec3& val)
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform3f(loc, val.x, val.y, val.z);
    }

    inline void uploadUniform(GLuint program, const char* name, const glm::vec4& val)
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform4f(loc, val.x, val.y, val.z, val.w);
    }
    
    inline void uploadUniform(GLuint program, const char* name, const std::vector<GLfloat>& val)
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform1fv(loc, (GLsizei) val.size(), val.data());
    }

    inline void uploadUniform(GLuint program, const char* name, const std::vector<glm::vec2>& val)
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform2fv(loc, (GLsizei) val.size(), (GLfloat*) val.data());
    }

    inline void uploadUniform(GLuint program, const char* name, const std::vector<glm::vec3>& val)
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform3fv(loc, (GLsizei) val.size(), (GLfloat*) val.data());
    }

    inline void uploadUniform(GLuint program, const char* name, const std::vector<glm::vec4>& val)
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform4fv(loc, (GLsizei) val.size(), (GLfloat*) val.data());
    }

    inline void uploadUniform(GLuint program, const char* name, GLint val)
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform1i(loc, val);
    }

    inline void uploadUniform(GLuint program, const char* name, const glm::ivec2& val)
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform2i(loc, val.x, val.y);
    }

    inline void uploadUniform(GLuint program, const char* name, const glm::ivec3& val)
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform3i(loc, val.x, val.y, val.z);
    }

    inline void uploadUniform(GLuint program, const char* name, const glm::ivec4& val)
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform4i(loc, val.x, val.y, val.z, val.w);
    }

    inline void uploadUniform(GLuint program, const char* name, const glm::mat2& val)
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniformMatrix2fv(loc, 1, GL_FALSE, glm::value_ptr(val));
    }

    inline void uploadUniform(GLuint program, const char* name, const glm::mat3& val)
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(val));
    }

    inline void uploadUniform(GLuint program, const char* name, const glm::mat4& val)
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(val));
    }

    /// Shader source type
    using ShaderSource = std::map<GLenum, std::vector<const char*>>;

    /// Creates a shader from the provided shader source.
    inline GLuint createShader(const ShaderSource& source)
    {
        // Buffer for getting the error text.
        static GLchar s_errorBuffer[4098];

        // Vector to holds the various shader object.
        std::vector<GLuint> shaders;

        // Create the individual shaders.
        for (const auto& shaderSource: source)
        {
            // Create the shader, set the source, compile
	        GLuint shader = glCreateShader(shaderSource.first);
            glShaderSource(shader, (GLsizei) shaderSource.second.size(), 
                shaderSource.second.data(), NULL);
            glCompileShader(shader);
            
            // Make sure it compiled successfully
            GLint status;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

            if (status == GL_FALSE)
            {
                glGetShaderInfoLog(shader, sizeof(s_errorBuffer) / sizeof(GLchar), 
                    NULL, s_errorBuffer);
                
                //TODO: handle the error
                std::cout << s_errorBuffer << std::endl;
            }

            // Store the shader
            shaders.push_back(shader);
        }

        // Create the program.
        GLuint program = glCreateProgram();

        // Attach the shaders.
        for (auto shader: shaders)
        {
            glAttachShader(program, shader);
        }

        // Link the program.
        glLinkProgram(program);

        // Detach and releases the shader objects that are no longer needed.
        for (auto shader: shaders)
        {
            glDetachShader(program, shader);
            glDeleteShader(shader);
        }
        
        // Make sure it linked successfully.
        GLint status;
        glGetProgramiv(program, GL_LINK_STATUS, &status);

        if (status == GL_FALSE)
        {
            glGetProgramInfoLog(program, sizeof(s_errorBuffer) / sizeof(GLchar), 
                NULL, s_errorBuffer);
            
            //TODO: handle the error
            std::cout << s_errorBuffer << std::endl;
        }

        // Return the successfully created and linked program.
        return program;
    }

}}