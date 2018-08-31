#include <L/src/container/Buffer.h>
#include <L/src/engine/Resource.inl>
#include <L/src/rendering/Shader.h>
#include <L/src/rendering/shader_lib.h>
#include <L/src/system/System.h>
#include <L/src/text/String.h>
#include <L/src/stream/CFileStream.h>

using namespace L;

bool glsl_loader(ResourceSlot& slot, Shader::Intermediate& intermediate) {
  intermediate.stage = VK_SHADER_STAGE_ALL;
  String stage_name;
  if(!strcmp(strrchr(slot.path, '.'), ".frag")) {
    intermediate.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stage_name = "frag";
  } else if(!strcmp(strrchr(slot.path, '.'), ".vert")) {
    intermediate.stage = VK_SHADER_STAGE_VERTEX_BIT;
    stage_name = "vert";
  }

  String cmd_output;
  char input_file[L_tmpnam], output_file[L_tmpnam];
  tmpnam(input_file);
  tmpnam(output_file);

  {
    Buffer original_text(CFileStream(slot.path, "rb").read_into_buffer());
    CFileStream input_stream(input_file, "wb");
    input_stream << L_GLSL_INTRO << L_SHAREDUNIFORM << L_PUSH_CONSTANTS;
    if(intermediate.stage == VK_SHADER_STAGE_FRAGMENT_BIT)
      input_stream << L_SHADER_LIB;
    input_stream << '\n';
    input_stream.write(original_text.data(), original_text.size());
  }

  const String cmd("glslangValidator -V -S "+stage_name+" "+input_file+" -o "+output_file);
  System::call(cmd, cmd_output);

  if(cmd_output.empty()) {
    return false;
  }

  { // Parse debug information
    Array<String> lines(cmd_output.explode('\n')), words;

    { // Early exit in case of errors
      bool has_errors(false);
      for(String& line : lines) {
        if(strstr(line, "ERROR:")) {
          line.replaceAll(input_file, String(slot.path));
          err << line << '\n';
          has_errors = true;
        }
      }

      if(has_errors) {
        return false;
      }
    }
  }

  if(CFileStream file_stream = CFileStream(output_file, "rb")) {
    intermediate.binary = file_stream.read_into_buffer();
    Shader::reflect(intermediate);
    return true;
  } else {
    return false;
  }
}

void glsl_module_init() {
  ResourceLoading<Shader>::add_loader("vert", glsl_loader);
  ResourceLoading<Shader>::add_loader("frag", glsl_loader);
}
