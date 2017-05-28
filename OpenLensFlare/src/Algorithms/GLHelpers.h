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
    
    template<size_t N>
    inline void uploadUniform(GLuint program, const char* name, GLfloat (&val)[N])
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform1fv(loc, N, (GLfloat*) val);
    }

    template<size_t N>
    inline void uploadUniform(GLuint program, const char* name, glm::vec2 (&val)[N])
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform2fv(loc, N, (GLfloat*) val);
    }

    template<size_t N>
    inline void uploadUniform(GLuint program, const char* name, glm::vec3 (&val)[N])
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform3fv(loc, N, (GLfloat*) val);
    }

    template<size_t N>
    inline void uploadUniform(GLuint program, const char* name, glm::vec4 (&val)[N])
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform4fv(loc, N, (GLfloat*) val);
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
    
    inline void uploadUniform(GLuint program, const char* name, const std::vector<GLint>& val)
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform1iv(loc, (GLsizei) val.size(), val.data());
    }

    inline void uploadUniform(GLuint program, const char* name, const std::vector<glm::ivec2>& val)
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform2iv(loc, (GLsizei) val.size(), (GLint*) val.data());
    }

    inline void uploadUniform(GLuint program, const char* name, const std::vector<glm::ivec3>& val)
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform3iv(loc, (GLsizei) val.size(), (GLint*) val.data());
    }

    inline void uploadUniform(GLuint program, const char* name, const std::vector<glm::ivec4>& val)
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform4iv(loc, (GLsizei) val.size(), (GLint*) val.data());
    }
    
    template<size_t N>
    inline void uploadUniform(GLuint program, const char* name, GLint (&val)[N])
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform1iv(loc, N, (GLint*) val);
    }

    template<size_t N>
    inline void uploadUniform(GLuint program, const char* name, glm::ivec2 (&val)[N])
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform2iv(loc, N, (GLint*) val);
    }

    template<size_t N>
    inline void uploadUniform(GLuint program, const char* name, glm::ivec3 (&val)[N])
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform3iv(loc, N, (GLint*) val);
    }

    template<size_t N>
    inline void uploadUniform(GLuint program, const char* name, glm::ivec4 (&val)[N])
    {
        GLuint loc = glGetUniformLocation(program, name);
        glUniform4iv(loc, N, (GLint*) val);
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

    /// Structure holding the shader parameters
    struct ShaderSource
    {
        /// The source code for the shader stages.
        std::map<GLenum, std::vector<const char*>> m_source;
        
        /// List of defines to set.
        std::vector<const char*> m_defines;

        /// List of varying attribute names to capture.
        std::vector<const char*> m_varyings;
    };

    /// Creates a shader from the provided shader source.
    inline GLuint createShader(const ShaderSource& source)
    {
        // Buffer for getting the error text.
        static GLchar s_errorBuffer[4098];

        // Vector to holds the various shader object.
        std::vector<GLuint> shaders;

        // Create the individual shaders.
        for (const auto& shaderSource: source.m_source)
        {
            std::vector<const char*> src;
            src.reserve(1 + shaderSource.second.size() + source.m_defines.size() * 2);

            src.push_back("#version 330\n");
            for (auto define: source.m_defines)
            {
                src.push_back(define);
                src.push_back("\n");
            }
            src.insert(src.end(), shaderSource.second.begin(), shaderSource.second.end());

            // Create the shader, set the source, compile
	        GLuint shader = glCreateShader(shaderSource.first);
            glShaderSource(shader, (GLsizei) src.size(), src.data(), NULL);
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
        
        // Bind any varyings
        if (!source.m_varyings.empty())
        {
            glTransformFeedbackVaryings(program, source.m_varyings.size(), 
                source.m_varyings.data(), GL_INTERLEAVED_ATTRIBS);
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