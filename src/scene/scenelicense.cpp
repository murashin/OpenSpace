/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2017                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#include <openspace/scene/scenelicense.h>

#include <openspace/openspace.h>
#include <openspace/documentation/verifier.h>

#include <ghoul/filesystem/filesystem.h>
#include <ghoul/misc/assert.h>
#include <ghoul/misc/dictionary.h>

#include <fstream>
#include <sstream>

namespace {
    const char* LicenseKeyName = "Name";
    const char* LicenseKeyAttribution = "Attribution";
    const char* LicenseKeyUrl = "URL";
    const char* LicenseKeyLicenseText = "License";
    
    const char* MainTemplateFilename = "${OPENSPACE_DATA}/web/scenelicense/main.hbs";
    const char* SceneLicenseTemplateFilename = "${OPENSPACE_DATA}/web/scenelicense/scenelicense.hbs";
    const char* HandlebarsFilename = "${OPENSPACE_DATA}/web/common/handlebars-v4.0.5.js";
    const char* JsFilename = "${OPENSPACE_DATA}/web/dscenelicense/script.js";
    const char* BootstrapFilename = "${OPENSPACE_DATA}/web/common/bootstrap.min.css";
    const char* CssFilename = "${OPENSPACE_DATA}/web/common/style.css";
}

namespace openspace {

documentation::Documentation SceneLicense::Documentation() {
    using namespace documentation;
    
    return {
        "License Information",
        "core_license",
        {
            {
                LicenseKeyName,
                new StringVerifier,
                "A short, descriptive name for the license employed for this node.",
                Optional::No
            },
            {
                LicenseKeyAttribution,
                new StringVerifier,
                "The organization that shall be attributed to the licensed content.",
                Optional::No
            },
            {
                LicenseKeyUrl,
                new StringVerifier,
                "The URL pointing to the original license.",
                Optional::Yes
            },
            {
                LicenseKeyLicenseText,
                new StringVerifier,
                "The full text of the license agreements.",
                Optional::No
            }
        },
        Exhaustive::Yes
    };
}
    
SceneLicense::SceneLicense(const ghoul::Dictionary& dictionary, std::string m)
    : module(std::move(m))
{
    ghoul_assert(!module.empty(), "Module name must not be empty");

    documentation::testSpecificationAndThrow(Documentation(), dictionary, "SceneLicense");
    
    name = dictionary.value<std::string>(LicenseKeyName);
    attribution = dictionary.value<std::string>(LicenseKeyAttribution);
    dictionary.getValue(LicenseKeyUrl, url);
    licenseText = dictionary.value<std::string>(LicenseKeyLicenseText);
}
    
void writeSceneLicenseDocumentation(const std::vector<SceneLicense>& licenses,
                                    const std::string& file, const std::string& type)
{
    if (type == "text") {
        std::ofstream f;
        f.exceptions(~std::ofstream::goodbit);
        f.open(absPath(file));
        
        for (const SceneLicense& license : licenses) {
            f << license.module << '\n';
            f << "============\n";
            f << "Name:        " << license.name << '\n';
            f << "Attribution: " << license.attribution << '\n';
            f << "URL:         " << license.url << '\n';
            f << "License:\n";
            f << license.licenseText;
            f << "\n\n";
        }
        
    }
    else if (type == "html") {
        std::ifstream handlebarsInput(absPath(HandlebarsFilename));
        std::ifstream jsInput(absPath(JsFilename));
        
        std::string jsContent;
        std::back_insert_iterator<std::string> jsInserter(jsContent);
        
        std::copy(std::istreambuf_iterator<char>{handlebarsInput}, std::istreambuf_iterator<char>(), jsInserter);
        std::copy(std::istreambuf_iterator<char>{jsInput}, std::istreambuf_iterator<char>(), jsInserter);
        
        std::ifstream bootstrapInput(absPath(BootstrapFilename));
        std::ifstream cssInput(absPath(CssFilename));
        
        std::string cssContent;
        std::back_insert_iterator<std::string> cssInserter(cssContent);
        
        std::copy(std::istreambuf_iterator<char>{bootstrapInput}, std::istreambuf_iterator<char>(), cssInserter);
        std::copy(std::istreambuf_iterator<char>{cssInput}, std::istreambuf_iterator<char>(), cssInserter);
        
        std::ifstream mainTemplateInput(absPath(MainTemplateFilename));
        std::string mainTemplateContent{ std::istreambuf_iterator<char>{mainTemplateInput},
            std::istreambuf_iterator<char>{}};
        
        std::ifstream sceneLicenseTemplateInput(absPath(SceneLicenseTemplateFilename));
        std::string sceneLicenseTemplateContent{ std::istreambuf_iterator<char>{sceneLicenseTemplateInput},
            std::istreambuf_iterator<char>{} };
        
        std::ofstream f;
        f.exceptions(~std::ofstream::goodbit);
        f.open(file);
        
        std::stringstream json;
        
        json << '[';
        
        for (const SceneLicense& license : licenses) {
            json << '{';
            
            json << "\"module\": \"" << license.module << "\",";
            json << "\"name\": \"" << license.name << "\",";
            json << "\"attribution\": \"" << license.attribution << "\",";
            json << "\"url\": \"" << license.url << "\",";
            json << "\"licenseText\": \"" << license.licenseText << "\"";
            
            json << '}';
            
            if (&license != &licenses.back()) {
                json << ',';
            }
        }
        
        json << ']';
        
        std::string jsonString = "";
        for (const char& c : json.str()) {
            if (c == '\'') {
                jsonString += "\\'";
            } else {
                jsonString += c;
            }
        }
        
        std::stringstream html;
        html << "<!DOCTYPE html>\n"
        << "<html>\n"
        << "\t<head>\n"
        << "\t\t<script id=\"mainTemplate\" type=\"text/x-handlebars-template\">\n"
        << mainTemplateContent << "\n"
        << "\t\t</script>\n"
        << "\t\t<script id=\"sceneLicenseTemplate\" type=\"text/x-handlebars-template\">\n"
        << sceneLicenseTemplateContent << "\n"
        << "\t\t</script>\n"
        << "\t<script>\n"
        << "var licenses = JSON.parse('" << jsonString << "');\n"
        << "var version = [" << OPENSPACE_VERSION_MAJOR << ", " << OPENSPACE_VERSION_MINOR << ", " << OPENSPACE_VERSION_PATCH << "];\n"
        << jsContent << "\n"
        << "\t</script>\n"
        << "\t<style type=\"text/css\">\n"
        << cssContent << "\n"
        << "\t</style>\n"
        << "\t\t<title>Scene License</title>\n"
        << "\t</head>\n"
        << "\t<body>\n"
        << "\t<body>\n"
        << "</html>\n";
        
        f << html.str();
    }
}

} // namespace openspace
