#include "GUI.h"

#include "../gui.h"

using namespace L;

GUI::Base* GUI::from(const XML& xml){
    Map<String,Base*> nothing;
    return from(xml,nothing);
}
GUI::Base* GUI::from(const XML& xml, Map<String,GUI::Base*>& ids){
    if(xml.text) // Text node
        return new Text(xml.name);
    else{
        Base* wtr;

        if(xml.name == "background"){
            if(!xml.attributes.has("color"))
                throw Exception("GUI: A background must have a color.");
            if(!xml.children.size())
                throw Exception("GUI: A background must have a child.");
            return new Background(GUI::from(xml.children[0],ids),Color::from(xml.attributes["color"]));
        }
        else if(xml.name == "border"){
            return new Border(GUI::from(xml.children[0],ids),
                              (xml.attributes.has("size")) ? FromString<size_t>(xml.attributes["size"]) : 1,
                              Color::from(xml.attributes["color"]));
        }
        else if(xml.name == "image"){
            if(!xml.attributes.has("src"))
                throw Exception("GUI: An image needs the src attribute");
            return new Image(xml.attributes["src"]);
        }
        else if(xml.name == "rectangle"){
            return new Rectangle(point(xml.attributes["size"]),
                                 Color::from(xml.attributes["color"]));
        }
        else if(xml.name == "line"){
            return new Line(point(xml.attributes["size"]),
                            Color::from(xml.attributes["color"]));
        }
        else if(xml.name == "list"){
            ListContainer* wtr((xml.attributes.has("spacing"))
                               ? new ListContainer(FromString<int>(xml.attributes["spacing"]))
                               : new ListContainer());
            L_Iter(xml.children,it)
                wtr->push_back(GUI::from(*it,ids));
            return wtr;
        }
        else if(xml.name == "text"){
            return new Text(xml.children[0].name,(xml.attributes.has("font"))
                                                  ? xml.attributes["font"]
                                                  : "");
        }
        else if(xml.name == "textinput"){
            return new TextInput((xml.attributes.has("size"))           ? GUI::point(xml.attributes["size"])   : Point<2,int>(100,20),
                                 (xml.attributes.has("font"))           ? xml.attributes["font"]               : "",
                                 (xml.attributes.has("placeholder"))    ? xml.attributes["placeholder"]        : "",
                                  xml.attributes.has("password"));
        }
        else throw Exception("GUI: Unknown node \""+xml.name+"\".");

        if(xml.attributes.has("id"))
            ids[xml.attributes["id"]] = wtr;

        return wtr;
    }
}

Point2i GUI::point(const String& str){
    List<String> coords(str.explode(' '));
    if(coords.size()==2)
        return Point<2,int>(FromString<int>(coords[0]),FromString<int>(coords[1]));
    else throw Exception("GUI: Point must be \"x y\", not \""+str+"\".");
}
