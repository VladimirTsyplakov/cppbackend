#include "json_serializer.h"
#include <string>
#include <iostream>

namespace json_serializer {

namespace json = boost::json;

using namespace std::literals;

json::value SerializeError(std::string code, std::string message) {
    json::object object;
	object["code"] = code;
	object["message"] = message;
    return json::value(std::move(object));
}

json::value Serialize(const Game::Maps& maps) {
    json::array maps_array;

    for (const auto& map : maps) {
        json::object res_object;
        res_object["id"] = *map.GetId();
        res_object["name"] = map.GetName();
        maps_array.push_back(res_object);
    }

    return json::value(std::move(maps_array));
}

namespace details {

json::value Serialize(const Road& road) {
    json::object object;

    const auto& [start_x, start_y] = road.GetStart();
    object["x0"] = start_x;
    object["y0"] = start_y;

    const auto& [end_x, end_y] = road.GetEnd();
    if (road.IsVertical()) {
        object["y1"] = end_y;
    } else {
        object["x1"] = end_x;
    }

    return json::value(std::move(object));
}

json::value Serialize(const Building& building) {
    json::object object;

    const auto& [position, size] = building.GetBounds();
    object["x"] = position.x;
    object["y"] = position.y;
    object["w"] = size.width;
    object["h"] = size.height;
    return json::value(std::move(object));
}

json::value Serialize(const Office& office) {
    json::object object;

    const auto& id = office.GetId();
    const auto& position = office.GetPosition();
    const auto& offset = office.GetOffset();

    object["id"] = *id;
    object["x"] = position.x;
    object["y"] = position.y;
    object["offsetX"] = offset.dx;
    object["offsetY"] = offset.dy;

    return json::value(std::move(object));
}

}  // namespace details

json::value Serialize(const Map& map) {
    json::object object;

    object["id"] = *map.GetId();
    object["name"] = map.GetName();

    json::array roads_array;
    /*for (const auto& road: map.GetRoads()) {
        roads_array.push_back(details::Serialize(road));
    }
    object["roads"] = roads_array;*/
	const auto roads = map.GetRoads();
	std::transform( roads.begin(),
			roads.end(), std::back_inserter(roads_array),
			[](const Road& a)
			{return details::Serialize(a);});
	object["roads"] = roads_array;
/*

*/

    json::array buildings_array;
	const auto buildings = map.GetBuildings();
	std::transform( buildings.begin(), 
			buildings.end(), std::back_inserter(roads_array),
			[](const Building& b)
			{return details::Serialize(b);});

    for (const auto& building: map.GetBuildings()) {
        buildings_array.push_back(details::Serialize(building));
    }
    object["buildings"] = buildings_array;

    json::array offices_array;
    for (const auto& office: map.GetOffices()) {
        offices_array.push_back(details::Serialize(office));
    }
    object["offices"] = offices_array;

    return json::value(std::move(object));
}

}  // namespace json_serializer
