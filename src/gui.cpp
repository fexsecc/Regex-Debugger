
#include <gui.h>
#include <imgui.h>
#include <filesystem>

//new imgui definitions

#define WRAPPED_BULLET_TEXT(text) ImGui::TextWrapped("-"); ImGui::SameLine(); ImGui::TextWrapped(text);
#define WRAPPED_ERROR_BULLET(error) ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,0,0,255)); ImGui::TextWrapped("-RE2_ERROR: "); ImGui::SameLine(); ImGui::TextWrapped(error); ImGui::PopStyleColor();

//end of new imgui definitions

ImFont* jetFont185;

void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

//start of opengl icon variables

const char* vertexShaderSource = "#version 130\n"
"in vec3 aPos;\n"
"out vec3 cPos;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"   cPos=aPos;\n"
"   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
"}\n";

const char* fragmentShaderSource = "#version 130\n"
"out vec4 FragColor;\n"
"in vec3 cPos;\n"
"uniform vec3 color;\n"
"uniform vec3 lightColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4((cPos.xyz/2.0+color.xyz/2.0)*lightColor, 1.0f);\n"
"}\n";

GLuint VAO, VBO, EBO, fragmentShader, vertexShader, shaderProgram, FBO, textureHolder;

glm::mat4 model, view, projection; // initialize identity matrix

float vertices[] = { //cube
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,

    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,

     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f
};

GLuint indices[] = { //cube triangles
     0, 1, 2, // trio of vertices that form the triangle
     3, 4, 5,
     6, 7, 8,
     9, 10, 11,
     12, 13, 14,
     15, 16, 17,
     18, 19, 20,
     21, 22, 23,
     24, 25, 26,
     27, 28, 29,
     30, 31, 32,
     33, 34, 35
};

//end of opengl icon variables

void initShaders() {
    vertexShader = glCreateShader(GL_VERTEX_SHADER); // create shader
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader); // compile shader

    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // fragment shader
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // create shader
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader); // compile shader
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    glGenBuffers(1, &EBO); //generate the Element Buffer Object
    glGenVertexArrays(1, &VAO); //generate the Vertex Array Object
    glGenBuffers(1, &VBO); // generate the Vertex Buffer Object (the only one for now)

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Generate and configure framebuffer
    glGenFramebuffers(1, &FBO);

    // Create texture to store the rendering output
    glGenTextures(1, &textureHolder);

    // Check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer not complete!" << std::endl;
        return; // Return early if FBO is not complete
    }

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // if you remove the clear framebuffer code, you get a trail on the polygon since it never gets cleared
    glEnable(GL_DEPTH_TEST);

}

GLFWwindow* InitializeGUI(ImVec2 initDisplaySize) { // Generate the main window
    const char* glsl_version = "#version 130";
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(initDisplaySize.x, initDisplaySize.y, "Regex-Debugger", nullptr, nullptr);
    if (window == nullptr)
        return NULL;

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1); // Enable vsync


    // Setup Dear Imgui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard navigation
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable gamepad navigation

    // Start of style setup
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    // Window bg colors
    style.Colors[ImGuiCol_WindowBg] = ImVec4(3.0/255.0, 1.0/255.0, 44.0/255.0, 1);

    //ImGui::StyleColorsLight();
  
    // End of style setup

    /* Set the font, if you fancy a different size
     * for some boxes and one for others simply load 
     * the font multiple times with different names and sizes.
     * NOTE: Make sure to follow the naming convention. */

    std::string fontPath = std::filesystem::current_path().string();
    fontPath += fontRelPath;
    const char* cFontPath = fontPath.c_str();


    io.Fonts->AddFontDefault();
    jetFont185 = io.Fonts->AddFontFromFileTTF(cFontPath, 18.5f);
    IM_ASSERT(jetFont185 != NULL);

    // Setup platform/renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);


    // Our state
    bool show_main_window = true;
    bool show_demo_window = true;

    initShaders();

    return window;
}

//the below functions are for the Explanation Window functionality

bool compareNumbers(char* s) { // returns true if first<second and false otherwise
                               //(for expressions of type "{n,m}", aka regexMultiCharOperatorsExplanation[2])
    char* p;
    p = strchr(s, '{');
    if (!p)
        return false;
    p = strpbrk(p+1, "0123456789");
    if (!p)
        return false;
    int num1=0, num2=0;
    char character[2];
    strncpy(character, p, 1);
    while (isdigit(character[0])) {
        num1 += atoi(character);
        if (num1 > 65535)
            return false;
        num1 *= 10;
        p = p + 1;
        strncpy(character, p, 1);
    }
    num1 /= 10;
    p = strpbrk(p + 1, "0123456789");
    if (!p)
        return false;
    strncpy(character, p, 1);
    while (isdigit(character[0])) {
        num2 += atoi(character);
        if (num2 > 65535)
            return false;
        num2 *= 10;
        p = p + 1;
        strncpy(character, p, 1);
    }
    num2 /= 10;
    return num1<=num2;
}

void Explain(char regexQuery[]) {
    ImGui::Begin("explanationWindow", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
    ImGui::PushFont(jetFont185);
    //single character regex expression explanations

    std::unordered_map<char, const char*> regexSingleCharOperatorsExplanation = {
        {'.', "'.' Matches any character (except newline)"},
        {'*', "'*' Matches 0 or more occurrences"},
        {'+', "'+' Matches 1 or more occurrences (can also stand for the possesive quantifier"},
        {'?', "'?' Matches 0 or 1 occurrence (can also stand for the lazy quantifier)"},
        {'|', "'|' Alternation (OR) operator"},
        {'^', "'^' Anchors to the start of a line"},
        {'$', "'$' Anchors to the end of a line"},
    };

    char* p;
    char sep[] = {
        "$^.*+?|"
    };
    p = strpbrk(regexQuery, sep);

    while (p != nullptr) {
        if (regexSingleCharOperatorsExplanation[p[0]] && (p==regexQuery || (*(p - 1) != '\\' && *(p - 1) != '['))) {
            WRAPPED_BULLET_TEXT(regexSingleCharOperatorsExplanation[p[0]]);
            regexSingleCharOperatorsExplanation.erase(p[0]);
        }
        p = strpbrk(p + 1, sep);
    }

    //end of single char regex operand explanations
    
    char* regexMultiCharOperatorsExplanation[45] = {
        // Quantifiers (SOLVED)
        {"{n} Matches exactly n times"}, // 0
        {"{n,} Matches at least n times"}, // 1
        {"{n,m} Matches between n and m times"}, // 2

        // Assertions (Lookaheads & Lookbehinds) (SOLVED)
        {"(?=...) Positive lookahead (ensures the pattern follows, but doesn't consume)"}, // 3
        {"(?!...) Negative lookahead (ensures the pattern does not follow)"}, // 4
        {"(?<=...) Positive lookbehind (ensures the pattern precedes, but doesn't consume)"}, // 5
        {"(?<!...) Negative lookbehind (ensures the pattern does not precede)"}, // 6

        // Grouping & Special Constructs (SOLVED)
        {"(?:...) Non-capturing group (groups pattern but does not store it)"}, // 7
        {"(?P<name>...) Named capturing group (Python, .NET)"}, // 8
        {"(? <name>...) Named capturing group (Java, .NET)"}, // 9
        {"(?>...) Atomic group (prevents backtracking)"}, // 10
        {"(...) is a capturing group, it captures what the expression inside matches"}, //11


        // Mode Modifiers (SOLVED)
        {"(?i) Case-insensitive mode"}, // 12
        {"(?m) Multi-line mode (^ and $ match at line breaks)"}, // 13
        {"(?s) Dot-all mode (dot matches newlines)"}, // 14
        {"(?x) Free - spacing mode(ignores spaces, allows # comments)"}, // 15
        {"(?imxs) Enables multiple modes, all at once"}, // 16

        // Conditional Expressions (SOLVED)
        {"(?(condition)left|right) - Conditional matching: If condition is met, match \"left\", otherwise match \"right\""}, // 17

        // Unicode & Advanced Escapes (SOLVED)
        {"\\p{L} Matches any Unicode letter"}, // 18
        {"\\P{L} Matches anything except a Unicode letter"}, // 19

        //Recursion (SOLVED)
        {"(?R) Calls the entire pattern again(Recursion)"}, // 20
        {"(?(DEFINE)...) Defines a subpattern for later use"}, // 21

        //Backslash Expressions (SOLVED)
        {"\\d Matches any digit character, equivalent to [0-9]"}, // 22
        {"\\D Matches any non-digit character, equivalent to [^0-9]"}, // 23
        {"\\w Matches any \"word character\", equivalent to [a-zA-Z0-9_]"}, // 24
        {"\\W Matches any \"non-word character\", equivalent to [^a-zA-Z0-9_]"}, // 25
        {"\\s Matches any whitespace characters (space, tab, newline, etc.)"}, // 26
        {"\\S Matches any non-whitespace characters (NOT space, tab, newline, etc.)"}, // 27
        {"\\b Matches a position between a word character (\\w) and a non-word character (\\W), or the beginning or end of a string"}, // 28
        {"\\B Matches everything other than a position between a word character (\\w) and a non-word character (\\W), or the beginning or end of a string"}, // 29
        {"\\n Matches a newline"}, // 30
        {"\\r Matches a carriage-return (useful for windows-style line breaks, next to \\n)"}, // 31
        {"\\t Matches a horizontal tab character (ASCII 9)"}, // 32
        {"\\f Matches a form feed (rarely used nowadays)"}, // 33
        {"\\v Matches a vertical tab character (ASCII 11)"}, // 34
        {"\\0 Matches a null byte (ASCII 0)"}, // 35
        {"\\xhh(where hh is a 2 digit hexadecimal(base 16) code) matches a character represented by its 2 digit hex code(works with 1 digit too)"}, // 36
        {"\\uHHHH(where HHHH is a 4 digit hexadecimal(base 16) code) matches a UNICODE character represented by its 4 digit hex code(useful for non - ASCII chars)(regex flavor - dependent)"}, // 37
        {"\\i (where i is a natural number reprezenting the i'th capturing group) Is a backreference to a previously captured group, it allows you to refer back to a previously matched group in the pattern (regex flavor-dependent)"}, //38

        //Square bracketed lists (SOLVED)
        {"[...] Matches any character present in the square brackets"}, //39
        {"[^...] Matches any other character than the ones present in the square brackets"}, //40
        {"[...x-y...](x and y are alphanumberical characters) matches any chars between the ASCII codes of x and y (ASCII code of x must be smaller than y's)"}, //41
        {"[...&&...] Represent the conjunction of the left and right expression, thus matching both sides (for complicated expressions)"}, // 42

        //Literal matching (literally matching expressions like "xx" or "\?")
        {"\\$ (where $ is any special character) matches s literally"}, // 43
        {"'c' (where c is any alphanumerical character or some special characters (-='\";:,<>&%#@!~`_) not used in an expression) matches the character 'c' literally"} // 44



    }; // the number after each expression is the index of that expression

    std::string regexFindingQuerys[45][1] = {
        {"{[0-9]+}"}, // 0
        {"{[0-9]+,}"}, // 1
        {"{[0-9]+,[0-9]+}"}, // 2
        {"\\(\\?\\=.*\\)"}, // 3
        {"\\(\\?\\!.*\\)"}, // 4
        {"\\(\\?\\<\\=.*\\)"}, // 5
        {"\\(\\?\\<\\!.*\\)"}, // 6
        {"\\(\\?\\:.*\\)"}, // 7
        {"\\(\\?P\\<.*\\>.*\\)"}, // 8
        {"\\(\\?\\<.*\\>.*\\)"}, // 9
        {"\\(\\?\\>.*\\)"}, // 10
        {"\\(.*\\)"}, // 11
        {"\\(\\?i\\)"}, // 12
        {"\\(\\?m\\)"}, // 13
        {"\\(\\?s\\)"}, // 14
        {"\\(\\?x\\)"}, // 15
        {"\\(\\?[xims]+\\)"}, // 16
        {"\\(\\?\\(\\?(?:!=|<!|=|<=).*|.*\\)\\)"}, // 17
        {"\\\\p\\{L\\}"}, // 18
        {"\\\\P\\{L\\}"}, // 19
        {"\\(\\?R\\)"}, // 20
        {"\\(\\?\\(DEFINE\\).*\\|.*\\)"}, // 21
        {"(?:[^\\\\]|^){1}\\\\d"}, // 22
        {"(?:[^\\\\]|^){1}\\\\D"}, // 23
        {"(?:[^\\\\]|^){1}\\\\w"}, // 24
        {"(?:[^\\\\]|^){1}\\\\W"}, // 25
        {"(?:[^\\\\]|^){1}\\\\s"}, // 26
        {"(?:[^\\\\]|^){1}\\\\S"}, // 27
        {"(?:[^\\\\]|^){1}\\\\b"}, // 28
        {"(?:[^\\\\]|^){1}\\\\B"}, // 29
        {"(?:[^\\\\]|^){1}\\\\n"}, // 30
        {"(?:[^\\\\]|^){1}\\\\r"}, // 31
        {"(?:[^\\\\]|^){1}\\\\t"}, // 32
        {"(?:[^\\\\]|^){1}\\\\f"}, // 33
        {"(?:[^\\\\]|^){1}\\\\v"}, // 34
        {"(?:[^\\\\]|^){1}\\\\0"}, // 35
        {"(?:[^\\\\]|^){1}\\\\x(?:\\d|[A-Fa-f]){1,2}"}, // 36
        {"(?:[^\\\\]|^){1}\\\\u(?:\\d|[A-Fa-f]){4}"}, // 37
        {"(?:[^\\\\]|^){1}\\\\1"}, // 38
        {"\\[[^^].*\\]"}, // 39
        {"\\[[\\^].*\\]"}, // 40
        {"\\[.*[a-zA-Z0-9]\\-[a-zA-Z0-9].*\\]"}, // 41
        {"\\[.+&&.+\\]"}, // 42
        {"\\\\[\\]\\[\\@\\#\\$\\%\\^\\&\\}\\*\\{\\)\\(\\\\\\-\\=\\.\\,\\!\\<\\>\\'\\\"\\;\\:\\_]"}, // 43
        {"(?:^[a-zA-Z0-9\\-='\";:,<>&%#!@~`_ ])|(?:[^\\\\][a-zA-Z0-9\\-='\";:,<>&%#!@~`_]+)"} // 44
    };

    for (int i = 0; i < 45; ++i) {
        RE2 pattern(regexFindingQuerys[i][0]);
        if (i == 2) {
            if (RE2::PartialMatch(regexQuery, pattern) && compareNumbers(regexQuery)) {
                    WRAPPED_BULLET_TEXT(regexMultiCharOperatorsExplanation[i]);
            }
        }
        else if (RE2::PartialMatch(regexQuery, pattern)) {
            WRAPPED_BULLET_TEXT(regexMultiCharOperatorsExplanation[i]);
        }
    }
    ImGui::PopFont();
    ImGui::End();
}

// the below functions are no longer for the explanation window functionality

//scaling function for ImGui windows
void ApplyScale(char name[], ImVec2 initSize, ImVec2 scale) {
    ImGui::Begin(name);
    ImGui::PushFont(jetFont185);

    ImGui::SetWindowSize(ImVec2(initSize.x * scale.x, initSize.y * scale.y), 0); //apply scale to the specifically named window

    ImGui::PopFont();
    ImGui::End();
}

//function for the Regexp Input Window
void showErrorsInRegexp(char regexQuery[]) {
    RE2 regexp(regexQuery);
    if (!regexp.ok()) {
        ImGui::Begin("Regexp Input Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration);
        ImGui::PushFont(jetFont185);
        WRAPPED_ERROR_BULLET(regexp.error().c_str());
        ImGui::PopFont();
        ImGui::End();
    }
}

//function for the Test String Input Window
void showWhatGetsMatched(char regexQuery[],char testString[]) {
    ImGui::Begin("Text String Input Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration);
    ImGui::PushFont(jetFont185);
    std::string word;
    RE2 regexp(regexQuery);
    std::string pattern = "("+ regexp.pattern() + ")"; // makes the pattern search for a group so that findAndConsume works
    RE2 realRegexp(pattern);
    if (RE2::FullMatch(absl::string_view(testString), regexp)) {
        WRAPPED_BULLET_TEXT("fully matched!");
    }
    else {
        re2::StringPiece testedString(testString);
        while (RE2::FindAndConsume(&testedString, realRegexp, &word)) {
            if (word.empty()) {
                WRAPPED_ERROR_BULLET("empty match captured, cant Partially Match!");
                break;
            }
            else
                WRAPPED_BULLET_TEXT(word.c_str());
        }
    }
    ImGui::PopFont();
    ImGui::End();
}


void generateFocusedExplanationWindow(ImVec2 scale, int state[], char regexQuery[]) {
    ImGui::Begin("explanationWindow", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
    ImGui::PushFont(jetFont185);
    if (ImGui::Button("Go back", ImVec2(150 * scale.x, 30 * scale.y)))
        state[0] = 0;
    ImGui::Text("This is the explanation window!");
    Explain(regexQuery);
    ImGui::SetWindowPos(ImVec2(380 * scale.x, 324 * scale.y), 0);
    ApplyScale("explanationWindow", ImVec2(900, 420), scale);
    ImGui::Text("");
    ImGui::PopFont();
    ImGui::End();
}

void generateFocusedValidInputWindow(ImVec2 scale, int state[], char regexQuery[]) {
    ImGui::Begin("1", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove); //if you use the same name it changes the already existing window
    ImGui::PushFont(jetFont185);
    if (ImGui::Button("Go back", ImVec2(150 * scale.x, 30 * scale.y)))
        state[0] = 0;
    ImGui::Text("This is the valid input window!");
    ImGui::SetWindowPos(ImVec2(380 * scale.x, 324 * scale.y), 0);
    ApplyScale("1", ImVec2(900, 420), scale);
    ImGui::Text("");
    ImGui::PopFont();
    ImGui::End();
}

void generateBothVIandEWindows(ImVec2 scale, int state[], char regexQuery[]) {

    //create the valid input window

    ImGui::Begin("1", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove); //if you use the same name it changes the already existing window
    ImGui::PushFont(jetFont185);
    if (ImGui::Button("Focus VI window", ImVec2(150 * scale.x, 30 * scale.y)))
        state[0] = 2;
    ImGui::Text("This is the valid input window!");
    ImGui::SetWindowPos(ImVec2(380 * scale.x, 324 * scale.y), 0);
    ApplyScale("1", ImVec2(450, 407.7), scale);
    ImGui::Text("");
    ImGui::PopFont();
    ImGui::End();

    // create the explanation window

    ImGui::Begin("explanationWindow", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
    ImGui::PushFont(jetFont185);
    if (ImGui::Button("Focus Ex window", ImVec2(150 * scale.x, 30 * scale.y)))
        state[0] = 1;
    ImGui::Text("This is the explanation window!");
    ImGui::SetWindowPos(ImVec2(830 * scale.x, 324 * scale.y), 0);
    ApplyScale("explanationWindow", ImVec2(450, 408), scale);
    Explain(regexQuery);
    ImGui::Text("");
    ImGui::PopFont();
    ImGui::End();

}

//The below function generates the above windows
void generateWindows(GLFWwindow* window, int& displayW, int& displayH, ImVec2 initDisplaySize, int state[]) { //This is where you put secondary windows (tabs,buttons,tables,checkboxes and other windows)
    glfwGetFramebufferSize(window, &displayW, &displayH);
    ImVec2 scale = ImVec2(displayW / initDisplaySize.x, displayH / initDisplaySize.y);

    static char buf[10001] = " "; //this is the Regex Input tab
    {
        ImGui::Begin("Regexp Input Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration);
        ImGui::PushFont(jetFont185);
        ImGui::InputText("<--regexInput (USES RE2)", buf, IM_ARRAYSIZE(buf));
        ImGui::PushTextWrapPos();
        ImGui::TextUnformatted(buf);
        ImGui::PopTextWrapPos();
        showErrorsInRegexp(buf);
        ImGui::SetWindowPos(ImVec2(380 * scale.x, 0), 0);
        ApplyScale("Regexp Input Window", ImVec2(900, 324), scale);
        ImGui::Text("");
        ImGui::PopFont();
        ImGui::End();
    }
    static char buf2[10001] = "Test String"; // this is the Test String tab
    {
        ImGui::Begin("Text String Input Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration);
        ImGui::PushFont(jetFont185);
        ImGui::InputTextMultiline(" ", buf2, IM_ARRAYSIZE(buf2), ImVec2(360*scale.x,120*scale.y));
        showWhatGetsMatched(buf, buf2);
        ImGui::SetWindowPos(ImVec2(0, 0), 0);
        ApplyScale("Text String Input Window", ImVec2(380, 324), scale);
        ImGui::Text("");
        ImGui::PopFont();
        ImGui::End();
    }
    switch (state[0]) {
    case 1:
    { //focused explanation window
        generateFocusedExplanationWindow(scale, state, buf);
        break;
    }
    case 2:
    { //focused valid input window
        generateFocusedValidInputWindow(scale, state, buf);
        break;
    }
    default:
    { //both valid input and explanation windows at the same time
        generateBothVIandEWindows(scale, state, buf);
        break;
    }
    }
}

//end of window generation functions


void initTextureAndViewport() {

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);  // Bind framebuffer

    // Clear the framebuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(10, -10, 250, 250);
    glBindTexture(GL_TEXTURE_2D, textureHolder);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 250, 250, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0); // Create texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Attach texture to the framebuffer
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureHolder, 0);

}

void performShaderUniformArithmetic() {
    //uniform arithmetic

    //"global" variables
    glUseProgram(shaderProgram);
    float timeValue = glfwGetTime();
    int Location;
    model = glm::mat4(1.0f);
    view = glm::mat4(1.0f);
    projection = glm::mat4(1.0f);
    //vertex shader uniform arithmetic

    Location = glGetUniformLocation(shaderProgram, "model"); //using a model matrix to transform the vertices (default transforms)
    model = glm::rotate(model, glm::radians(65.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f)); //scale setter
    model = glm::translate(model, glm::vec3(sin(timeValue * 0.5f) * 5.0f, -cos(timeValue * 0.5f) * 5.0f, 0.0f));
    model = glm::rotate(model, glm::radians(timeValue * 300), glm::vec3(1.0f, 1.0f, 1.0f));
    glUniformMatrix4fv(Location, 1, GL_FALSE, glm::value_ptr(model));

    Location = glGetUniformLocation(shaderProgram, "view"); // view transform sets the camera
    GLfloat camerax = cos(timeValue) * 2;
    GLfloat cameraz = sin(timeValue) * 2;
    view = glm::lookAt(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // creates a "camera" (a separate xyz coordinate system that simulates a camera)
    glUniformMatrix4fv(Location, 1, GL_FALSE, glm::value_ptr(view));

    Location = glGetUniformLocation(shaderProgram, "projection"); //projection matrix sets the view mode to "perspective" (so things look smaller further away)
    projection = glm::perspective(glm::radians(45.0f), 200.0f / 200.0f, 0.1f, 100.0f);
    glUniformMatrix4fv(Location, 1, GL_FALSE, glm::value_ptr(projection));

    //fragment shader uniform arithmetic
    Location = glGetUniformLocation(shaderProgram, "color");
    glUniform3f(Location, sin(timeValue) / 2.0 + 0.5, cos(timeValue) / 2.0 + 0.5, sin(timeValue) / 4.0 + cos(timeValue) / 4.0 + 0.5);
    Location = glGetUniformLocation(shaderProgram, "lightColor");
    glUniform3f(Location, 1.0f, 1.0f, 1.0f); //diffuse light

    glUseProgram(0);

    //end of uniform arithmetic

}

void drawAndDisplayTexture(ImVec2 scale) {

    // Render to the framebuffer
    glUseProgram(shaderProgram);   // Use the shader program
    glBindVertexArray(VAO);       // Bind the vertex array object

    glDrawElements(GL_TRIANGLES, 120, GL_UNSIGNED_INT, 0);  // Render the Triangle

    // Display the texture in ImGui
    ImGui::Begin("Icon", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
    ImGui::PushFont(jetFont185);
    ImGui::SetWindowPos(ImVec2(0, 420 * scale.y), 0);
    ImGui::Image(textureHolder, ImVec2(300 * scale.x, 300 * scale.y)); // Display the texture
    ApplyScale("Icon", ImVec2(300, 300), scale);
    ImGui::PopFont();
    ImGui::End();

}

//the function below generates the floating 3d model using the above three functions
void generateIcon(ImVec2 displaySize, ImVec2 InitDisplaySize) {

    /*
    This function puts a texture into a frameBuffer using a frameBufferObject(FBO),

    Then it generates an empty image using textureHolder and applies the shader to it.

    After rendering and drawing the texture, this function puts the texture into an ImGui tab using ImGui::Image
    */

    ImVec2 scale = ImVec2(displaySize.x / InitDisplaySize.x, displaySize.y / InitDisplaySize.y);

    initTextureAndViewport();

    performShaderUniformArithmetic();

    drawAndDisplayTexture(scale);

    // Unbind framebuffer (back to default framebuffer)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

//render loop(core function of the whole GUI)
void RenderGUI(GLFWwindow* window, ImVec2 initDisplaySize) {
    bool showCoolButton = true;
    ImVec4 clear_color = ImVec4(0.01f, 0.01f, 0.25f, 1.00f);
    int display_w, display_h;
    int state[1] = {}; // change this when you add another window with different states

    while (!glfwWindowShouldClose(window)) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Window contents are in the generateWindows() function (use Begin/End pair to create a named window)
        generateWindows(window, display_h, display_w, initDisplaySize, state);

        generateIcon(ImVec2(display_h, display_w), initDisplaySize);

        ImGui::Render();
        glfwGetFramebufferSize(window, &display_w, &display_h);

        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

#undef WRAPPED_BULLET_TEXT
#undef WRAPPED_ERROR_BULLET
